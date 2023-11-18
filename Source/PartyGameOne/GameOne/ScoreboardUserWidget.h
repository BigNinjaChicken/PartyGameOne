#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreboardUserWidget.generated.h"


UCLASS()
class PARTYGAMEONE_API UScoreboardUserWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    // Use the USTRUCT in your UFUNCTION
    UFUNCTION(BlueprintImplementableEvent)
    void DisplayTopScoreboard(const TMap<FString, int32>& TopPlayerScores);
};
