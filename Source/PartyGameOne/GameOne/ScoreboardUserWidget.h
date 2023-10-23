// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreboardUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEONE_API UScoreboardUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void OpenNextLevel();

	UPROPERTY()
	FTimerHandle TutorialTimerHandle;

	UPROPERTY(EditAnywhere)
	float ScoreboardTime = 5.0f;

	class UWebSocketGameInstance* GameInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> DrinkingBonusLevel;
};
