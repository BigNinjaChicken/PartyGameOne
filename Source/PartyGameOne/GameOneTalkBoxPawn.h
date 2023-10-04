// Copyright (C) [Year] [Your Name]

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameOneTalkBoxPawn.generated.h"

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
struct FGamePrompts
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FEncapsule SentenceFragments;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FragmentOnePlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FragmentTwoPlayerId;
};

UCLASS()
class PARTYGAMEONE_API AGameOneTalkBoxPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGameOneTalkBoxPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CreateSentencePossibility(FString FragmentOne, FString FragmentTwo);

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UTimerUserWidget> TimerUserWidget;

	UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UShowResponsesUserWidget> ShowResponcesUserWidget;

	UTimerUserWidget* TimerWidgetInstance;
	UShowResponsesUserWidget* ShowResponcesWidgetInstance;

	class UWebSocketGameInstance* GameInstance;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void StartGame();

	void EndRound();
	void OnWebSocketRecieveMessage(const FString& MessageString);
	TArray<FEncapsule> SentencePossibilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game, meta = (AllowPrivateAccess = "true"))
	TArray<FGamePrompts> AllGamePrompts;

	UPROPERTY()
	FTimerHandle GameTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Game, meta = (AllowPrivateAccess = "true"))
    TMap<FString, bool> PlayerReadinessMap;
};
template<typename Type> static void ShuffleArray(FRandomStream& Stream, TArray<Type>& Array);