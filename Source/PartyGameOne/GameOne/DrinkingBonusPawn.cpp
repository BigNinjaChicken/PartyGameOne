// Fill out your copyright notice in the Description page of Project Settings.


#include "DrinkingBonusPawn.h"
#include "WebSocketGameInstance.h"
#include <Dom/JsonObject.h>
#include <Kismet/GameplayStatics.h>
#include <Misc/CString.h>
#include <Kismet/KismetStringLibrary.h>
#include <Misc/DefaultValueHelper.h>

// Sets default values
ADrinkingBonusPawn::ADrinkingBonusPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADrinkingBonusPawn::BeginPlay()
{
    Super::BeginPlay();

    GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
    if (!GameInstance) {
        UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
        return;
    }

    GameInstance->WebSocket->OnMessage().AddUObject(this, &ADrinkingBonusPawn::OnWebSocketRecieveMessage);

    // Create a struct to store player info
    struct FPlayerInfo
    {
        FString PlayerName;
        int32 Score;
    };

    // Collect player info into an array
    TArray<FPlayerInfo> PlayerInfos;

    for (const auto& PlayerInfo : GameInstance->AllPlayerInfo)
    {
        FPlayerInfo Info;
        Info.PlayerName = PlayerInfo.Key;
        Info.Score = PlayerInfo.Value.Score;
        PlayerInfos.Add(Info);
    }

    // Sort the player info array by score in ascending order
    PlayerInfos.Sort([](const FPlayerInfo& A, const FPlayerInfo& B)
        {
            return A.Score < B.Score;
        });

    int32 DifficultyLevel = GameInstance->DifficultyLevel;

    // Find the multiplier for the current difficulty level
    float Multiplier = DifficultyMultipliers.Contains(DifficultyLevel) ? DifficultyMultipliers[DifficultyLevel] : 0.3f;

    // Calculate the number of players to keep
    NumPlayersGettingSelection = FMath::CeilToInt(PlayerInfos.Num() * Multiplier);
    
    // Add a warning log to check the number of players to keep
    UE_LOG(LogTemp, Warning, TEXT("Number of players to keep: %d"), NumPlayersGettingSelection);

    // Reset Multipliers
    for (auto& PlayerInfo : GameInstance->AllPlayerInfo) {
        PlayerInfo.Value.ScoreMultiplier = 1.0f;
    }

    // Bottom 30% player IDs
    for (int32 i = 0; i < NumPlayersGettingSelection; i++)
    {
        FString PlayerName = PlayerInfos[i].PlayerName;

        TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
        JsonObject->SetStringField("playerName", PlayerName);

        int j = 0;
        for (const auto& PlayerInfo : GameInstance->AllPlayerInfo) {
            float RawRandomOffset = FMath::RandRange(RandomOffsetMinRange, RandomOffsetMaxRange);  // Generate a random float value in the range -0.2 to 0.2
            float RoundedRandomOffset = FMath::RoundToFloat(RawRandomOffset * 10.0f) / 10.0f;  // Round to the nearest first decimal point

            if (PlayerInfo.Key == PlayerName) {
                JsonObject->SetStringField("PlayerScoreBonusOption" + FString::FromInt(j), FString::SanitizeFloat(SelectSelfValue + RoundedRandomOffset));
            }
            else {
                JsonObject->SetStringField("PlayerScoreBonusOption" + FString::FromInt(j), FString::SanitizeFloat(SelectOtherPlayerValue + RoundedRandomOffset));
            }
            JsonObject->SetStringField("PlayerName" + FString::FromInt(j), PlayerInfo.Key);

            j++;
        }

        GameInstance->SendJsonObject(JsonObject);
    }

}

void ADrinkingBonusPawn::OnWebSocketRecieveMessage(const FString& MessageString) {
    // Create JSON object to be sent out
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MessageString);

    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        UE_LOG(LogTemp, Warning, TEXT("Deserialize Failed"));
        return;
    }

    ApplySelectedMultiplier(JsonObject);
    return;
}

void ADrinkingBonusPawn::ApplySelectedMultiplier(TSharedPtr<FJsonObject> JsonObject)
{
	FString selectedPlayerName;
	if (!JsonObject->TryGetStringField(TEXT("selectedPlayerName"), selectedPlayerName)) {
		UE_LOG(LogTemp, Warning, TEXT("selectedPlayerName null"))
			return;
	}

	FString multiplier;
	if (!JsonObject->TryGetStringField(TEXT("multiplier"), multiplier)) {
		UE_LOG(LogTemp, Warning, TEXT("multiplier null"))
			return;
	}

    float ScoreMultiplierValue;
    FDefaultValueHelper::ParseFloat(multiplier, ScoreMultiplierValue);
    GameInstance->AllPlayerInfo[selectedPlayerName].ScoreMultiplier = ScoreMultiplierValue;

    NumPlayersGettingSelection--;
    if (NumPlayersGettingSelection == 0) {
        if (TalkBoxActTwoLevel.IsNull()) {
            UE_LOG(LogTemp, Error, TEXT("Invalid TalkBoxActTwoLevel"));
            return;
        }
        UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), TalkBoxActTwoLevel);
    }
}

// Called every frame
void ADrinkingBonusPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADrinkingBonusPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

