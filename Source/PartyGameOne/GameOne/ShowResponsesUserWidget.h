// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TalkBoxPawn.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShowResponsesUserWidget.generated.h"

/**
 *
 */
UCLASS()
class PARTYGAMEONE_API UShowResponsesUserWidget : public UUserWidget
{
    GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category = "Progress")
    void ShowPrompts(const TArray<FGamePrompt>& AllGamePrompts, int32 ActNumber = 1);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = State)
    int index = 0;
};
