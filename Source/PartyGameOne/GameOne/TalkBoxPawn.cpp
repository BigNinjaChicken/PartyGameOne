// Fill out your copyright notice in the Description page of Project Settings.


#include "TalkBoxPawn.h"
#include "TimerUserWidget.h"
#include "WebSocketGameInstance.h"

// Sets default values
ATalkBoxPawn::ATalkBoxPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATalkBoxPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// Create GameInstance
	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}
	GameInstance->AllPlayerInfo.GetKeys(AllPlayerIds);

	// Create TimerUserWidget
	if (!TimerUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
		return;
	}
	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *TimerUserWidget);
	CreatedWidgetInstance->AddToViewport();
	TimerWidgetInstance = Cast<UTimerUserWidget>(CreatedWidgetInstance);
}

// Called every frame
void ATalkBoxPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATalkBoxPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

template<typename Type>
void ATalkBoxPawn::ShuffleArray(FRandomStream& Stream, TArray<Type>& Array) {
	// This is the correct shuffle method but resulted in people getting the same teammate
	const int32 LastIndex = Array.Num() - 1;

	for (int32 i = 0; i <= LastIndex; i += 1) {
		const int32 Index = Stream.RandRange(i, LastIndex);
		if (i == Index) {
			continue;
		}

		Array.Swap(i, Index);
	}
}

void ATalkBoxPawn::OnWinnerDisplayed()
{

}
