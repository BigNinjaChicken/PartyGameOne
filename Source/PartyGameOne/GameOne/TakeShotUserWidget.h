// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TakeShotUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEONE_API UTakeShotUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateReadyPlayers(const TMap<FString, bool>& ReadyMap);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateReadyPlayers(const TMap<FString, bool>& ReadyMap);
};
