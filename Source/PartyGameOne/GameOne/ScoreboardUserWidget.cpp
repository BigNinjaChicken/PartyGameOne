// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreboardUserWidget.h"
#include <Engine/World.h>
#include <UMG/Public/Blueprint/UserWidget.h>
#include <Dom/JsonObject.h>
#include "WebSocketGameInstance.h"
#include <Kismet/GameplayStatics.h>

void UScoreboardUserWidget::OpenNextLevel()
{
	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(TutorialTimerHandle, [this]() {

		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField("Stage", "DrinkBonus");
		GameInstance->SendJsonObject(JsonObject);

		if (DrinkingBonusLevel.IsNull()) {
			UE_LOG(LogTemp, Error, TEXT("Invalid DrinkingBonusLevel"));
			return;
		}

		UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), DrinkingBonusLevel);

		}, ScoreboardTime, false);
}
