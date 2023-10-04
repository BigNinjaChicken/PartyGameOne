// Fill out your copyright notice in the Description page of Project Settings.


#include "GameOneTalkBoxPawn.h"
#include "TimerUserWidget.h"
#include "JsonUtilities.h"
#include "WebSocketGameInstance.h"
#include "ShowResponsesUserWidget.h"

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

	GetWorld()->GetTimerManager().SetTimer(GameTimerHandle, this, &AGameOneTalkBoxPawn::EndRound, 30.0f, false);
	TimerWidgetInstance->StartTimer(30.0f);
	StartGame();
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

void AGameOneTalkBoxPawn::StartGame() {
	// Check if the GameInstance is valid
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("GameInstance is not valid."));
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(GameTimerHandle);

	TArray<FString> AllPlayerIds = GameInstance->AllPlayerIds;
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
	// Convert the FString to a JSON object
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