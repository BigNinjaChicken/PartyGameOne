// Fill out your copyright notice in the Description page of Project Settings.


#include "GameOneTalkBoxPawn.h"
#include "TimerUserWidget.h"
#include "JsonUtilities.h"
#include "WebSocketGameInstance.h"
#include "ShowResponsesUserWidget.h"
#include "ShowAllGoupResponsesUserWidget.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AGameOneTalkBoxPawn::AGameOneTalkBoxPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void AGameOneTalkBoxPawn::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}

	GameInstance->AllPlayerInfo.GetKeys(AllPlayerIds);

	CreateSentencePossibility("The dragon", "until the warrior suddenly");
	CreateSentencePossibility("My ex-wife", "the other day, now I'm");
	CreateSentencePossibility("Did you hear about", "? I've heard they");
	CreateSentencePossibility("I pulled out some mints, when", "then my friends asked, ");
	CreateSentencePossibility("I was walking when", "my dog suddenly");
	CreateSentencePossibility("I could only have", "So I grabbed a giant");
	CreateSentencePossibility("I had a great day", "Now I'm not allowed to");
	CreateSentencePossibility("Only one day till", "then we gotta deal with");
	CreateSentencePossibility("I am going to", " and then I'm gonna");
	CreateSentencePossibility("They had to ban", "after the incident when");

	GameInstance->WebSocket->OnMessage().AddUObject(this, &AGameOneTalkBoxPawn::OnWebSocketRecieveMessage);

	if (!TimerUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
		return;
	}

	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *TimerUserWidget);
	CreatedWidgetInstance->AddToViewport();
	TimerWidgetInstance = Cast<UTimerUserWidget>(CreatedWidgetInstance);
	TimerWidgetInstance->StartTimer(30.0f);

	GetWorld()->GetTimerManager().SetTimer(GameTimerHandle, this, &AGameOneTalkBoxPawn::EndRound, 30.0f, false);
	
	SendPlayersSentenceFragments();
}

void AGameOneTalkBoxPawn::CreateSentencePossibility(FString FragmentOne, FString FragmentTwo)
{
	FEncapsule Item;
	Item.SentenceFragmentOne = FragmentOne;
	Item.SentenceFragmentTwo = FragmentTwo;
	SentencePossibilities.Add(Item);
}

// Called every frame
void AGameOneTalkBoxPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGameOneTalkBoxPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGameOneTalkBoxPawn::SendPlayersSentenceFragments() {
	// Check if the GameInstance is valid
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("GameInstance is not valid."));
		return;
	}

	FRandomStream RandomStream;
	ShuffleArray(RandomStream, AllPlayerIds);

	int32 NumPlayers = AllPlayerIds.Num();

	// Check if there are enough SentencePossibilities
	if (SentencePossibilities.Num() < NumPlayers) {
		UE_LOG(LogTemp, Warning, TEXT("Not enough SentencePossibilities for all players."));
		return;
	}

	AllGamePrompts.Empty(); // Clear the array
	// Generate game prompts for each player
	for (int32 i = 0; i < NumPlayers; i++) {
		FEncapsule SentenceElement = SentencePossibilities[FMath::RandRange(0, SentencePossibilities.Num() - 1)];

		FGamePrompts Item;
		Item.FragmentOnePlayerId = AllPlayerIds[i];
		Item.FragmentTwoPlayerId = AllPlayerIds[(i + 1) % NumPlayers];
		Item.SentenceFragments = SentenceElement;

		AllGamePrompts.Add(Item);
		SentencePossibilities.RemoveAtSwap(FMath::RandRange(0, SentencePossibilities.Num() - 1), 1, false);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Successfully Connected");
	}
	// Send Game Prompts
	for (int32 i = 0; i < NumPlayers; i++) {
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField("promptOneFragmentOne", AllGamePrompts[i].SentenceFragments.SentenceFragmentOne);
		JsonObject->SetStringField("promptOneFragmentOnePlayerId", AllGamePrompts[i].FragmentOnePlayerId);
		JsonObject->SetStringField("promptOneFragmentTwo", AllGamePrompts[i].SentenceFragments.SentenceFragmentTwo);
		JsonObject->SetStringField("promptOneFragmentTwoPlayerId", AllGamePrompts[i].FragmentTwoPlayerId);

		JsonObject->SetStringField("promptTwoFragmentOne", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentOne);
		JsonObject->SetStringField("promptTwoFragmentOnePlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentOnePlayerId);
		JsonObject->SetStringField("promptTwoFragmentTwo", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentTwo);
		JsonObject->SetStringField("promptTwoFragmentTwoPlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentTwoPlayerId);

		JsonObject->SetStringField("clientId", AllPlayerIds[i]);
		GameInstance->SendJsonObject(JsonObject);
	}

	// Create a duplicate readiness map for the ready up stage
	for (const FString& PlayerId : AllPlayerIds)
	{
		PlayerReadinessMap.Add(PlayerId, false);
	}
}


void AGameOneTalkBoxPawn::EndRound() {

}

void AGameOneTalkBoxPawn::OnWebSocketRecieveMessage(const FString& MessageString) {
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

	uint64 option;
	if (JsonObject->TryGetNumberField(TEXT("option"), option))
	{
		GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentOnePlayerId].Score += 300;
		GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentTwoPlayerId].Score += 300;
		TotalOptionsInputed++;

		if (TotalOptionsInputed == GameInstance->AllPlayerInfo.Num()) {
			if (ScoreboardLevel.IsNull()) {
				UE_LOG(LogTemp, Error, TEXT("Invalid ScoreboardLevel"));
				return;
			}
			UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), ScoreboardLevel);
			return;
		}

		return;
	}

	ReceivedPlayerPromptResponces(JsonObject, clientId);

	RecievedPlayerPoleVote(JsonObject);
}

