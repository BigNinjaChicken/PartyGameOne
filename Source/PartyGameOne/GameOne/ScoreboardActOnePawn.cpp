// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreboardActOnePawn.h"
#include <TimerManager.h>
#include <Dom/JsonObject.h>
#include <Kismet/GameplayStatics.h>
#include "WebSocketGameInstance.h"
#include "ShowResponsesUserWidget.h"
#include "ScoreboardUserWidget.h"

// Sets default values
AScoreboardActOnePawn::AScoreboardActOnePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AScoreboardActOnePawn::BeginPlay()
{
	Super::BeginPlay();

    GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
    if (!GameInstance) {
        UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
        return;
    }

    // Step 1: Create and fill an array with pairs of player name and score
    TArray<TPair<FString, int32>> PlayerScores;
    for (const TPair<FString, FPlayerInfo>& Player : GameInstance->AllPlayerInfo) {
        PlayerScores.Emplace(Player.Key, Player.Value.Score);
    }

    // Step 2: Sort the array based on scores
    PlayerScores.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B) {
        return A.Value > B.Value; // Sort in descending order
        });

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    TMap<FString, int32> TopPlayerScores;
    int32 Standing = 1;
    for (const TPair<FString, int32>& PlayerScore : PlayerScores) {
        TSharedPtr<FJsonObject> JsonObjectStage = MakeShareable(new FJsonObject);
        JsonObjectStage->SetNumberField("Stage", 7);
        JsonObjectStage->SetStringField("playerName", PlayerScore.Key);
        GameInstance->SendJsonObject(JsonObjectStage);

        JsonObject->SetStringField("playerName", PlayerScore.Key);
        JsonObject->SetNumberField("playerScore", PlayerScore.Value);
        JsonObject->SetNumberField("Standing", Standing++);

        // Send the JSON object
        GameInstance->SendJsonObject(JsonObject);

        if (Standing < 4) TopPlayerScores.Add(PlayerScore.Key, PlayerScore.Value);
    }

    if (!ScoreboardUserWidget) {
        UE_LOG(LogTemp, Error, TEXT("Failed to get ScoreboardUserWidget"));
        return;
    }

    UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *ScoreboardUserWidget);
    CreatedWidgetInstance->AddToViewport();
    ScoreboardWidgetInstance = Cast<UScoreboardUserWidget>(CreatedWidgetInstance);
    ScoreboardWidgetInstance->DisplayTopScoreboard(TopPlayerScores);

    if (GameInstance->DifficultyLevel == 1) {
        OpenNextLevel(TalkBoxLevel);
    }
    else {
        OpenNextLevel(DrinkingBonusLevel);
    }
}

// Called every frame
void AScoreboardActOnePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AScoreboardActOnePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AScoreboardActOnePawn::SendStageNumToPlayer()
{
    TSharedPtr<FJsonObject> JsonObjectStage = MakeShareable(new FJsonObject);
    JsonObjectStage->SetNumberField("Stage", NextStageNumber);
    GameInstance->SendJsonObject(JsonObjectStage);
}

void AScoreboardActOnePawn::OpenNextLevel(TSoftObjectPtr<UWorld> Level)
{
    UE_LOG(LogTemp, Warning, TEXT("Start Timer"));
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    TimerManager.SetTimer(TutorialTimerHandle, [this, Level]() {
        UE_LOG(LogTemp, Warning, TEXT("Finish Timer"));

        SendStageNumToPlayer();

        if (Level.IsNull()) {
            UE_LOG(LogTemp, Error, TEXT("Invalid Level"));
            return;
        }

        // Open next level
        UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), Level);

        }, ScoreboardTime, false);
}


