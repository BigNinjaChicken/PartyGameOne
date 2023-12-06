// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TakeShotPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEveryoneReady);

UCLASS()
class PARTYGAMEONE_API ATakeShotPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATakeShotPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnWebSocketRecieveMessage(const FString& MessageString);

	UFUNCTION(BlueprintCallable)
	void OpenNextLevel();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> NextLevel;

	class UWebSocketGameInstance* GameInstance;

	int32 PlayerAmountRecivedMult = 0;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UTakeShotUserWidget> TakeShotUserWidget;

	UTakeShotUserWidget* TakeShotUserWidgetInstance;

	TMap<FString, bool> ReadyMap;

	UPROPERTY(EditAnywhere, Category = State)
	int ClientStage = 9;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, EditAnywhere, Category = "Events")
	FOnEveryoneReady OnEveryoneReady;
};
