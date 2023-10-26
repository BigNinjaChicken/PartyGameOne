// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TalkBoxActTwoPawn.generated.h"

UCLASS()
class PARTYGAMEONE_API ATalkBoxActTwoPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATalkBoxActTwoPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OnWebSocketRecieveMessage(const FString& MessageString);
	void EndRound();
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
	float InputPromptTime = 50.0f;

	class UWebSocketGameInstance* GameInstance;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UTimerUserWidget> TimerUserWidget;
	UTimerUserWidget* TimerWidgetInstance;

	UPROPERTY()
	FTimerHandle GameTimerHandle;
};
