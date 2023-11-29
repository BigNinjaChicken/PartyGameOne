// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DrinkingBonusPawn.generated.h"

UCLASS()
class PARTYGAMEONE_API ADrinkingBonusPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADrinkingBonusPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void OnWebSocketRecieveMessage(const FString& MessageString);

	void ApplySelectedMultiplier(TSharedPtr<FJsonObject> JsonObject);

public:
	class UWebSocketGameInstance* GameInstance;

	int32 NumPlayersGettingSelection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> TalkBoxActTwoLevel;

	UPROPERTY(EditAnywhere, Category = State)
	float RandomOffsetMinRange = -0.2f;

	UPROPERTY(EditAnywhere, Category = State)
	float RandomOffsetMaxRange = 0.2f;

	UPROPERTY(EditAnywhere, Category = State)
	float SelectSelfValue = 2.0f;

	UPROPERTY(EditAnywhere, Category = State)
	float SelectOtherPlayerValue = 3.0f;

	UPROPERTY(EditAnywhere, Category = State)
	float BottomPercentPlayersSelected = 0.3f;

	// Define a map for difficulty level multipliers
	UPROPERTY(EditAnywhere, Category = State)
	TMap<int32, float> DifficultyMultipliers = {
		{1, 0.2f},
		{2, 0.3f},
		{3, 0.5f}
		// Add more difficulty levels and their multipliers as needed
	};
};
