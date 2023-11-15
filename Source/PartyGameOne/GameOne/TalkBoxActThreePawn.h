// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TalkBoxPawn.h"
#include "TalkBoxActThreePawn.generated.h"

UCLASS()
class PARTYGAMEONE_API ATalkBoxActThreePawn : public ATalkBoxPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATalkBoxActThreePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void SendPlayersSentenceFragments();

	virtual void EndRound();

	virtual void OnWebSocketRecieveMessage(const FString& MessageString);

	virtual void OnWinnerDisplayed();

	virtual void UpdateScoreOnDevice(FString playerName);

	virtual void ReceivePlayerAllPoleVote(TSharedPtr<FJsonObject> JsonObject);

	virtual void RecievedPlayerPoleVote(TSharedPtr<FJsonObject> JsonObject);

	virtual void PromptResponceUserInputPromptTwo(TSharedPtr<FJsonObject> JsonObject, FString playerName);

	virtual void PromptReadyUp(FString playerName);

	virtual void PromptResponceUserInputPromptOne(TSharedPtr<FJsonObject> JsonObject, FString playerName);

	virtual void SendPlayerPole();
};
