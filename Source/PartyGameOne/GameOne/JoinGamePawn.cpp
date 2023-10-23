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

    if (CurrentGameState == EGameState::JoinGame) {
        TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
        JsonObject->SetStringField("gameType", "GameOne");
        GameInstance->SendJsonObject(JsonObject);
    }
}

void AJoinGamePawn::OnWebSocketRecieveMessage(const FString& MessageString)
{
    // Convert the FString to a JSON object
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MessageString);

    if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        if (CurrentGameState == EGameState::JoinGame) {
            // Display Session Code
            FString SessionCode;
            if (JsonObject->TryGetStringField(TEXT("sessionCode"), SessionCode))
            {
                if (!WidgetInstance) {
                    UE_LOG(LogTemp, Error, TEXT("Invalid WidgetInstance"));
                    return;
                }
                WidgetInstance->SetSessionCode = FText::FromString(SessionCode);
                return;
            }

            FString PlayerName = GameInstance->GetJsonChildrenString(JsonObject, "clientInfo", "playerName");
            if (PlayerName.IsEmpty()) {
                UE_LOG(LogTemp, Error, TEXT("Invalid Player Name"));
                return;
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

                if (TutorialLevel.IsNull()) {
                    UE_LOG(LogTemp, Error, TEXT("Invalid TutorialLevel"));
                    return;
                }
				UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), TutorialLevel);
                CurrentGameState = EGameState::Tutorial;
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

                    FPlayerInfo NewPlayerInfo;
                    NewPlayerInfo.PlayerName = GameInstance->GetJsonChildrenString(JsonObject, "clientInfo", "playerName");
                    FString PlayerId = GameInstance->GetJsonChildrenString(JsonObject, "clientInfo", "clientId");
                    GameInstance->AllPlayerInfo.Add(PlayerId, NewPlayerInfo);
                }
            }
        }
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
