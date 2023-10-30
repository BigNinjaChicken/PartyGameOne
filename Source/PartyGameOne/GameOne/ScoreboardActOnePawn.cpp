// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreboardActOnePawn.h"
#include <TimerManager.h>
#include <Dom/JsonObject.h>
#include <Kismet/GameplayStatics.h>
#include "WebSocketGameInstance.h"

// Sets default values
AScoreboardActOnePawn::AScoreboardActOnePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AScoreboardActOnePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AScoreboardActOnePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AScoreboardActOnePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AScoreboardActOnePawn::OpenNextLevel()
{
	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Start Timer"));
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(TutorialTimerHandle, [this]() {
		UE_LOG(LogTemp, Warning, TEXT("Finish Timer"));

		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField("Stage", "DrinkBonus");
		GameInstance->SendJsonObject(JsonObject);

		if (DrinkingBonusLevel.IsNull()) {
			UE_LOG(LogTemp, Error, TEXT("Invalid DrinkingBonusLevel"));
			return;
		}

		UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), DrinkingBonusLevel);

	}, ScoreboardTime, false);
}
