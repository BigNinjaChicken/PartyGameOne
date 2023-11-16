// Fill out your copyright notice in the Description page of Project Settings.


#include "TalkBoxActThreePawn.h"
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
ATalkBoxActThreePawn::ATalkBoxActThreePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATalkBoxActThreePawn::BeginPlay()
{
	Super::BeginPlay();

	GameInstance->WebSocket->OnMessage().AddUObject(this, &ATalkBoxActThreePawn::OnWebSocketRecieveMessage);

	if (!TimerUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
		return;
	}

	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *TimerUserWidget);
	CreatedWidgetInstance->AddToViewport();
	TimerWidgetInstance = Cast<UTimerUserWidget>(CreatedWidgetInstance);
	TimerWidgetInstance->StartTimer(InputPromptTime - InputPromptSafetyTime);

	GetWorld()->GetTimerManager().SetTimer(GameTimerHandle, this, &ATalkBoxActThreePawn::EndRound, InputPromptTime, false);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("Stage", 15);
	GameInstance->SendJsonObject(JsonObject);

	SendPlayersSentenceFragments();

	for (auto Player : GameInstance->AllPlayerInfo) {
		TSharedPtr<FJsonObject> JsonObjectTime = MakeShareable(new FJsonObject);
		JsonObjectTime->SetStringField("Timer", FString::FromInt(InputPromptTime / Player.Value.ScoreMultiplier));
		JsonObjectTime->SetStringField("playerName", Player.Key);
		GameInstance->SendJsonObject(JsonObjectTime);
	}
}

// Called every frame
void ATalkBoxActThreePawn::Tick(float DeltaTime)
{

}

void ATalkBoxActThreePawn::UpdateScoreOnDevice(FString playerName) {
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("Score", GameInstance->AllPlayerInfo[playerName].Score);
	JsonObject->SetStringField("playerName", playerName);
	GameInstance->SendJsonObject(JsonObject);
}

void ATalkBoxActThreePawn::OnWebSocketRecieveMessage(const FString& MessageString) {
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

void ATalkBoxActThreePawn::ReceivePlayerAllPoleVote(TSharedPtr<FJsonObject> JsonObject)
{
	uint64 option;
	if (JsonObject->TryGetNumberField(TEXT("option"), option))
	{
		AllGamePrompts[option].SentenceFragments.FragFiveSixGroupPoints += 150;
		GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentFivePlayerId].Score += 300 * GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentFivePlayerId].ScoreMultiplier;
		GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentSixPlayerId].Score += 300 * GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentSixPlayerId].ScoreMultiplier;
		UpdateScoreOnDevice(AllGamePrompts[option].FragmentFivePlayerId);
		UpdateScoreOnDevice(AllGamePrompts[option].FragmentSixPlayerId);
		
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

void ATalkBoxActThreePawn::RecievedPlayerPoleVote(TSharedPtr<FJsonObject> JsonObject)
{
	FString poleSelection;
	if (JsonObject->TryGetStringField(TEXT("poleSelection"), poleSelection))
	{
		if (poleSelection == "Option1") {
			CurrentPoleVoteTotals.Option1Votes++;
			GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFivePlayerId].Score += 200 * GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFivePlayerId].ScoreMultiplier;
			UpdateScoreOnDevice(AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFivePlayerId);
		}
		else if (poleSelection == "Option2") {
			CurrentPoleVoteTotals.Option2Votes++;
			GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentSixPlayerId].Score += 200 * GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentSixPlayerId].ScoreMultiplier;
			UpdateScoreOnDevice(AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentSixPlayerId);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Invalid Option"))
		}

		if (CurrentPoleVoteTotals.TotalVotes() == AllPlayerIds.Num()) {
			// All players have submitted a vote

			// Determine the winner of the current round and display it
			FString winnerPlayerId;
			if (CurrentPoleVoteTotals.Option1Votes > CurrentPoleVoteTotals.Option2Votes) {
				winnerPlayerId = AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFivePlayerId;
			}
			else {
				winnerPlayerId = AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentSixPlayerId;
			}

			// Display the winner on the ShowResponcesWidgetInstance
			ShowResponcesWidgetInstance->DisplayWinner(winnerPlayerId, CurrentPoleVoteTotals.Option1Votes > CurrentPoleVoteTotals.Option2Votes);

			// Callback or event after winner is displayed
			ShowResponcesWidgetInstance->OnWinnerDisplayed.AddDynamic(this, &ATalkBoxActThreePawn::OnWinnerDisplayed);
		}
	}
}

void ATalkBoxActThreePawn::OnWinnerDisplayed() {
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
	JsonObjectBeingSent->SetNumberField("Stage", 16);
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
		JsonObjectAllFragments->SetStringField(PromptFragmentOneKey, SentenceFragments.SentenceFragmentFive[0] + " " + SentenceFragments.SentenceFragmentFive[1]);
		JsonObjectAllFragments->SetStringField(PromptFragmentOneResponceKey, SentenceFragments.SentenceFragmentFiveResponce);
		JsonObjectAllFragments->SetStringField(PromptFragmentOnePlayerIdKey, GamePrompts.FragmentFivePlayerId);
		JsonObjectAllFragments->SetStringField(PromptFragmentTwoKey, SentenceFragments.SentenceFragmentSix[0] + " " + SentenceFragments.SentenceFragmentSix[1]);
		JsonObjectAllFragments->SetStringField(PromptFragmentTwoResponceKey, SentenceFragments.SentenceFragmentSixResponce);
		JsonObjectAllFragments->SetStringField(PromptFragmentTwoPlayerIdKey, GamePrompts.FragmentSixPlayerId);
	}

	GameInstance->SendJsonObject(JsonObjectAllFragments);
}

