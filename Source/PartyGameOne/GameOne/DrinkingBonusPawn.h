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

	void OnWebSocketRecieveMessage(const FString& MessageString);

	void ApplySelectedMultiplier(TSharedPtr<FJsonObject> JsonObject);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	class UWebSocketGameInstance* GameInstance;

	int32 NumPlayersGettingSelecction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> TalkBoxActTwoLevel;

};
