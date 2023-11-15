// Fill out your copyright notice in the Description page of Project Settings.


#include "TakeShotPawn.h"
#include <Kismet/GameplayStatics.h>
#include "WebSocketGameInstance.h"
#include <Serialization/JsonReader.h>
#include <Dom/JsonObject.h>
#include "TakeShotUserWidget.h"
#include <UMG/Public/Blueprint/UserWidget.h>

// Sets default values
ATakeShotPawn::ATakeShotPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATakeShotPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// Create GameInstance
	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}

	// Create Widget
	if (!TakeShotUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
		return;
	}
	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *TakeShotUserWidget);
	CreatedWidgetInstance->AddToViewport();
	TakeShotUserWidgetInstance = Cast<UTakeShotUserWidget>(CreatedWidgetInstance);

	// Send stage
	TSharedPtr<FJsonObject> JsonObjectStage = MakeShareable(new FJsonObject);
	JsonObjectStage->SetNumberField("Stage", ClientStage);
	GameInstance->SendJsonObject(JsonObjectStage);

	GameInstance->WebSocket->OnMessage().AddUObject(this, &ATakeShotPawn::OnWebSocketRecieveMessage);

	// Send notification to change to the take shot screen for players
	for (auto PlayerInfo : GameInstance->AllPlayerInfo) {
		if (PlayerInfo.Value.ScoreMultiplier != 1.0f) {
			PlayerAmountRecivedMult++;

			ReadyMap.Add(PlayerInfo.Key, false);

			TSharedPtr<FJsonObject> JsonObjectReady = MakeShareable(new FJsonObject);
			JsonObjectReady->SetStringField("playerName", PlayerInfo.Key);
			JsonObjectReady->SetStringField("ReadyScreen", "TakeShot");
			GameInstance->SendJsonObject(JsonObjectReady);
		}
	}

	TakeShotUserWidgetInstance->CreateReadyPlayers(ReadyMap);
}

void ATakeShotPawn::OnWebSocketRecieveMessage(const FString& MessageString) {
	// Create JSON object to be sent out
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MessageString);

	// Verify Variables
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("Deserialize Failed"));
		return;
	}

	FString playerName = GameInstance->GetJsonChildrenString(JsonObject, "clientInfo", "playerName");
	if (playerName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("invalid cliendId"));
		return;
	}

	// Check if bIsReady Status
	if (JsonObject->GetBoolField(TEXT("bIsReady")))
	{
		PlayerAmountRecivedMult--;

		TakeShotUserWidgetInstance->UpdateReadyPlayers(ReadyMap);

		// Everyone is ready
		if (PlayerAmountRecivedMult == 0) {
			if (NextLevel.IsNull()) {
				UE_LOG(LogTemp, Error, TEXT("Invalid NextLevel"));
				return;
			}
			UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), NextLevel);
			return;
		}
	}
	else {
		PlayerAmountRecivedMult++;
	}
}

// Called every frame
void ATakeShotPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATakeShotPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

