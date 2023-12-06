// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TalkBoxPawn.generated.h"

USTRUCT(BlueprintType)
struct FEncapsule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SentenceFragmentOne;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SentenceFragmentOneResponce;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SentenceFragmentTwo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SentenceFragmentTwoResponce;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FragOneTwoGroupPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SentenceFragmentThree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SentenceFragmentThreeResponce;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SentenceFragmentFour;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SentenceFragmentFourResponce;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FragThreeFourGroupPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FString> SentenceFragmentFive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SentenceFragmentFiveResponce;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FString> SentenceFragmentSix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SentenceFragmentSixResponce;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FragFiveSixGroupPoints = 0;
};


USTRUCT(BlueprintType)
struct FGamePrompt
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FEncapsule SentenceFragments;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FragmentOnePlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FragmentTwoPlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FragmentThreePlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FragmentFourPlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FragmentFivePlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FragmentSixPlayerId;
};

USTRUCT(BlueprintType)
struct FPoleVoteTotals
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Option1Votes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Option2Votes = 0;

	void Reset()
	{
		Option1Votes = 0;
		Option2Votes = 0;
	}

	int TotalVotes() {
		return Option1Votes + Option2Votes;
	}
};

UCLASS()
class PARTYGAMEONE_API ATalkBoxPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATalkBoxPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SendPlayersSentenceFragments();

	void EndRound();

	void OnWebSocketRecieveMessage(const FString& MessageString);

	void UpdateScoreOnDevice(FString playerName);

	void ReceivePlayerAllPoleVote(TSharedPtr<FJsonObject> JsonObject);

	void RecievedPlayerPoleVote(TSharedPtr<FJsonObject> JsonObject);

	UFUNCTION()
	void OnWinnerDisplayed();

	void PromptResponceUserInputPromptTwo(TSharedPtr<FJsonObject> JsonObject, FString playerName);

	void PromptReadyUp(FString playerName);

	void PromptResponceUserInputPromptOne(TSharedPtr<FJsonObject> JsonObject, FString playerName);

	void SendPlayerPole();

	void CreateSentencePossibility(FString FragmentOne, FString FragmentTwo, FString FragmentThree, FString FragmentFour);

	template<typename Type>
	void ShuffleArray(FRandomStream& Stream, TArray<Type>& Array);
public:
	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UTimerUserWidget> TimerUserWidget;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UShowResponsesUserWidget> ShowResponcesUserWidget;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UShowAllGoupResponsesUserWidget> ShowAllGoupResponsesUserWidget;

	UTimerUserWidget* TimerWidgetInstance;
	UShowResponsesUserWidget* ShowResponcesWidgetInstance;
	UShowAllGoupResponsesUserWidget* ShowAllGoupResponsesWidgetInstance;

	class UWebSocketGameInstance* GameInstance;
	TArray<FString> AllPlayerIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
	float InputPromptTime = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
	float InputPromptSafetyTime = 2.0f;

	bool bPostEnterPrompt = false;
	int32 ReadyUpPostEnterPrompts = 0;

	TArray<FEncapsule> SentencePossibilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game, meta = (AllowPrivateAccess = "true"))
	TArray<FGamePrompt> AllGamePrompts;

	UPROPERTY()
	FTimerHandle GameTimerHandle;

	int ReadyPlayerCount = 0;

	int32 RoundEndReadyUpCount = 0;

	FPoleVoteTotals CurrentPoleVoteTotals;
	
	int32 TotalOptionsInputed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> ScoreboardLevel;

	UPROPERTY(EditAnywhere)
	int32 ActNumber = 1;

	UPROPERTY(EditAnywhere)
	int32 PromptStageNumber = 4;

	UPROPERTY(EditAnywhere)
	int32 PollStageNumber = 5;

	UPROPERTY(EditAnywhere)
	int32 AllPoleStageNumber = 6;

	bool bRoundEnded = false;
};
