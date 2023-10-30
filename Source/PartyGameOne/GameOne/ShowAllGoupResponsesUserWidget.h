// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShowAllGoupResponsesUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEONE_API UShowAllGoupResponsesUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Progress")
	void ShowPrompts(const TArray<FGamePrompt>& AllGamePrompts);
};
