// Fill out your copyright notice in the Description page of Project Settings.


#include "WebSocketGameInstance.h"
#include "WebSocketsModule.h"
#include "JsonUtilities.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
 
#include "GameFramework/GameUserSettings.h" 
#include <Dom/JsonObject.h>
#include <Serialization/JsonReader.h>


void UWebSocketGameInstance::Init()
{
	Super::Init();

	Command = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Skip"),
		TEXT("Skip Current Scene"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateUObject(this, &UWebSocketGameInstance::OnCallSkipCommand),
		ECVF_Cheat
	);

	if (!FModuleManager::Get().IsModuleLoaded("WebSockets")) {
		FModuleManager::Get().LoadModule("WebSockets");
	}

	// WebSocket = FWebSocketsModule::Get().CreateWebSocket("ws://localhost:8080");
	WebSocket = FWebSocketsModule::Get().CreateWebSocket("wss://party-game-web-service.onrender.com");

	WebSocket->OnConnected().AddLambda([]()
		{
			UE_LOG(LogTemp, Warning, TEXT("Successfully Connected"));
		});

	WebSocket->OnConnectionError().AddLambda([](const FString& Error)
		{
			UE_LOG(LogTemp, Error, TEXT("Connection Error: %s"), *Error);
		});

	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			FColor MessageColor = bWasClean ? FColor::Green : FColor::Red;
			UE_LOG(LogTemp, Warning, TEXT("Connection Closed: %s"), *Reason);
		});

	WebSocket->OnMessage().AddLambda([this](const FString& MessageString)
		{
			UE_LOG(LogTemp, Warning, TEXT("Received Message: %s"), *MessageString);
		});

	WebSocket->OnMessageSent().AddLambda([](const FString& MessageString)
		{
			UE_LOG(LogTemp, Warning, TEXT("Sent Message: %s"), *MessageString);
		});

	WebSocket->Connect();

	// Tell Server that Unreal is Connected.
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("clientType"), TEXT("Unreal"));
	SendJsonObject(JsonObject);
}

void UWebSocketGameInstance::Shutdown()
{
	if (WebSocket->IsConnected()) {
		WebSocket->Close();
	}
	Super::Shutdown();
}

void UWebSocketGameInstance::OnCallSkipCommand(const TArray<FString>& Args, UWorld* World)
{
}

FString UWebSocketGameInstance::GetJsonChildrenString(TSharedPtr<FJsonObject> JsonObject, FString ParentVariableString, FString ChildVariableString)
{
	if (!JsonObject.IsValid()) // Check if the input JsonObject is valid
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Websocket JSON Object"))
		return FString();
	}

	const TSharedPtr<FJsonObject>* OutObject = nullptr;

	if (JsonObject->TryGetObjectField(ParentVariableString, OutObject) && OutObject->IsValid()) // Check if the parent field exists and is a valid object
	{
		FString ChildString;
		// Assuming you want to access the content of the shared pointer.
		if ((*OutObject)->TryGetStringField(ChildVariableString, ChildString))
		{
			return ChildString;
		}
	}

	return FString();
}

void UWebSocketGameInstance::SendJsonObject(TSharedPtr<FJsonObject>& JsonObject)
{
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	WebSocket->Send(OutputString);
}
