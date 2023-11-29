#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StartScreenUserWidget.h"
#include "JoinGamePawn.generated.h"

/*UENUM(BlueprintType)
enum class EGameState : uint8
{
    JoinGame,
    Tutorial
};*/

UCLASS()
class PARTYGAMEONE_API AJoinGamePawn : public APawn
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    AJoinGamePawn();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

private:
    // Function to handle the OnConnected event
    void OnWebSocketConnected();

    class UWebSocketGameInstance* GameInstance;

    bool bWebSocketConnectedHasRun = false;

    void OnWebSocketRecieveMessage(const FString& MessageString);

    

public:

    UFUNCTION(BlueprintImplementableEvent)
    void PlayerJoined(const FString& PlayerName);

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(EditAnywhere, Category = UI)
    TSubclassOf<class UStartScreenUserWidget> StartScreenUserWidget;
    UStartScreenUserWidget* WidgetInstance;
    TMap<FString, bool> PlayerReadyMap;

    UPROPERTY(EditAnywhere, Category = UI)
    int32 MinPlayerCount = 2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = State)
    TSoftObjectPtr<UWorld> TutorialLevel;

    TMap<int32, int32> PlayerSelectedDifficultyMap;

    UFUNCTION(BlueprintImplementableEvent)
    void DisplayChatMessage(const FString& PlayerName, const FString& ChatMessage);
};