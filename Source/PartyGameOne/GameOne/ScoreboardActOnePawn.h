// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ScoreboardActOnePawn.generated.h"

UCLASS()
class PARTYGAMEONE_API AScoreboardActOnePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AScoreboardActOnePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SendStageNumToPlayer(int StageNumber);

	UFUNCTION(BlueprintCallable)
	void OpenNextLevel(TSoftObjectPtr<UWorld> Level);

	UPROPERTY()
	FTimerHandle TutorialTimerHandle;

	UPROPERTY(EditAnywhere)
	float ScoreboardTime = 5.0f;

	UPROPERTY(EditAnywhere)
	int32 ScoreboardStageNumber = 7;

	class UWebSocketGameInstance* GameInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> DrinkingBonusLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> TalkBoxLevel;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UScoreboardUserWidget> ScoreboardUserWidget;

	UScoreboardUserWidget* ScoreboardWidgetInstance;
};
