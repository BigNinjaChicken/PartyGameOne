// Fill out your copyright notice in the Description page of Project Settings.


#include "GameOneTutorialPawn.h"
#include <Engine/World.h>
#include <Kismet/GameplayStatics.h>
#include <Dom/JsonObject.h>
#include "WebSocketGameInstance.h"

// Sets default values
AGameOneTutorialPawn::AGameOneTutorialPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGameOneTutorialPawn::BeginPlay()
{
	Super::BeginPlay();


	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(TutorialTimerHandle, this, &AGameOneTutorialPawn::StartGame, 2.0f, false);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("Stage", "Tutorial");
	GameInstance->SendJsonObject(JsonObject);
}

// Called every frame
void AGameOneTutorialPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGameOneTutorialPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// The function to call after the delay
void AGameOneTutorialPawn::StartGame()
{
	if (TalkBoxLevel.IsNull()) {
		UE_LOG(LogTemp, Error, TEXT("Invalid TutorialLevel"));
		return;
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), TalkBoxLevel);
}

