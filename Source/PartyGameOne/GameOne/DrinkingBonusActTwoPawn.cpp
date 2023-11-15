// Fill out your copyright notice in the Description page of Project Settings.


#include "DrinkingBonusActTwoPawn.h"
#include "WebSocketGameInstance.h"
#include <Dom/JsonObject.h>
#include <Kismet/GameplayStatics.h>
#include <Misc/CString.h>
#include <Kismet/KismetStringLibrary.h>

// Sets default values
ADrinkingBonusActTwoPawn::ADrinkingBonusActTwoPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADrinkingBonusActTwoPawn::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ADrinkingBonusActTwoPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADrinkingBonusActTwoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

