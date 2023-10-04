// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include <IWebSocket.h>
#include "WebSocketGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PARTYGAMEONE_API UWebSocketGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	FString GetJsonChildrenString(TSharedPtr<FJsonObject> JsonObject, FString ParentVariableString, FString ChildVariableString);
	void SendJsonObject(TSharedPtr<FJsonObject>& JsonObject);
	TSharedPtr<IWebSocket> WebSocket;

	TArray<FString> AllPlayerIds;
};
