// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TalkBoxPawn.h"
#include "TalkBoxActTwoPawn.generated.h"

UCLASS()
class PARTYGAMEONE_API ATalkBoxActTwoPawn : public ATalkBoxPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATalkBoxActTwoPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CreateSentencePossibility(FString FragmentOne, FString FragmentTwo, FString FragmentThree, FString FragmentFour);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void SendPlayersSentenceFragments();

	virtual void EndRound();

	virtual void OnWinnerDisplayed();

	virtual void OnWebSocketRecieveMessage(const FString& MessageString);

	virtual void UpdateScoreOnDevice(FString playerName);

	virtual void ReceivePlayerAllPoleVote(TSharedPtr<FJsonObject> JsonObject);

	virtual void RecievedPlayerPoleVote(TSharedPtr<FJsonObject> JsonObject);

	virtual void PromptResponceUserInputPromptTwo(TSharedPtr<FJsonObject> JsonObject, FString playerName);

	virtual void PromptReadyUp(FString playerName);

	virtual void PromptResponceUserInputPromptOne(TSharedPtr<FJsonObject> JsonObject, FString playerName);

	virtual void SendPlayerPole();
};