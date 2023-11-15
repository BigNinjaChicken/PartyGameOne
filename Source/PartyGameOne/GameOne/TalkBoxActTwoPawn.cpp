// Fill out your copyright notice in the Description page of Project Settings.


#include "TalkBoxActTwoPawn.h"
#include "WebSocketGameInstance.h"
#include "TimerUserWidget.h"
#include <Dom/JsonObject.h>
#include <GameFramework/Actor.h>
#include "TalkBoxActOnePawn.h"
#include "ShowResponsesUserWidget.h"
#include <UMG/Public/Blueprint/UserWidget.h>
#include "ShowAllGoupResponsesUserWidget.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
ATalkBoxActTwoPawn::ATalkBoxActTwoPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATalkBoxActTwoPawn::BeginPlay()
{
	Super::BeginPlay();

	GameInstance->WebSocket->OnMessage().AddUObject(this, &ATalkBoxActTwoPawn::OnWebSocketRecieveMessage);

	TimerWidgetInstance->StartTimer(InputPromptTime - InputPromptSafetyTime);

	GetWorld()->GetTimerManager().SetTimer(GameTimerHandle, this, &ATalkBoxActTwoPawn::EndRound, InputPromptTime, false);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("Stage", 9);
	GameInstance->SendJsonObject(JsonObject);

	SendPlayersSentenceFragments();

	for (auto Player : GameInstance->AllPlayerInfo) {
		TSharedPtr<FJsonObject> JsonObjectTime = MakeShareable(new FJsonObject);
		JsonObjectTime->SetStringField("Timer", FString::FromInt(InputPromptTime / Player.Value.ScoreMultiplier));
		JsonObjectTime->SetStringField("playerName", Player.Key);
		GameInstance->SendJsonObject(JsonObjectTime);
	}
}

void ATalkBoxActTwoPawn::UpdateScoreOnDevice(FString playerName) {
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("Score", GameInstance->AllPlayerInfo[playerName].Score);
	JsonObject->SetStringField("playerName", playerName);
	GameInstance->SendJsonObject(JsonObject);
}

// Called every frame
void ATalkBoxActTwoPawn::Tick(float DeltaTime)
{
	
}

void ATalkBoxActTwoPawn::OnWebSocketRecieveMessage(const FString& MessageString) {
	// Create JSON object to be sent out
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MessageString);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("Deserialize Failed"))
			return;
	}

	FString playerName = GameInstance->GetJsonChildrenString(JsonObject, "clientInfo", "playerName");
	if (playerName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("invalid cliendId"))
			return;
	}

	ReceivePlayerAllPoleVote(JsonObject);

	PromptResponceUserInputPromptOne(JsonObject, playerName);

	PromptResponceUserInputPromptTwo(JsonObject, playerName);

	RecievedPlayerPoleVote(JsonObject);
}

void ATalkBoxActTwoPawn::ReceivePlayerAllPoleVote(TSharedPtr<FJsonObject> JsonObject)
{
	uint64 option;
	if (JsonObject->TryGetNumberField(TEXT("option"), option))
	{
		AllGamePrompts[option].SentenceFragments.FragThreeFourGroupPoints += 100;
		GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentThreePlayerId].Score += 300 * GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentThreePlayerId].ScoreMultiplier;
		UpdateScoreOnDevice(AllGamePrompts[option].FragmentThreePlayerId);
		GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentFourPlayerId].Score += 300 * GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentFourPlayerId].ScoreMultiplier;
		UpdateScoreOnDevice(AllGamePrompts[option].FragmentFourPlayerId);

		TotalOptionsInputed++;

		if (TotalOptionsInputed == GameInstance->AllPlayerInfo.Num()) {
			if (ScoreboardLevel.IsNull()) {
				UE_LOG(LogTemp, Error, TEXT("Invalid ScoreboardLevel"));
				return;
			}
			GameInstance->AllGamePrompts = AllGamePrompts;
			UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), ScoreboardLevel);
			return;
		}
	}
}

void ATalkBoxActTwoPawn::RecievedPlayerPoleVote(TSharedPtr<FJsonObject> JsonObject)
{
	FString poleSelection;
	if (JsonObject->TryGetStringField(TEXT("poleSelection"), poleSelection))
	{
		if (poleSelection == "Option1") {
			CurrentPoleVoteTotals.Option1Votes++;
			GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentThreePlayerId].Score += 200 * GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentThreePlayerId].ScoreMultiplier;
			UpdateScoreOnDevice(AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentThreePlayerId);
		}
		else if (poleSelection == "Option2") {
			CurrentPoleVoteTotals.Option2Votes++;
			GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFourPlayerId].Score += 200 * GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFourPlayerId].ScoreMultiplier;
			UpdateScoreOnDevice(AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFourPlayerId);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Invalid Option"))
		}

		if (CurrentPoleVoteTotals.TotalVotes() == AllPlayerIds.Num()) {
			// All players have submitted a vote

			// Determine the winner of the current round and display it
			FString winnerPlayerId;
			if (CurrentPoleVoteTotals.Option1Votes > CurrentPoleVoteTotals.Option2Votes) {
				winnerPlayerId = AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentThreePlayerId;
			}
			else {
				winnerPlayerId = AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFourPlayerId;
			}

			// Display the winner on the ShowResponcesWidgetInstance
			ShowResponcesWidgetInstance->DisplayWinner(winnerPlayerId, CurrentPoleVoteTotals.Option1Votes > CurrentPoleVoteTotals.Option2Votes);

			// Callback or event after winner is displayed
			ShowResponcesWidgetInstance->OnWinnerDisplayed.AddDynamic(this, &ATalkBoxActTwoPawn::OnWinnerDisplayed);
		}
	}
}

