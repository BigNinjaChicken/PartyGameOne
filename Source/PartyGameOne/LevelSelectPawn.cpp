// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelSelectPawn.h"

// Sets default values
ALevelSelectPawn::ALevelSelectPawn()
{

}

// Called when the game starts or when spawned
void ALevelSelectPawn::BeginPlay()
{
	Super::BeginPlay();

	if (!LevelSelectUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
		return;
	}

	UUserWidget* WidgetInstance = CreateWidget(GetWorld(), *LevelSelectUserWidget);
	WidgetInstance->AddToViewport();
}

// Called every frame
void ALevelSelectPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ALevelSelectPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

