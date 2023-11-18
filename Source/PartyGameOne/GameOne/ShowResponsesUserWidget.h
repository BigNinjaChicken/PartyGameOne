// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TalkBoxActOnePawn.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShowResponsesUserWidget.generated.h"

// Define a multicast delegate type (no parameters in this case)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWinnerDisplayedDelegate);

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

	UFUNCTION(BlueprintImplementableEvent, Category = "Progress")
	void DisplayWinner(const FString& winner, bool bOption1Wins);

	UPROPERTY(BlueprintAssignable, BlueprintCallable, EditAnywhere, Category = "Events")
    FOnWinnerDisplayedDelegate OnWinnerDisplayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = State)
    int index = 0;
};