void ATalkBoxActThreePawn::EndRound() {
	GetWorld()->GetTimerManager().ClearTimer(GameTimerHandle);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("Stage", 17);
	GameInstance->SendJsonObject(JsonObject);

	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *ShowResponcesUserWidget);
	TimerWidgetInstance->RemoveFromParent();
	CreatedWidgetInstance->AddToViewport();
	ShowResponcesWidgetInstance = Cast<UShowResponsesUserWidget>(CreatedWidgetInstance);
	ShowResponcesWidgetInstance->ShowPrompts(AllGamePrompts);
	SendPlayerPole();
}

// Called to bind functionality to input
void ATalkBoxActThreePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ATalkBoxActThreePawn::SendPlayersSentenceFragments() {
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
		AllGamePrompts[i].FragmentFivePlayerId = AllPlayerIds[i];
		AllGamePrompts[i].FragmentSixPlayerId = AllPlayerIds[(i + 1) % NumPlayers];
	}

	for (FGamePrompt& GamePrompt : AllGamePrompts) {
		GamePrompt.SentenceFragments.SentenceFragmentFive.SetNum(2);
		GamePrompt.SentenceFragments.SentenceFragmentFive[0] = GamePrompt.SentenceFragments.SentenceFragmentOne;
		GamePrompt.SentenceFragments.SentenceFragmentFive[1] = GamePrompt.SentenceFragments.SentenceFragmentOneResponce;
		GamePrompt.SentenceFragments.SentenceFragmentSix.SetNum(2);
		GamePrompt.SentenceFragments.SentenceFragmentSix[0] = GamePrompt.SentenceFragments.SentenceFragmentTwo;
		GamePrompt.SentenceFragments.SentenceFragmentSix[1] = GamePrompt.SentenceFragments.SentenceFragmentTwoResponce;
	}

	// Send Game Prompts
	for (int32 i = 0; i < NumPlayers; i++) {
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField("promptOneFragmentOne1", AllGamePrompts[i].SentenceFragments.SentenceFragmentFive[0]);
		JsonObject->SetStringField("promptOneFragmentOne2", AllGamePrompts[i].SentenceFragments.SentenceFragmentFive[1]);
		JsonObject->SetStringField("promptOneFragmentOnePlayerId", AllGamePrompts[i].FragmentFivePlayerId);
		JsonObject->SetStringField("promptOneFragmentTwo1", AllGamePrompts[i].SentenceFragments.SentenceFragmentSix[0]);
		JsonObject->SetStringField("promptOneFragmentTwo2", AllGamePrompts[i].SentenceFragments.SentenceFragmentSix[1]);
		JsonObject->SetStringField("promptOneFragmentTwoPlayerId", AllGamePrompts[i].FragmentSixPlayerId);

		JsonObject->SetStringField("promptTwoFragmentOne1", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentFive[0]);
		JsonObject->SetStringField("promptTwoFragmentOne2", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentFive[1]);
		JsonObject->SetStringField("promptTwoFragmentOnePlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentFivePlayerId);
		JsonObject->SetStringField("promptTwoFragmentTwo1", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentSix[0]);
		JsonObject->SetStringField("promptTwoFragmentTwo2", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentSix[1]);
		JsonObject->SetStringField("promptTwoFragmentTwoPlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentSixPlayerId);

		JsonObject->SetStringField("partner", AllGamePrompts[i].FragmentFivePlayerId);
		JsonObject->SetStringField("playerName", AllGamePrompts[i].FragmentSixPlayerId);
		GameInstance->SendJsonObject(JsonObject);
	}
}

void ATalkBoxActThreePawn::PromptResponceUserInputPromptOne(TSharedPtr<FJsonObject> JsonObject, FString playerName)
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
		if (promptOneFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentFive[0] ||
			promptTwoFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentFive[0]) {
			if (GamePrompt.SentenceFragments.SentenceFragmentFiveResponce.IsEmpty()) {
				UE_LOG(LogTemp, Warning, TEXT("userInputPromptOne Submitted"));
				GamePrompt.SentenceFragments.SentenceFragmentFiveResponce = userInputPromptOne;
				break;
			}
		}
	}

	PromptReadyUp(playerName);
}

void ATalkBoxActThreePawn::PromptResponceUserInputPromptTwo(TSharedPtr<FJsonObject> JsonObject, FString playerName)
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
		if (promptOneFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentSix[0] ||
			promptTwoFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentSix[0]) {
			if (GamePrompt.SentenceFragments.SentenceFragmentSixResponce.IsEmpty()) {
				UE_LOG(LogTemp, Warning, TEXT("userInputPromptTwo Submitted"));
				GamePrompt.SentenceFragments.SentenceFragmentSixResponce = userInputPromptTwo;
				break;
			}
		}
	}

	PromptReadyUp(playerName);
}

void ATalkBoxActThreePawn::PromptReadyUp(FString playerName)
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

void ATalkBoxActThreePawn::SendPlayerPole()
{
	if (AllGamePrompts.Num() == ShowResponcesWidgetInstance->index) return;

	const FGamePrompt& GamePrompt = AllGamePrompts[ShowResponcesWidgetInstance->index];
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	FEncapsule SentenceFragments = GamePrompt.SentenceFragments;
	JsonObject->SetStringField("Option1", SentenceFragments.SentenceFragmentFiveResponce);
	JsonObject->SetStringField("Option2", SentenceFragments.SentenceFragmentSixResponce);
	GameInstance->SendJsonObject(JsonObject);

	CurrentPoleVoteTotals.Reset();
}