void ATalkBoxActTwoPawn::OnWinnerDisplayed() {
	if (ShowResponcesWidgetInstance->index < AllGamePrompts.Num()) {
		// Move to the next prompt
		ShowResponcesWidgetInstance->index++;
		ShowResponcesWidgetInstance->ShowPrompts(AllGamePrompts); // Display next prompt
		SendPlayerPole();

		return;
	}

	if (!ShowAllGoupResponsesUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("ShowAllGoupResponsesUserWidget null"));
		return;
	}

	// Display All Responses Pole
	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *ShowAllGoupResponsesUserWidget);
	ShowResponcesWidgetInstance->RemoveFromParent();
	CreatedWidgetInstance->AddToViewport();
	ShowAllGoupResponsesWidgetInstance = Cast<UShowAllGoupResponsesUserWidget>(CreatedWidgetInstance);
	ShowAllGoupResponsesWidgetInstance->ShowPrompts(AllGamePrompts);
	SendPlayerPole();

	TSharedPtr<FJsonObject> JsonObjectBeingSent = MakeShareable(new FJsonObject);
	JsonObjectBeingSent->SetNumberField("Stage", 10);
	GameInstance->SendJsonObject(JsonObjectBeingSent);

	TSharedPtr<FJsonObject> JsonObjectAllFragments = MakeShareable(new FJsonObject);
	for (int32 i = 0; i < AllGamePrompts.Num(); ++i) {
		FGamePrompt GamePrompts = AllGamePrompts[i];

		FString PromptFragmentOneKey = FString::Printf(TEXT("promptFragmentOne%d"), i);
		FString PromptFragmentOneResponceKey = FString::Printf(TEXT("promptFragmentOneResponce%d"), i);
		FString PromptFragmentOnePlayerIdKey = FString::Printf(TEXT("promptFragmentOnePlayerId%d"), i);
		FString PromptFragmentTwoKey = FString::Printf(TEXT("promptFragmentTwo%d"), i);
		FString PromptFragmentTwoResponceKey = FString::Printf(TEXT("promptFragmentTwoResponce%d"), i);
		FString PromptFragmentTwoPlayerIdKey = FString::Printf(TEXT("promptFragmentTwoPlayerId%d"), i);

		FEncapsule SentenceFragments = GamePrompts.SentenceFragments;
		JsonObjectAllFragments->SetStringField(PromptFragmentOneKey, SentenceFragments.SentenceFragmentThree);
		JsonObjectAllFragments->SetStringField(PromptFragmentOneResponceKey, SentenceFragments.SentenceFragmentThreeResponce);
		JsonObjectAllFragments->SetStringField(PromptFragmentOnePlayerIdKey, GamePrompts.FragmentThreePlayerId);
		JsonObjectAllFragments->SetStringField(PromptFragmentTwoKey, SentenceFragments.SentenceFragmentFour);
		JsonObjectAllFragments->SetStringField(PromptFragmentTwoResponceKey, SentenceFragments.SentenceFragmentFourResponce);
		JsonObjectAllFragments->SetStringField(PromptFragmentTwoPlayerIdKey, GamePrompts.FragmentFourPlayerId);
	}

	GameInstance->SendJsonObject(JsonObjectAllFragments);
}

void ATalkBoxActTwoPawn::EndRound() {
	GetWorld()->GetTimerManager().ClearTimer(GameTimerHandle);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("Stage", 11);
	GameInstance->SendJsonObject(JsonObject);

	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *ShowResponcesUserWidget);
	TimerWidgetInstance->RemoveFromParent();
	CreatedWidgetInstance->AddToViewport();
	ShowResponcesWidgetInstance = Cast<UShowResponsesUserWidget>(CreatedWidgetInstance);
	ShowResponcesWidgetInstance->ShowPrompts(AllGamePrompts);
	SendPlayerPole();
}

