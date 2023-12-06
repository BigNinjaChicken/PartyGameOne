// Fill out your copyright notice in the Description page of Project Settings.


#include "TutorialPawn.h"
#include <Engine/World.h>
#include <Kismet/GameplayStatics.h>
#include <Dom/JsonObject.h>
#include "WebSocketGameInstance.h"

// Sets default values
ATutorialPawn::ATutorialPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATutorialPawn::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(TutorialTimerHandle, this, &ATutorialPawn::StartGame, TutorialTotalTime, false);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("Stage", TutorialStageNum);
	GameInstance->SendJsonObject(JsonObject);

	if (bSkipTutorial) {
		StartGame();
	}
}

// Called every frame
void ATutorialPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATutorialPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// The function to call after the delay
void ATutorialPawn::StartGame()
{
	if (TalkBoxLevel.IsNull()) {
		UE_LOG(LogTemp, Error, TEXT("Invalid TutorialLevel"));
		return;
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), TalkBoxLevel);
}

