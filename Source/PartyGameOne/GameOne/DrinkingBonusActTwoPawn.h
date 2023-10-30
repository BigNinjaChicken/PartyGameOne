// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DrinkingBonusActTwoPawn.generated.h"

UCLASS()
class PARTYGAMEONE_API ADrinkingBonusActTwoPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADrinkingBonusActTwoPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void OnWebSocketRecieveMessage(const FString& MessageString);

	void ApplySelectedMultiplier(TSharedPtr<FJsonObject> JsonObject);

	class UWebSocketGameInstance* GameInstance;

	int32 NumPlayersGettingSelecction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> TalkBoxActThreeLevel;

};
