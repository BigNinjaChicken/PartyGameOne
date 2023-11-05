// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyCodeActor.h"
#include "WebSocketGameInstance.h"
#include <Dom/JsonObject.h>

// Sets default values
ALobbyCodeActor::ALobbyCodeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALobbyCodeActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind the OnWebSocketConnected function to the WebSocketConnectedDelegate
	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		return;
	}

	GameInstance->WebSocket->OnMessage().AddUObject(this, &ALobbyCodeActor::OnWebSocketRecieveMessage);
}

void ALobbyCodeActor::OnWebSocketRecieveMessage(const FString& MessageString) {
	// Convert the FString to a JSON object
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MessageString);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("Deserialize Failed"))
			return;
	}

	// Display Session Code
	FString SessionCode;
	if (JsonObject->TryGetStringField(TEXT("sessionCode"), SessionCode))
	{
		SetLobbyCode(FText::FromString(SessionCode));
	}
}

// Called every frame
void ALobbyCodeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

