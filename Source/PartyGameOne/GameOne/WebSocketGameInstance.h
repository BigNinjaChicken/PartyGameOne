// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include <IWebSocket.h>
#include "TalkBoxPawn.h"
#include <HAL/IConsoleManager.h>
#include "WebSocketGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Score = 0;

	float ScoreMultiplier = 1.0f;
};

UCLASS()
class PARTYGAMEONE_API UWebSocketGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	void OnCallSkipCommand(const TArray<FString>& Args, UWorld* World);
	FString GetJsonChildrenString(TSharedPtr<FJsonObject> JsonObject, FString ParentVariableString, FString ChildVariableString);
	void SendJsonObject(TSharedPtr<FJsonObject>& JsonObject);
	TSharedPtr<IWebSocket> WebSocket;

	// Key: Player Id
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FString, FPlayerInfo> AllPlayerInfo;
	TArray<FGamePrompt> AllGamePrompts;
	struct IConsoleCommand* Command;
};
