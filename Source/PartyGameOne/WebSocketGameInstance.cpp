// Fill out your copyright notice in the Description page of Project Settings.


#include "WebSocketGameInstance.h"
#include "WebSocketsModule.h"
#include "JsonUtilities.h"


void UWebSocketGameInstance::Init()
{
	Super::Init();

	if (!FModuleManager::Get().IsModuleLoaded("WebSockets")) {
		FModuleManager::Get().LoadModule("WebSockets");
	}

	WebSocket = FWebSocketsModule::Get().CreateWebSocket("ws://localhost:8080");
	// WebSocket = FWebSocketsModule::Get().CreateWebSocket("wss://partygame-399004.uk.r.appspot.com");

	WebSocket->OnConnected().AddLambda([]()
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Successfully Connected");
		});

	WebSocket->OnConnectionError().AddLambda([](const FString& Error)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Error);
		});

	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, bWasClean ? FColor::Green : FColor::Red, "Connection Closed: " + Reason);
		});

	WebSocket->OnMessage().AddLambda([](const FString& MessageString)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, "Received Message: " + MessageString);
		});

	WebSocket->OnMessageSent().AddLambda([](const FString& MessageString)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Sent Message: " + MessageString);
		});

	WebSocket->Connect();

	// Tell Server that Unreal is Connected
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

