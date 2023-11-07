// Fill out your copyright notice in the Description page of Project Settings.


#include "FinalStoryPawn.h"
#include "WebSocketGameInstance.h"

// Sets default values
AFinalStoryPawn::AFinalStoryPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFinalStoryPawn::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance."));
		return;
	}

	GameInstance->WebSocket->OnMessage().AddUObject(this, &AFinalStoryPawn::OnWebSocketRecieveMessage);

	AllGamePrompts = GameInstance->AllGamePrompts;
	
	int32 TopGamePromptScore = 0;
	for (FGamePrompt GamePrompt : AllGamePrompts) {
		int32 GamePromptTotalScore = GamePrompt.SentenceFragments.FragFiveSixGroupPoints +
			GamePrompt.SentenceFragments.FragOneTwoGroupPoints +
			GamePrompt.SentenceFragments.FragThreeFourGroupPoints;
		
		if (GamePromptTotalScore > TopGamePromptScore) {
			TopGamePrompt = GamePrompt;
			TopGamePromptScore = GamePromptTotalScore;
		}
	}

	if (TopGamePromptScore == 0) {
		UE_LOG(LogTemp, Error, TEXT("Failed to find TopGamePrompt"));
	}

	DisplayBestGamePrompt();
}

// Called every frame
void AFinalStoryPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFinalStoryPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AFinalStoryPawn::OnWebSocketRecieveMessage(const FString& MessageString)
{

}

FString AFinalStoryPawn::GetPlayerName(FString clientId)
{
	return GameInstance->AllPlayerInfo[clientId].PlayerName;
}

