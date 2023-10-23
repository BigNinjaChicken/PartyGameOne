// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartScreenUserWidget.generated.h"

/**
 * 
 */

UCLASS()
class PARTYGAMEONE_API UStartScreenUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	FText SetSessionCode;

	UFUNCTION(BlueprintImplementableEvent, Category = UI)
    void AddPlayer(const FString& PlayerName);
};
