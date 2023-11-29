// Fill out your copyright notice in the Description page of Project Settings.


#include "JoinGamePawn.h"
#include "WebSocketGameInstance.h"
#include "JsonUtilities.h"
#include "StartScreenUserWidget.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AJoinGamePawn::AJoinGamePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AJoinGamePawn::BeginPlay()
{
    Super::BeginPlay();

    // Bind the OnWebSocketConnected function to the WebSocketConnectedDelegate
    GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
    if (!GameInstance) {
        return;
    }

    GameInstance->WebSocket->OnConnected().AddUObject(this, &AJoinGamePawn::OnWebSocketConnected);
	GameInstance->WebSocket->OnMessage().AddUObject(this, &AJoinGamePawn::OnWebSocketRecieveMessage);
    if (GameInstance->WebSocket->IsConnected()) {
        OnWebSocketConnected();
    }

	if (!StartScreenUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
		return;
	}

	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *StartScreenUserWidget);
    CreatedWidgetInstance->AddToViewport();
    WidgetInstance = Cast<UStartScreenUserWidget>(CreatedWidgetInstance);

}

void AJoinGamePawn::OnWebSocketConnected()
{
    if (bWebSocketConnectedHasRun) {
        return;
    }
    bWebSocketConnectedHasRun = true;

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("gameType", "GameOne");
    GameInstance->SendJsonObject(JsonObject);
}

void AJoinGamePawn::OnWebSocketRecieveMessage(const FString& MessageString)
{
    // Convert the FString to a JSON object
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MessageString);

    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        UE_LOG(LogTemp, Warning, TEXT("Deserialize Failed"))
            return;
    }

    FString PlayerName = GameInstance->GetJsonChildrenString(JsonObject, "clientInfo", "playerName");
    if (PlayerName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid Player Name"));
        return;
    }

    int32 difficultySelected;
    if (JsonObject->TryGetNumberField(TEXT("difficultySelected"), difficultySelected)) {
        // Check if the difficultySelected key exists in the map
        if (PlayerSelectedDifficultyMap.Contains(difficultySelected)) {
            // If it exists, increment its value
            PlayerSelectedDifficultyMap[difficultySelected]++;
        }
        else {
            // If it doesn't exist, add the key with a value of 1
            PlayerSelectedDifficultyMap.Add(difficultySelected, 1);
        }
    }

    // Ready Up
    bool bIsReady;
    if (JsonObject->TryGetBoolField(TEXT("bIsReady"), bIsReady))
    {
        PlayerReadyMap.Add(PlayerName, bIsReady);

        for (const auto& PlayerEntry : PlayerReadyMap)
        {
            FString PlayerName2 = PlayerEntry.Key;
            bool IsReady = PlayerEntry.Value;

            UE_LOG(LogTemp, Warning, TEXT("PlayerName: %s, IsReady: %s"), *PlayerName2, IsReady ? TEXT("True") : TEXT("False"));
        }

        // Don't start unless enough players
        if (PlayerReadyMap.Num() < MinPlayerCount) {
            UE_LOG(LogTemp, Warning, TEXT("Invalid Player Count"));
            return;
        }

        // Check if everyone is ready
        for (const auto& PlayerEntry : PlayerReadyMap)
        {
            if (!PlayerEntry.Value)
            {
                return;
            }
        }

        // Everyone is ready!
        /*
        Democracy Voting for difficulty
        int32 AmountSelectedDifficulty = TNumericLimits<int32>::Min();
        int32 VotedDifficulty = 0; 

        for (const auto& SelectedDifficulty : PlayerSelectedDifficultyMap)
        {
            if (SelectedDifficulty.Value > AmountSelectedDifficulty)
            {
                AmountSelectedDifficulty = SelectedDifficulty.Value;
                VotedDifficulty = SelectedDifficulty.Key;
            }
        }

        GameInstance->DifficultyLevel = VotedDifficulty;
        */

        // Average Difficulty
        float VotedDifficulty = 0;
        float index = 0;
        for (const auto& SelectedDifficulty : PlayerSelectedDifficultyMap)
        {
            index++;
            VotedDifficulty += SelectedDifficulty.Key;
            
        }
        GameInstance->DifficultyLevel = FMath::RoundToInt(VotedDifficulty / index);

        UE_LOG(LogTemp, Warning, TEXT("Select difficulty is %d"), GameInstance->DifficultyLevel);

        if (TutorialLevel.IsNull()) {
            UE_LOG(LogTemp, Error, TEXT("Invalid TutorialLevel"));
            return;
        }

        UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), TutorialLevel);
        return;
    }

    bool bJoinedGame;
    if (JsonObject->TryGetBoolField(TEXT("bJoinedGame"), bJoinedGame))
    {
        if (PlayerReadyMap.Contains(PlayerName)) {
            return;
        }
        if (WidgetInstance)
        {
            WidgetInstance->AddPlayer(PlayerName);
            PlayerReadyMap.Add(PlayerName, false); // Not Ready Yet

            PlayerJoined(PlayerName);

            FPlayerInfo NewPlayerInfo;
            NewPlayerInfo.Score = 0;
            GameInstance->AllPlayerInfo.Add(PlayerName, NewPlayerInfo);
        }
    }

    FString ChatMessage;
    if (JsonObject->TryGetStringField(TEXT("ChatMessage"), ChatMessage)) {
        DisplayChatMessage(PlayerName, ChatMessage);
    }
}


// Called every frame
void AJoinGamePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AJoinGamePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}