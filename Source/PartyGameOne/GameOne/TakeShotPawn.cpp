// Fill out your copyright notice in the Description page of Project Settings.


#include "TakeShotPawn.h"
#include <Kismet/GameplayStatics.h>
#include "WebSocketGameInstance.h"
#include <Serialization/JsonReader.h>
#include <Dom/JsonObject.h>

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
	
	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObjectStage = MakeShareable(new FJsonObject);
	JsonObjectStage->SetStringField("Stage", "TakeShot");
	GameInstance->SendJsonObject(JsonObjectStage);

	GameInstance->WebSocket->OnMessage().AddUObject(this, &ATakeShotPawn::OnWebSocketRecieveMessage);
	for (auto PlayerInfo : GameInstance->AllPlayerInfo) {
		if (PlayerInfo.Value.ScoreMultiplier != 1.0f) {
			PlayerAmountRecivedMult++;

			TSharedPtr<FJsonObject> JsonObjectReady = MakeShareable(new FJsonObject);
			JsonObjectReady->SetStringField("clientId", PlayerInfo.Key);
			JsonObjectReady->SetStringField("ReadyScreen", "TakeShot");
			GameInstance->SendJsonObject(JsonObjectReady);
		}
	}
}

void ATakeShotPawn::OnWebSocketRecieveMessage(const FString& MessageString) {
	// Create JSON object to be sent out
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MessageString);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("Deserialize Failed"));
		return;
	}

	FString clientId = GameInstance->GetJsonChildrenString(JsonObject, "clientInfo", "clientId");
	if (clientId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("invalid cliendId"));
		return;
	}

	if (JsonObject->GetBoolField(TEXT("bIsReady")))
	{
		PlayerAmountRecivedMult--;

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

