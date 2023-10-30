// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TalkBoxPawn.h"
#include "TalkBoxActThreePawn.generated.h"

UCLASS()
class PARTYGAMEONE_API ATalkBoxActThreePawn : public APawn
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

	void OnWebSocketRecieveMessage(const FString& MessageString);
	void ReceivePlayerAllPoleVote(TSharedPtr<FJsonObject> JsonObject);
	void RecievedPlayerPoleVote(TSharedPtr<FJsonObject> JsonObject);
	void EndRound();
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SendPlayersSentenceFragments();

	void PromptResponceUserInputPromptOne(TSharedPtr<FJsonObject> JsonObject, FString clientId);
	void PromptResponceUserInputPromptTwo(TSharedPtr<FJsonObject> JsonObject, FString clientId);
	void PromptReadyUp(FString clientId);

	void SendPlayerPole();
	int ReadyPlayerCount = 0;
	TArray<FString> AllPlayerIds;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UShowResponsesUserWidget> ShowResponcesUserWidget;
	UShowResponsesUserWidget* ShowResponcesWidgetInstance;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UShowAllGoupResponsesUserWidget> ShowAllGoupResponsesUserWidget;
	UShowAllGoupResponsesUserWidget* ShowAllGoupResponsesWidgetInstance;

	UPROPERTY(EditAnywhere)
	float InputPromptTime = 50.0f;

	class UWebSocketGameInstance* GameInstance;

	TArray<FGamePrompt> AllGamePrompts;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UTimerUserWidget> TimerUserWidget;
	UTimerUserWidget* TimerWidgetInstance;

	FPoleVoteTotals CurrentPoleVoteTotals;

	int32 TotalOptionsInputed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> ScoreboardLevel;

	UPROPERTY()
	FTimerHandle GameTimerHandle;

	template<typename Type> void ShuffleArray(FRandomStream& Stream, TArray<Type>& Array);
};