// Called to bind functionality to input
void ATalkBoxActTwoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ATalkBoxActTwoPawn::SendPlayersSentenceFragments() {
	// Check if the GameInstance is valid
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("GameInstance is not valid."));
		return;
	}

	int32 NumPlayers = GameInstance->AllPlayerInfo.Num();
	AllGamePrompts = GameInstance->AllGamePrompts;

	GameInstance->AllPlayerInfo.GetKeys(AllPlayerIds);
	FRandomStream RandomStream;
	ShuffleArray(RandomStream, AllPlayerIds);

	// Generate game prompts for each player
	for (int32 i = 0; i < NumPlayers; i++) {
		AllGamePrompts[i].FragmentThreePlayerId = AllPlayerIds[i];
		AllGamePrompts[i].FragmentFourPlayerId = AllPlayerIds[(i + 1) % NumPlayers];
	}

	// Send Game Prompts
	for (int32 i = 0; i < NumPlayers; i++) {
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField("promptOneFragmentOne", AllGamePrompts[i].SentenceFragments.SentenceFragmentThree);
		JsonObject->SetStringField("promptOneFragmentOnePlayerId", AllGamePrompts[i].FragmentThreePlayerId);
		JsonObject->SetStringField("promptOneFragmentTwo", AllGamePrompts[i].SentenceFragments.SentenceFragmentFour);
		JsonObject->SetStringField("promptOneFragmentTwoPlayerId", AllGamePrompts[i].FragmentFourPlayerId);

		JsonObject->SetStringField("promptTwoFragmentOne", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentThree);
		JsonObject->SetStringField("promptTwoFragmentOnePlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentThreePlayerId);
		JsonObject->SetStringField("promptTwoFragmentTwo", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentFour);
		JsonObject->SetStringField("promptTwoFragmentTwoPlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentFourPlayerId);

		JsonObject->SetStringField("partner", AllGamePrompts[i].FragmentThreePlayerId);
		JsonObject->SetStringField("playerName", AllGamePrompts[i].FragmentFourPlayerId);
		GameInstance->SendJsonObject(JsonObject);
	}
}

void ATalkBoxActTwoPawn::PromptResponceUserInputPromptOne(TSharedPtr<FJsonObject> JsonObject, FString playerName)
{
	FString userInputPromptOne;
	if (!JsonObject->TryGetStringField(TEXT("userInputPromptOne"), userInputPromptOne))
	{
		return;
	}

	FString promptOneFragmentOne;
	JsonObject->TryGetStringField(TEXT("promptOneFragmentOne"), promptOneFragmentOne);

	FString promptTwoFragmentOne;
	JsonObject->TryGetStringField(TEXT("promptTwoFragmentOne"), promptTwoFragmentOne);

	for (FGamePrompt& GamePrompt : AllGamePrompts) {
		if (promptOneFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentThree ||
			promptTwoFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentThree) {
			if (GamePrompt.SentenceFragments.SentenceFragmentThreeResponce.IsEmpty()) {
				UE_LOG(LogTemp, Warning, TEXT("userInputPromptOne Submitted"));
				GamePrompt.SentenceFragments.SentenceFragmentThreeResponce = userInputPromptOne;
				break;
			}
		}
	}

	PromptReadyUp(playerName);
}

void ATalkBoxActTwoPawn::PromptResponceUserInputPromptTwo(TSharedPtr<FJsonObject> JsonObject, FString playerName)
{
	FString userInputPromptTwo;
	if (!JsonObject->TryGetStringField(TEXT("userInputPromptTwo"), userInputPromptTwo))
	{
		return;
	}

	FString promptOneFragmentTwo;
	JsonObject->TryGetStringField(TEXT("promptOneFragmentTwo"), promptOneFragmentTwo);

	FString promptTwoFragmentTwo;
	JsonObject->TryGetStringField(TEXT("promptTwoFragmentTwo"), promptTwoFragmentTwo);

	for (FGamePrompt& GamePrompt : AllGamePrompts) {
		if (promptOneFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentFour ||
			promptTwoFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentFour) {
			if (GamePrompt.SentenceFragments.SentenceFragmentFourResponce.IsEmpty()) {
				UE_LOG(LogTemp, Warning, TEXT("userInputPromptTwo Submitted"));
				GamePrompt.SentenceFragments.SentenceFragmentFourResponce = userInputPromptTwo;
				break;
			}
		}
	}

	PromptReadyUp(playerName);
}

void ATalkBoxActTwoPawn::PromptReadyUp(FString playerName)
{
	ReadyPlayerCount++;

	// ReadyPlayerCount should be twice the number of players due to double submissions
	if (ReadyPlayerCount == GameInstance->AllPlayerInfo.Num() * 2)
	{
		if (!ShowResponcesUserWidget) {
			UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
			return;
		}

		EndRound();
	}
}

void ATalkBoxActTwoPawn::SendPlayerPole()
{
	if (AllGamePrompts.Num() == ShowResponcesWidgetInstance->index) return;

	const FGamePrompt& GamePrompt = AllGamePrompts[ShowResponcesWidgetInstance->index];
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	FEncapsule SentenceFragments = GamePrompt.SentenceFragments;
	JsonObject->SetStringField("Option1", SentenceFragments.SentenceFragmentThreeResponce);
	JsonObject->SetStringField("Option2", SentenceFragments.SentenceFragmentFourResponce);
	GameInstance->SendJsonObject(JsonObject);

	CurrentPoleVoteTotals.Reset();
}

template<typename Type>
void ATalkBoxActTwoPawn::ShuffleArray(FRandomStream& Stream, TArray<Type>& Array) {
	const int32 NumElements = Array.Num();

	for (int32 i = 0; i < NumElements; ++i) {
		int32 NewIndex = (i + 1) % NumElements;
		Array.Swap(i, NewIndex);
	}
}