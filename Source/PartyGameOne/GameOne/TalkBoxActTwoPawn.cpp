// Fill out your copyright notice in the Description page of Project Settings.


#include "TalkBoxActTwoPawn.h"
#include "WebSocketGameInstance.h"
#include "TimerUserWidget.h"
#include <Dom/JsonObject.h>
#include <GameFramework/Actor.h>

// Sets default values
ATalkBoxActTwoPawn::ATalkBoxActTwoPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATalkBoxActTwoPawn::BeginPlay()
{
	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}

	GameInstance->WebSocket->OnMessage().AddUObject(this, &ATalkBoxActTwoPawn::OnWebSocketRecieveMessage);

	if (!TimerUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
		return;
	}

	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *TimerUserWidget);
	CreatedWidgetInstance->AddToViewport();
	TimerWidgetInstance = Cast<UTimerUserWidget>(CreatedWidgetInstance);
	TimerWidgetInstance->StartTimer(InputPromptTime);

	GetWorld()->GetTimerManager().SetTimer(GameTimerHandle, this, &ATalkBoxActTwoPawn::EndRound, InputPromptTime, false);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("Stage", "TalkBoxActTwo");
	GameInstance->SendJsonObject(JsonObject);
}

// Called every frame
void ATalkBoxActTwoPawn::Tick(float DeltaTime)
{
	
}

void ATalkBoxActTwoPawn::OnWebSocketRecieveMessage(const FString& MessageString) {

}

void ATalkBoxActTwoPawn::EndRound() {

}

// Called to bind functionality to input
void ATalkBoxActTwoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