void AGameOneTalkBoxPawn::RecievedPlayerPoleVote(TSharedPtr<FJsonObject> JsonObject)
{
	FString poleSelection;
	if (JsonObject->TryGetStringField(TEXT("poleSelection"), poleSelection))
	{
		if (poleSelection == "Option1") {
			CurrentPoleVoteTotals.Option1Votes++;
			GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentOnePlayerId].Score += 200;
		}
		else if (poleSelection == "Option2") {
			CurrentPoleVoteTotals.Option2Votes++;
			GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentTwoPlayerId].Score += 200;
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Invalid Option"))
		}

		if (CurrentPoleVoteTotals.TotalVotes() == AllPlayerIds.Num()) {
			// All players have submitted a vote
			ShowResponcesWidgetInstance->index++;
			ShowResponcesWidgetInstance->ShowPrompts(this); // Display next prompt

			if (ShowResponcesWidgetInstance->index == AllGamePrompts.Num()) {
				if (!ShowAllGoupResponsesUserWidget) {
					UE_LOG(LogTemp, Error, TEXT("ShowAllGoupResponsesUserWidget null"));
					return;
				}

				// Display All Responses Pole
				UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *ShowAllGoupResponsesUserWidget);
				ShowResponcesWidgetInstance->RemoveFromViewport();
				CreatedWidgetInstance->AddToViewport();
				ShowAllGoupResponsesWidgetInstance = Cast<UShowAllGoupResponsesUserWidget>(CreatedWidgetInstance);
				ShowAllGoupResponsesWidgetInstance->ShowPrompts(this);

				TSharedPtr<FJsonObject> JsonObjectAllFragments = MakeShareable(new FJsonObject);
				for (int32 i = 0; i < AllGamePrompts.Num(); ++i) {
					FGamePrompts GamePrompts = AllGamePrompts[i];

					FString PromptFragmentOneKey = FString::Printf(TEXT("promptFragmentOne%d"), i);
					FString PromptFragmentOneResponceKey = FString::Printf(TEXT("promptFragmentOneResponce%d"), i);
					FString PromptFragmentOnePlayerIdKey = FString::Printf(TEXT("promptFragmentOnePlayerId%d"), i);
					FString PromptFragmentTwoKey = FString::Printf(TEXT("promptFragmentTwo%d"), i);
					FString PromptFragmentTwoResponceKey = FString::Printf(TEXT("promptFragmentTwoResponce%d"), i);
					FString PromptFragmentTwoPlayerIdKey = FString::Printf(TEXT("promptFragmentTwoPlayerId%d"), i);

					JsonObjectAllFragments->SetStringField(PromptFragmentOneKey, GamePrompts.SentenceFragments.SentenceFragmentOne);
					JsonObjectAllFragments->SetStringField(PromptFragmentOneResponceKey, GamePrompts.SentenceFragments.SentenceFragmentOneResponce);
					JsonObjectAllFragments->SetStringField(PromptFragmentOnePlayerIdKey, GamePrompts.FragmentOnePlayerId);
					JsonObjectAllFragments->SetStringField(PromptFragmentTwoKey, GamePrompts.SentenceFragments.SentenceFragmentTwo);
					JsonObjectAllFragments->SetStringField(PromptFragmentTwoResponceKey, GamePrompts.SentenceFragments.SentenceFragmentTwoResponce);
					JsonObjectAllFragments->SetStringField(PromptFragmentTwoPlayerIdKey, GamePrompts.FragmentTwoPlayerId);
				}

				JsonObjectAllFragments->SetBoolField("ShowAllPrompts", true);
				GameInstance->SendJsonObject(JsonObjectAllFragments);
			}
		}

		return;
	}
}

