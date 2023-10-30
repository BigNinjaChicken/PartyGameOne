// Fill out your copyright notice in the Description page of Project Settings.

#include "TalkBoxPawn.h"
#include "TimerUserWidget.h"
#include "JsonUtilities.h"
#include "WebSocketGameInstance.h"
#include "ShowResponsesUserWidget.h"
#include "ShowAllGoupResponsesUserWidget.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
ATalkBoxPawn::ATalkBoxPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void ATalkBoxPawn::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}

	GameInstance->AllPlayerInfo.GetKeys(AllPlayerIds);

	CreateSentencePossibility("The dragon", "until the warrior suddenly", "My ex-wife", "the other day, now I'm");
	CreateSentencePossibility("Did you hear about", "? I've heard they", "I pulled out some mints, when", "then my friends asked, ");
	CreateSentencePossibility("I was walking when", "my dog suddenly", "I could only have", "So I grabbed a giant");
	CreateSentencePossibility("I had a great day", "Now I'm not allowed to", "Only one day till", "then we gotta deal with");
	CreateSentencePossibility("I am going to", " and then I'm gonna", "They had to ban", "after the incident when");

	GameInstance->WebSocket->OnMessage().AddUObject(this, &ATalkBoxPawn::OnWebSocketRecieveMessage);

	if (!TimerUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
		return;
	}

	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *TimerUserWidget);
	CreatedWidgetInstance->AddToViewport();
	TimerWidgetInstance = Cast<UTimerUserWidget>(CreatedWidgetInstance);
	TimerWidgetInstance->StartTimer(InputPromptTime);

	GetWorld()->GetTimerManager().SetTimer(GameTimerHandle, this, &ATalkBoxPawn::EndRound, InputPromptTime, false);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("Stage", "TalkBox");
	GameInstance->SendJsonObject(JsonObject);

	SendPlayersSentenceFragments();
}

void ATalkBoxPawn::CreateSentencePossibility(FString FragmentOne, FString FragmentTwo, FString FragmentThree, FString FragmentFour)
{
	FEncapsule Item;
	Item.SentenceFragmentOne = FragmentOne;
	Item.SentenceFragmentTwo = FragmentTwo;
	Item.SentenceFragmentThree = FragmentThree;
	Item.SentenceFragmentFour = FragmentFour;
	SentencePossibilities.Add(Item);
}

// Called every frame
void ATalkBoxPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATalkBoxPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ATalkBoxPawn::SendPlayersSentenceFragments() {
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

	// Generate game prompts for each player
	for (int32 i = 0; i < NumPlayers; i++) {
		int index = FMath::RandRange(0, SentencePossibilities.Num() - 1);
		FEncapsule SentenceElement = SentencePossibilities[index];

		FGamePrompt Item;
		Item.FragmentOnePlayerId = AllPlayerIds[i];
		Item.FragmentTwoPlayerId = AllPlayerIds[(i + 1) % NumPlayers];
		Item.SentenceFragments = SentenceElement;

		AllGamePrompts.Add(Item);
		SentencePossibilities.RemoveAt(index);
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

		JsonObject->SetStringField("clientId", AllGamePrompts[i].FragmentTwoPlayerId); // Fix Play-Test
		GameInstance->SendJsonObject(JsonObject);
	}
}


void ATalkBoxPawn::EndRound() {

}

void ATalkBoxPawn::OnWebSocketRecieveMessage(const FString& MessageString) {
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

void ATalkBoxPawn::ReceivePlayerAllPoleVote(TSharedPtr<FJsonObject> JsonObject)
{
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
			GameInstance->AllGamePrompts = AllGamePrompts;
			UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), ScoreboardLevel);
			return;
		}
	}
}

void ATalkBoxPawn::RecievedPlayerPoleVote(TSharedPtr<FJsonObject> JsonObject)
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
			ShowResponcesWidgetInstance->ShowPrompts(AllGamePrompts); // Display next prompt
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
					JsonObjectAllFragments->SetStringField(PromptFragmentOneKey, SentenceFragments.SentenceFragmentOne);
					JsonObjectAllFragments->SetStringField(PromptFragmentOneResponceKey, SentenceFragments.SentenceFragmentOneResponce);
					JsonObjectAllFragments->SetStringField(PromptFragmentOnePlayerIdKey, GamePrompts.FragmentOnePlayerId);
					JsonObjectAllFragments->SetStringField(PromptFragmentTwoKey, SentenceFragments.SentenceFragmentTwo);
					JsonObjectAllFragments->SetStringField(PromptFragmentTwoResponceKey, SentenceFragments.SentenceFragmentTwoResponce);
					JsonObjectAllFragments->SetStringField(PromptFragmentTwoPlayerIdKey, GamePrompts.FragmentTwoPlayerId);
				}

				GameInstance->SendJsonObject(JsonObjectAllFragments);
			}
		}

		return;
	}
}

void ATalkBoxPawn::PromptResponceUserInputPromptOne(TSharedPtr<FJsonObject> JsonObject, FString clientId)
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
				(promptOneFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentOne ||
					promptTwoFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentOne)) {
				UE_LOG(LogTemp, Warning, TEXT("userInputPromptOne Submitted"));
				GamePrompt.SentenceFragments.SentenceFragmentOneResponce = userInputPromptOne;
				break;
			}
		}
	}

	PromptReadyUp(clientId);
}

void ATalkBoxPawn::PromptResponceUserInputPromptTwo(TSharedPtr<FJsonObject> JsonObject, FString clientId)
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
			(promptOneFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentTwo ||
				promptTwoFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentTwo)) {
			UE_LOG(LogTemp, Warning, TEXT("userInputPromptTwo Submitted"));
			GamePrompt.SentenceFragments.SentenceFragmentTwoResponce = userInputPromptTwo;
			break;
		}
	}

	PromptReadyUp(clientId);
}

void ATalkBoxPawn::PromptReadyUp(FString clientId)
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
		ShowResponcesWidgetInstance->ShowPrompts(AllGamePrompts);
		SendPlayerPole();
	}
}

void ATalkBoxPawn::SendPlayerPole()
{
	if (AllGamePrompts.Num() == ShowResponcesWidgetInstance->index) return;

	const FGamePrompt& GamePrompt = AllGamePrompts[ShowResponcesWidgetInstance->index];
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	FEncapsule SentenceFragments = GamePrompt.SentenceFragments;
	JsonObject->SetStringField("Option1", SentenceFragments.SentenceFragmentOneResponce);
	JsonObject->SetStringField("Option2", SentenceFragments.SentenceFragmentTwoResponce);
	GameInstance->SendJsonObject(JsonObject);

	CurrentPoleVoteTotals.Reset();
}

template<typename Type>
void ATalkBoxPawn::ShuffleArray(FRandomStream& Stream, TArray<Type>& Array) {
	const int32 LastIndex = Array.Num() - 1;

	for (int32 i = 0; i <= LastIndex; i += 1) {
		const int32 Index = Stream.RandRange(i, LastIndex);
		if (i == Index) {
			continue;
		}

		Array.Swap(i, Index);
	}
}