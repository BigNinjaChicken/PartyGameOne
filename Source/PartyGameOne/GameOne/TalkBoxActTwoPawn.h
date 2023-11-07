// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TalkBoxPawn.h"
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

	void UpdateScoreOnDevice(FString clientId);
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
	template<typename Type> void ShuffleArray(FRandomStream& Stream, TArray<Type>& Array);
	int ReadyPlayerCount = 0;
	TArray<FString> AllPlayerIds;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UShowResponsesUserWidget> ShowResponcesUserWidget;
	UShowResponsesUserWidget* ShowResponcesWidgetInstance;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UShowAllGoupResponsesUserWidget> ShowAllGoupResponsesUserWidget;
	UShowAllGoupResponsesUserWidget* ShowAllGoupResponsesWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
	float InputPromptTime = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
	float InputPromptSafetyTime = 2.0f;

	bool bPostEnterPrompt = false;
	int32 ReadyUpPostEnterPrompts = 0;

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
};