void AGameOneTalkBoxPawn::ReceivedPlayerPromptResponces(TSharedPtr<FJsonObject> JsonObject, FString clientId)
{
	FString userInputPromptOne;
	if (JsonObject->TryGetStringField(TEXT("userInputPromptOne"), userInputPromptOne))
	{
		for (FGamePrompts& GamePrompt : AllGamePrompts) {
			if (clientId == GamePrompt.FragmentOnePlayerId) {
				GamePrompt.SentenceFragments.SentenceFragmentOneResponce = userInputPromptOne;
			}
		}

		return;
	}

	FString userInputPromptTwo;
	if (JsonObject->TryGetStringField(TEXT("userInputPromptTwo"), userInputPromptTwo))
	{
		for (FGamePrompts& GamePrompt : AllGamePrompts) {
			if (clientId == GamePrompt.FragmentTwoPlayerId) {
				GamePrompt.SentenceFragments.SentenceFragmentTwoResponce = userInputPromptTwo;
			}
		}


		// Ready Up
		PlayerReadinessMap.Add(clientId, true);

		bool bAreAllPlayersReady = true;
		for (const auto& Pair : PlayerReadinessMap)
		{
			if (!Pair.Value)
			{
				bAreAllPlayersReady = false;
				break; // Break the loop as soon as a player is not ready
			}
		}

		if (bAreAllPlayersReady)
		{
			// All players are ready (true)
			if (!ShowResponcesUserWidget) {
				UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
				return;
			}

			UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *ShowResponcesUserWidget);
			TimerWidgetInstance->RemoveFromViewport();
			CreatedWidgetInstance->AddToViewport();
			ShowResponcesWidgetInstance = Cast<UShowResponsesUserWidget>(CreatedWidgetInstance);
			ShowResponcesWidgetInstance->ShowPrompts(this);
		}

		return;
	}
}

void AGameOneTalkBoxPawn::SendPlayerPole(const FGamePrompts& GamePrompts)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("Option1", GamePrompts.SentenceFragments.SentenceFragmentOneResponce);
	JsonObject->SetStringField("Option2", GamePrompts.SentenceFragments.SentenceFragmentTwoResponce);
	GameInstance->SendJsonObject(JsonObject);

	CurrentPoleVoteTotals.Reset();
}

template<typename Type>
static void ShuffleArray(FRandomStream& Stream, TArray<Type>& Array) {
	const int32 LastIndex = Array.Num() - 1;

	for (int32 i = 0; i <= LastIndex; i += 1) {
		const int32 Index = Stream.RandRange(i, LastIndex);
		if (i == Index) {
			continue;
		}

		Array.Swap(i, Index);
	}
}