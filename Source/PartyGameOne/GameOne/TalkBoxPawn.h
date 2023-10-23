// Copyright (C) [Year] [Your Name]

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

	void CreateSentencePossibility(FString FragmentOne, FString FragmentTwo);

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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SendPlayersSentenceFragments();

	void EndRound();
	void OnWebSocketRecieveMessage(const FString& MessageString);

	void ReceivePlayerAllPoleVote(TSharedPtr<FJsonObject> JsonObject);

	void RecievedPlayerPoleVote(TSharedPtr<FJsonObject> JsonObject);

	void PromptResponceUserInputPromptTwo(TSharedPtr<FJsonObject> JsonObject, FString clientId);

	void PromptReadyUp(FString clientId);

	void PromptResponceUserInputPromptOne(TSharedPtr<FJsonObject> JsonObject, FString clientId);

	float InputPromptTime = 50.0f;

	TArray<FEncapsule> SentencePossibilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game, meta = (AllowPrivateAccess = "true"))
	TArray<FGamePrompt> AllGamePrompts;

	UPROPERTY()
	FTimerHandle GameTimerHandle;

	int ReadyPlayerCount = 0;

	UFUNCTION(BlueprintCallable)
	void SendPlayerPole(const FGamePrompt& GamePrompts);

	FPoleVoteTotals CurrentPoleVoteTotals;
	
	int32 TotalOptionsInputed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> ScoreboardLevel;
};
template<typename Type> static void ShuffleArray(FRandomStream& Stream, TArray<Type>& Array);