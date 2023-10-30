// Fill out your copyright notice in the Description page of Project Settings.


#include "TalkBoxActTwoPawn.h"
#include "WebSocketGameInstance.h"
#include "TimerUserWidget.h"
#include <Dom/JsonObject.h>
#include <GameFramework/Actor.h>
#include "TalkBoxPawn.h"
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
	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance."));
		return;
	}

	GameInstance->WebSocket->OnMessage().AddUObject(this, &ATalkBoxActTwoPawn::OnWebSocketRecieveMessage);

	if (!TimerUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
		return;
	}

	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *TimerUserWidget);
	CreatedWidgetInstance->AddToViewport();
	TimerWidgetInstance = Cast<UTimerUserWidget>(CreatedWidgetInstance);
	TimerWidgetInstance->StartTimer(InputPromptTime);

	GetWorld()->GetTimerManager().SetTimer(GameTimerHandle, this, &ATalkBoxActTwoPawn::EndRound, InputPromptTime, false);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("Stage", "TalkBoxActTwo");
	GameInstance->SendJsonObject(JsonObject);

	SendPlayersSentenceFragments();
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

	FString clientId = GameInstance->GetJsonChildrenString(JsonObject, "clientInfo", "clientId");
	if (clientId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("invalid cliendId"))
			return;
	}

	ReceivePlayerAllPoleVote(JsonObject);

	PromptResponceUserInputPromptOne(JsonObject, clientId);

	PromptResponceUserInputPromptTwo(JsonObject, clientId);

	RecievedPlayerPoleVote(JsonObject);
}

void ATalkBoxActTwoPawn::ReceivePlayerAllPoleVote(TSharedPtr<FJsonObject> JsonObject)
{
	uint64 option;
	if (JsonObject->TryGetNumberField(TEXT("option"), option))
	{
		GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentThreePlayerId].Score += 300 * GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentThreePlayerId].ScoreMultiplier;
		GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentFourPlayerId].Score += 300 * GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentFourPlayerId].ScoreMultiplier;
		TotalOptionsInputed++;

		if (TotalOptionsInputed == GameInstance->AllPlayerInfo.Num()) {
			if (ScoreboardLevel.IsNull()) {
				UE_LOG(LogTemp, Error, TEXT("Invalid ScoreboardLevel"));
				return;
			}
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
		}
		else if (poleSelection == "Option2") {
			CurrentPoleVoteTotals.Option2Votes++;
			GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFourPlayerId].Score += 200 * GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFourPlayerId].ScoreMultiplier;
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Invalid Option"))
		}

		if (CurrentPoleVoteTotals.TotalVotes() == AllPlayerIds.Num()) {
			// All players have submitted a vote
			ShowResponcesWidgetInstance->index++;
			ShowResponcesWidgetInstance->ShowPrompts(AllGamePrompts, 2); // Display next prompt
			SendPlayerPole();

			if (ShowResponcesWidgetInstance->index == AllGamePrompts.Num()) {
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
				JsonObjectBeingSent->SetStringField("Stage", "ShowAllPole");
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
		}

		return;
	}
}

void ATalkBoxActTwoPawn::EndRound() {

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

		JsonObject->SetStringField("clientId", AllGamePrompts[i].FragmentFourPlayerId);
		GameInstance->SendJsonObject(JsonObject);
	}
}

void ATalkBoxActTwoPawn::PromptResponceUserInputPromptOne(TSharedPtr<FJsonObject> JsonObject, FString clientId)
{
	FString userInputPromptOne;
	if (JsonObject->TryGetStringField(TEXT("userInputPromptOne"), userInputPromptOne))
	{
		if (userInputPromptOne.IsEmpty()) {
			return;
		}

		FString promptOneFragmentOne;
		JsonObject->TryGetStringField(TEXT("promptOneFragmentOne"), promptOneFragmentOne);

		FString promptTwoFragmentOne;
		JsonObject->TryGetStringField(TEXT("promptTwoFragmentOne"), promptTwoFragmentOne);

		for (FGamePrompt& GamePrompt : AllGamePrompts) {
			if (clientId == GamePrompt.FragmentOnePlayerId &&
				(promptOneFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentThree ||
					promptTwoFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentThree)) {
				UE_LOG(LogTemp, Warning, TEXT("userInputPromptOne Submitted"));
				GamePrompt.SentenceFragments.SentenceFragmentThreeResponce = userInputPromptOne;
				break;
			}
		}
	}

	PromptReadyUp(clientId);
}

void ATalkBoxActTwoPawn::PromptResponceUserInputPromptTwo(TSharedPtr<FJsonObject> JsonObject, FString clientId)
{
	FString userInputPromptTwo;
	if (JsonObject->TryGetStringField(TEXT("userInputPromptTwo"), userInputPromptTwo))
	{
		if (userInputPromptTwo.IsEmpty()) {
			return;
		}
	}

	FString promptOneFragmentTwo;
	JsonObject->TryGetStringField(TEXT("promptOneFragmentTwo"), promptOneFragmentTwo);

	FString promptTwoFragmentTwo;
	JsonObject->TryGetStringField(TEXT("promptTwoFragmentTwo"), promptTwoFragmentTwo);

	for (FGamePrompt& GamePrompt : AllGamePrompts) {
		if (clientId == GamePrompt.FragmentTwoPlayerId &&
			(promptOneFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentFour ||
				promptTwoFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentFour)) {
			UE_LOG(LogTemp, Warning, TEXT("userInputPromptTwo Submitted"));
			GamePrompt.SentenceFragments.SentenceFragmentFourResponce = userInputPromptTwo;
			break;
		}
	}

	PromptReadyUp(clientId);
}

void ATalkBoxActTwoPawn::PromptReadyUp(FString clientId)
{
	ReadyPlayerCount++;

	// ReadyPlayerCount should be twice the number of players due to double submissions
	if (ReadyPlayerCount == GameInstance->AllPlayerInfo.Num() * 2)
	{
		// All players are ready
		if (!ShowResponcesUserWidget) {
			UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
			return;
		}

		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField("Stage", "Pole");
		GameInstance->SendJsonObject(JsonObject);

		UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *ShowResponcesUserWidget);
		TimerWidgetInstance->RemoveFromParent();
		CreatedWidgetInstance->AddToViewport();
		ShowResponcesWidgetInstance = Cast<UShowResponsesUserWidget>(CreatedWidgetInstance);
		ShowResponcesWidgetInstance->ShowPrompts(AllGamePrompts, 2);
		SendPlayerPole();
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
	const int32 LastIndex = Array.Num() - 1;

	for (int32 i = 0; i <= LastIndex; i += 1) {
		const int32 Index = Stream.RandRange(i, LastIndex);
		if (i == Index) {
			continue;
		}

		Array.Swap(i, Index);
	}
}