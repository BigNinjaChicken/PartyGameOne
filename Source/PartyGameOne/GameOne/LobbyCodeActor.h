// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyCodeActor.generated.h"

UCLASS()
class PARTYGAMEONE_API ALobbyCodeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALobbyCodeActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnWebSocketRecieveMessage(const FString& MessageString);

	class UWebSocketGameInstance* GameInstance;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Progress")
	void SetLobbyCode(const FText& LobbyText);
};
