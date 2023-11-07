// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TalkBoxPawn.h"

#include "FinalStoryPawn.generated.h"

UCLASS()
class PARTYGAMEONE_API AFinalStoryPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFinalStoryPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void OnWebSocketRecieveMessage(const FString& MessageString);

	TArray<FGamePrompt> AllGamePrompts;
	class UWebSocketGameInstance* GameInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGamePrompt TopGamePrompt;

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayBestGamePrompt();

	//************************************
	// Method:    GetPlayerName
	// FullName:  AFinalStoryPawn::GetPlayerName
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: String clientId
	// 
	// Given a client ID this method will return the player name associated with that id
	//************************************
	UFUNCTION(BlueprintCallable)
	FString GetPlayerName(FString clientId);
};
