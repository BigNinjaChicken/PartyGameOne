// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameOneTalkBoxPawn.h"

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
    void ShowPrompts(AGameOneTalkBoxPawn* TalkBoxPawn);
};
