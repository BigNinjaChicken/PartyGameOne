// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEONE_API UTimerUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, Category = Progress)
	void StartTimer(float TotalTimeSec);
};
