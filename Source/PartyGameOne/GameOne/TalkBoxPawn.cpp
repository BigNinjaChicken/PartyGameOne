// Fill out your copyright notice in the Description page of Project Settings.


#include "TalkBoxPawn.h"
#include "TimerUserWidget.h"
#include "WebSocketGameInstance.h"
#include "ShowResponsesUserWidget.h"
#include <Kismet/GameplayStatics.h>
#include "ShowAllGoupResponsesUserWidget.h"
#include <cmath>

// Sets default values
ATalkBoxPawn::ATalkBoxPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATalkBoxPawn::BeginPlay()
{
	Super::BeginPlay();

	// Create GameInstance
	GameInstance = Cast<UWebSocketGameInstance>(GetGameInstance());
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance"));
		return;
	}
	GameInstance->AllPlayerInfo.GetKeys(AllPlayerIds);

	if (ActNumber == 1) {
		// CreateSentencePossibility("", "", "", "");
		CreateSentencePossibility("Old man Roger caught us trespassing and he", "but then I", "Once he caught my friend,", "But you should\'ve seen his face when");
		CreateSentencePossibility("The stoner absolutely lost it when", "Once his weed was back though,", "Since one of the officers made it to him,", "Once he was in custody, he");
		CreateSentencePossibility("My fellow Americans,", "Despite this news,", "The White House has just received news that", "And because of this revelation,");
		CreateSentencePossibility("The woke left doesn\'t want you to know this, but", "And because of that, society", "But the rest of us in the real world know", "Especially once we take over the");
		CreateSentencePossibility("The dragon swoops down, killing", "But as the knights come to defend", "The dragon was scared away by", "After that, he never came back to");
		CreateSentencePossibility("Reagan shouted out to the nation", "But Gorbachev said that", "He really didn\'t care when", "And since then, Reagan has");
		CreateSentencePossibility("Ya reckon that New Brunswickshire will", "I couldn\'t count on Nem to do", "What in the hell was", "Especially because he didn\'t");
		CreateSentencePossibility("Thor ripped off his shirt when he saw", "then Odin said, \"Son,\"", "Tony Stark rose from the dead to say", "In revolt, Captain America retorted, stating that");
		CreateSentencePossibility("Tommy, you can\'t just", "But Arthur, you haven\'t considered", "Mr. Kimber told me that", "Yeah, but I bet he didn\'t think of");
		CreateSentencePossibility("Master Jedi, have you the gall to", "With your unique control of the force,", "But since you all have such an ego,", "I would kill you where you stand, but");
		CreateSentencePossibility("Hey! You can\'t do that! That\'s my", "Tough shit, nerd. You can", "Once we get to lunch, I\'m gonna", "Thankfully, the carrot didn\'t");
		CreateSentencePossibility("Yes Ma\'am, we sell quite a few", "In fact, our pricing is some of the best! We have", "Once you buy our plan, you get", "But only if you are willing to");
		CreateSentencePossibility("Have you heard Green Day\'s new album? They said", "And by the way, they really didn\'t", "But if you make sure to listen to", "Then you\'ll be entirely set up to check out the");
		CreateSentencePossibility("How many miles are we gonna walk? I don\'t want to", "Alongside the bears, too. Can\'t we just", "Oh my god! I didn\'t think we\'d see", "We have to run. It\'s going to");
		CreateSentencePossibility("First thing\'s first, you want to make sure that", "But in order to make sure that your material goes through,", "Once we have the raw iron, we feed it into", "And then shit gets CRAZY. We turn it into");
	}

	GameInstance->WebSocket->OnMessage().AddUObject(this, &ATalkBoxPawn::OnWebSocketRecieveMessage);

	// Create TimerUserWidget
	if (!TimerUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
		return;
	}
	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *TimerUserWidget);
	CreatedWidgetInstance->AddToViewport();
	TimerWidgetInstance = Cast<UTimerUserWidget>(CreatedWidgetInstance);
	TimerWidgetInstance->StartTimer(InputPromptTime - InputPromptSafetyTime);

	GetWorld()->GetTimerManager().SetTimer(GameTimerHandle, this, &ATalkBoxPawn::EndRound, InputPromptTime, false);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("Stage", PromptStageNumber);
	GameInstance->SendJsonObject(JsonObject);

	SendPlayersSentenceFragments();

	for (auto Player : GameInstance->AllPlayerInfo) {
		TSharedPtr<FJsonObject> JsonObjectTime = MakeShareable(new FJsonObject);
		float TimeDecreaseBasedOnScore = log(Player.Value.ScoreMultiplier - 0.4f) + 1.0f;
		JsonObjectTime->SetStringField("Timer", FString::SanitizeFloat(InputPromptTime / TimeDecreaseBasedOnScore));
		JsonObjectTime->SetStringField("playerName", Player.Key);
		GameInstance->SendJsonObject(JsonObjectTime);
	}
}

// Called every frame
void ATalkBoxPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATalkBoxPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ATalkBoxPawn::CreateSentencePossibility(FString FragmentOne, FString FragmentTwo, FString FragmentThree, FString FragmentFour)
{
	FEncapsule Item;
	Item.SentenceFragmentOne = FragmentOne.ToUpper();
	Item.SentenceFragmentTwo = FragmentTwo.ToUpper();
	Item.SentenceFragmentThree = FragmentThree.ToUpper();
	Item.SentenceFragmentFour = FragmentFour.ToUpper();
	Item.FragFiveSixGroupPoints = 0;
	Item.FragOneTwoGroupPoints = 0;
	Item.FragThreeFourGroupPoints = 0;
	SentencePossibilities.Add(Item);
}

void ATalkBoxPawn::SendPlayersSentenceFragments() {
	// Check if the GameInstance is valid
	if (!GameInstance) {
		UE_LOG(LogTemp, Error, TEXT("GameInstance is not valid."));
		return;
	}

	FRandomStream RandomStream;
	ShuffleArray(RandomStream, AllPlayerIds);

	int32 NumPlayers = AllPlayerIds.Num();
	
	if (ActNumber == 1) {
		// Check if there are enough SentencePossibilities
		if (SentencePossibilities.Num() < NumPlayers) {
			UE_LOG(LogTemp, Warning, TEXT("Not enough SentencePossibilities for all players."));
			return;
		}

		// Generate game prompts for each player
		for (int32 i = 0; i < NumPlayers; i++) {
			int index = FMath::RandRange(0, SentencePossibilities.Num() - 1);
			FEncapsule SentenceElement = SentencePossibilities[index];

			FGamePrompt Item;
			Item.FragmentOnePlayerId = AllPlayerIds[i];
			Item.FragmentTwoPlayerId = AllPlayerIds[(i + 1) % NumPlayers];
			Item.SentenceFragments = SentenceElement;

			AllGamePrompts.Add(Item);
			SentencePossibilities.RemoveAt(index);
		}
	}

	if (ActNumber == 2) {
		AllGamePrompts = GameInstance->AllGamePrompts;

		// Generate game prompts for each player
		for (int32 i = 0; i < NumPlayers; i++) {
			AllGamePrompts[i].FragmentThreePlayerId = AllPlayerIds[i];
			AllGamePrompts[i].FragmentFourPlayerId = AllPlayerIds[(i + 1) % NumPlayers];
		}

	}

	if (ActNumber == 3) {
		AllGamePrompts = GameInstance->AllGamePrompts;

		// Generate game prompts for each player
		for (int32 i = 0; i < NumPlayers; i++) {
			AllGamePrompts[i].FragmentFivePlayerId = AllPlayerIds[i];
			AllGamePrompts[i].FragmentSixPlayerId = AllPlayerIds[(i + 1) % NumPlayers];
		}

		for (FGamePrompt& GamePrompt : AllGamePrompts) {
			GamePrompt.SentenceFragments.SentenceFragmentFive.SetNum(2);
			GamePrompt.SentenceFragments.SentenceFragmentFive[0] = GamePrompt.SentenceFragments.SentenceFragmentOne;
			GamePrompt.SentenceFragments.SentenceFragmentFive[1] = GamePrompt.SentenceFragments.SentenceFragmentOneResponce;
			GamePrompt.SentenceFragments.SentenceFragmentSix.SetNum(2);
			GamePrompt.SentenceFragments.SentenceFragmentSix[0] = GamePrompt.SentenceFragments.SentenceFragmentTwo;
			GamePrompt.SentenceFragments.SentenceFragmentSix[1] = GamePrompt.SentenceFragments.SentenceFragmentTwoResponce;
		}
	}

	// Send Game Prompts
	for (int32 i = 0; i < NumPlayers; i++) {
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		if (ActNumber == 1) {
			JsonObject->SetStringField("promptOneFragmentOne", AllGamePrompts[i].SentenceFragments.SentenceFragmentOne);
			JsonObject->SetStringField("promptOneFragmentOnePlayerId", AllGamePrompts[i].FragmentOnePlayerId);
			JsonObject->SetStringField("promptOneFragmentTwo", AllGamePrompts[i].SentenceFragments.SentenceFragmentTwo);
			JsonObject->SetStringField("promptOneFragmentTwoPlayerId", AllGamePrompts[i].FragmentTwoPlayerId);

			JsonObject->SetStringField("promptTwoFragmentOne", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentOne);
			JsonObject->SetStringField("promptTwoFragmentOnePlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentOnePlayerId);
			JsonObject->SetStringField("promptTwoFragmentTwo", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentTwo);
			JsonObject->SetStringField("promptTwoFragmentTwoPlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentTwoPlayerId);

			JsonObject->SetStringField("playerName", AllGamePrompts[i].FragmentTwoPlayerId); 
			GameInstance->SendJsonObject(JsonObject);
		}

		if (ActNumber == 2) {
			JsonObject->SetStringField("promptOneFragmentOne", AllGamePrompts[i].SentenceFragments.SentenceFragmentThree);
			JsonObject->SetStringField("promptOneFragmentOnePlayerId", AllGamePrompts[i].FragmentThreePlayerId);
			JsonObject->SetStringField("promptOneFragmentTwo", AllGamePrompts[i].SentenceFragments.SentenceFragmentFour);
			JsonObject->SetStringField("promptOneFragmentTwoPlayerId", AllGamePrompts[i].FragmentFourPlayerId);

			JsonObject->SetStringField("promptTwoFragmentOne", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentThree);
			JsonObject->SetStringField("promptTwoFragmentOnePlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentThreePlayerId);
			JsonObject->SetStringField("promptTwoFragmentTwo", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentFour);
			JsonObject->SetStringField("promptTwoFragmentTwoPlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentFourPlayerId);

			JsonObject->SetStringField("partner", AllGamePrompts[i].FragmentThreePlayerId);
			JsonObject->SetStringField("playerName", AllGamePrompts[i].FragmentFourPlayerId);
			GameInstance->SendJsonObject(JsonObject);
		}

		if (ActNumber == 3) {
			JsonObject->SetStringField("promptOneFragmentOne1", AllGamePrompts[i].SentenceFragments.SentenceFragmentFive[0]);
			JsonObject->SetStringField("promptOneFragmentOne2", AllGamePrompts[i].SentenceFragments.SentenceFragmentFive[1]);
			JsonObject->SetStringField("promptOneFragmentOnePlayerId", AllGamePrompts[i].FragmentFivePlayerId);
			JsonObject->SetStringField("promptOneFragmentTwo1", AllGamePrompts[i].SentenceFragments.SentenceFragmentSix[0]);
			JsonObject->SetStringField("promptOneFragmentTwo2", AllGamePrompts[i].SentenceFragments.SentenceFragmentSix[1]);
			JsonObject->SetStringField("promptOneFragmentTwoPlayerId", AllGamePrompts[i].FragmentSixPlayerId);

			JsonObject->SetStringField("promptTwoFragmentOne1", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentFive[0]);
			JsonObject->SetStringField("promptTwoFragmentOne2", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentFive[1]);
			JsonObject->SetStringField("promptTwoFragmentOnePlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentFivePlayerId);
			JsonObject->SetStringField("promptTwoFragmentTwo1", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentSix[0]);
			JsonObject->SetStringField("promptTwoFragmentTwo2", AllGamePrompts[(i + 1) % NumPlayers].SentenceFragments.SentenceFragmentSix[1]);
			JsonObject->SetStringField("promptTwoFragmentTwoPlayerId", AllGamePrompts[(i + 1) % NumPlayers].FragmentSixPlayerId);

			JsonObject->SetStringField("partner", AllGamePrompts[i].FragmentFivePlayerId);
			JsonObject->SetStringField("playerName", AllGamePrompts[i].FragmentSixPlayerId);
			GameInstance->SendJsonObject(JsonObject);
		}

	}
}

void ATalkBoxPawn::EndRound() {
	if (bRoundEnded) {
		UE_LOG(LogTemp, Warning, TEXT("Extra EndRound() Call"));
		return;
	}
	bRoundEnded = true;

	GetWorld()->GetTimerManager().ClearTimer(GameTimerHandle);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("Stage", PollStageNumber);
	GameInstance->SendJsonObject(JsonObject);

	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *ShowResponcesUserWidget);
	TimerWidgetInstance->RemoveFromParent();
	CreatedWidgetInstance->AddToViewport();
	ShowResponcesWidgetInstance = Cast<UShowResponsesUserWidget>(CreatedWidgetInstance);
	ShowResponcesWidgetInstance->OnWinnerDisplayed.AddDynamic(this, &ATalkBoxPawn::OnWinnerDisplayed);
	ShowResponcesWidgetInstance->ShowPrompts(AllGamePrompts, ActNumber);
	SendPlayerPole();
}

void ATalkBoxPawn::OnWebSocketRecieveMessage(const FString& MessageString) {
	// Create JSON object to be sent out
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MessageString);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("Deserialize Failed"));
		return;
	}

	FString playerName = GameInstance->GetJsonChildrenString(JsonObject, "clientInfo", "playerName");
	if (playerName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("invalid cliendId"));
		return;
	}

	PromptResponceUserInputPromptOne(JsonObject, playerName);

	PromptResponceUserInputPromptTwo(JsonObject, playerName);

	RecievedPlayerPoleVote(JsonObject);

	ReceivePlayerAllPoleVote(JsonObject);
}

void ATalkBoxPawn::UpdateScoreOnDevice(FString playerName) {
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField("Score", GameInstance->AllPlayerInfo[playerName].Score);
	JsonObject->SetStringField("playerName", playerName);
	GameInstance->SendJsonObject(JsonObject);
}

void ATalkBoxPawn::ReceivePlayerAllPoleVote(TSharedPtr<FJsonObject> JsonObject)
{
	uint64 option;
	if (JsonObject->TryGetNumberField(TEXT("option"), option))
	{
		if (ActNumber == 1) {
			AllGamePrompts[option].SentenceFragments.FragOneTwoGroupPoints += 100;
			GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentOnePlayerId].Score += 300;
			UpdateScoreOnDevice(AllGamePrompts[option].FragmentOnePlayerId);
			GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentTwoPlayerId].Score += 300;
			UpdateScoreOnDevice(AllGamePrompts[option].FragmentTwoPlayerId);
		}

		if (ActNumber == 2) {
			AllGamePrompts[option].SentenceFragments.FragThreeFourGroupPoints += 100;
			GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentThreePlayerId].Score += 300 * GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentThreePlayerId].ScoreMultiplier;
			UpdateScoreOnDevice(AllGamePrompts[option].FragmentThreePlayerId);
			GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentFourPlayerId].Score += 300 * GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentFourPlayerId].ScoreMultiplier;
			UpdateScoreOnDevice(AllGamePrompts[option].FragmentFourPlayerId);
		}

		if (ActNumber == 3) {
			AllGamePrompts[option].SentenceFragments.FragFiveSixGroupPoints += 150;
			GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentFivePlayerId].Score += 300 * GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentFivePlayerId].ScoreMultiplier;
			GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentSixPlayerId].Score += 300 * GameInstance->AllPlayerInfo[AllGamePrompts[option].FragmentSixPlayerId].ScoreMultiplier;
			UpdateScoreOnDevice(AllGamePrompts[option].FragmentFivePlayerId);
			UpdateScoreOnDevice(AllGamePrompts[option].FragmentSixPlayerId);
		}

		TotalOptionsInputed++;

		if (TotalOptionsInputed == GameInstance->AllPlayerInfo.Num()) {
			if (ScoreboardLevel.IsNull()) {
				UE_LOG(LogTemp, Error, TEXT("Invalid ScoreboardLevel"));
				return;
			}
			GameInstance->AllGamePrompts = AllGamePrompts;
			UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), ScoreboardLevel);
			return;
		}
	}
}

void ATalkBoxPawn::RecievedPlayerPoleVote(TSharedPtr<FJsonObject> JsonObject)
{
	FString poleSelection;
	if (JsonObject->TryGetStringField(TEXT("poleSelection"), poleSelection))
	{
		if (ActNumber == 1) {
			if (poleSelection == "Option1") {
				CurrentPoleVoteTotals.Option1Votes++;
				GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentOnePlayerId].Score += 200;
				UpdateScoreOnDevice(AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentOnePlayerId);
			}
			else if (poleSelection == "Option2") {
				CurrentPoleVoteTotals.Option2Votes++;
				GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentTwoPlayerId].Score += 200;
				UpdateScoreOnDevice(AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentTwoPlayerId);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Invalid Option"))
			}
		}
		
		if (ActNumber == 2) {
			if (poleSelection == "Option1") {
				CurrentPoleVoteTotals.Option1Votes++;
				GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentThreePlayerId].Score += 200 * GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentThreePlayerId].ScoreMultiplier;
				UpdateScoreOnDevice(AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentThreePlayerId);
			}
			else if (poleSelection == "Option2") {
				CurrentPoleVoteTotals.Option2Votes++;
				GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFourPlayerId].Score += 200 * GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFourPlayerId].ScoreMultiplier;
				UpdateScoreOnDevice(AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFourPlayerId);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Invalid Option"))
			}
		}

		if (ActNumber == 3) {
			if (poleSelection == "Option1") {
				CurrentPoleVoteTotals.Option1Votes++;
				GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFivePlayerId].Score += 200 * GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFivePlayerId].ScoreMultiplier;
				UpdateScoreOnDevice(AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFivePlayerId);
			}
			else if (poleSelection == "Option2") {
				CurrentPoleVoteTotals.Option2Votes++;
				GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentSixPlayerId].Score += 200 * GameInstance->AllPlayerInfo[AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentSixPlayerId].ScoreMultiplier;
				UpdateScoreOnDevice(AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentSixPlayerId);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Invalid Option"))
			}

		}

		if (CurrentPoleVoteTotals.TotalVotes() == AllPlayerIds.Num()) {
			// All players have submitted a vote

			// Determine the winner of the current round and display it
			FString winnerPlayerId;
			if (ActNumber == 1) {
				if (CurrentPoleVoteTotals.Option1Votes > CurrentPoleVoteTotals.Option2Votes) {
					winnerPlayerId = AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentOnePlayerId;
				}
				else {
					winnerPlayerId = AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentTwoPlayerId;
				}
			}
			
			if (ActNumber == 2) {
				if (CurrentPoleVoteTotals.Option1Votes > CurrentPoleVoteTotals.Option2Votes) {
					winnerPlayerId = AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentThreePlayerId;
				}
				else {
					winnerPlayerId = AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFourPlayerId;
				}
			}

			if (ActNumber == 3) {
				if (CurrentPoleVoteTotals.Option1Votes > CurrentPoleVoteTotals.Option2Votes) {
					winnerPlayerId = AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentFivePlayerId;
				}
				else {
					winnerPlayerId = AllGamePrompts[ShowResponcesWidgetInstance->index].FragmentSixPlayerId;
				}
			}

			// Display the winner on the ShowResponcesWidgetInstance
			ShowResponcesWidgetInstance->DisplayWinner(winnerPlayerId, CurrentPoleVoteTotals.Option1Votes > CurrentPoleVoteTotals.Option2Votes);
		}
	}
}

void ATalkBoxPawn::OnWinnerDisplayed() {
	UE_LOG(LogTemp, Warning, TEXT("OnWinnerDisplayed"));
	if (ShowResponcesWidgetInstance->index < AllGamePrompts.Num() - 1) {
		// Move to the next prompt
		ShowResponcesWidgetInstance->index++;
		ShowResponcesWidgetInstance->ShowPrompts(AllGamePrompts, ActNumber); // Display next prompt
		SendPlayerPole();

		return;
	}

	if (!ShowAllGoupResponsesUserWidget) {
		UE_LOG(LogTemp, Error, TEXT("ShowAllGoupResponsesUserWidget null"));
		return;
	}

	// Display All Responses Pole
	UUserWidget* CreatedWidgetInstance = CreateWidget(GetWorld(), *ShowAllGoupResponsesUserWidget);
	ShowResponcesWidgetInstance->RemoveFromParent();
	CreatedWidgetInstance->AddToViewport();
	ShowAllGoupResponsesWidgetInstance = Cast<UShowAllGoupResponsesUserWidget>(CreatedWidgetInstance);
	ShowAllGoupResponsesWidgetInstance->ShowPrompts(AllGamePrompts);
	SendPlayerPole();

	TSharedPtr<FJsonObject> JsonObjectBeingSent = MakeShareable(new FJsonObject);
	JsonObjectBeingSent->SetNumberField("Stage", AllPoleStageNumber);
	GameInstance->SendJsonObject(JsonObjectBeingSent);

	TSharedPtr<FJsonObject> JsonObjectAllFragments = MakeShareable(new FJsonObject);
	for (int32 i = 0; i < AllGamePrompts.Num(); ++i) {
		FGamePrompt GamePrompts = AllGamePrompts[i];

		FString PromptFragmentOneKey = FString::Printf(TEXT("promptFragmentOne%d"), i);
		FString PromptFragmentOneResponceKey = FString::Printf(TEXT("promptFragmentOneResponce%d"), i);
		FString PromptFragmentOnePlayerIdKey = FString::Printf(TEXT("promptFragmentOnePlayerId%d"), i);
		FString PromptFragmentTwoKey = FString::Printf(TEXT("promptFragmentTwo%d"), i);
		FString PromptFragmentTwoResponceKey = FString::Printf(TEXT("promptFragmentTwoResponce%d"), i);
		FString PromptFragmentTwoPlayerIdKey = FString::Printf(TEXT("promptFragmentTwoPlayerId%d"), i);

		if (ActNumber == 1) {

			FEncapsule SentenceFragments = GamePrompts.SentenceFragments;
			JsonObjectAllFragments->SetStringField(PromptFragmentOneKey, SentenceFragments.SentenceFragmentOne);
			JsonObjectAllFragments->SetStringField(PromptFragmentOneResponceKey, SentenceFragments.SentenceFragmentOneResponce);
			JsonObjectAllFragments->SetStringField(PromptFragmentOnePlayerIdKey, GamePrompts.FragmentOnePlayerId);
			JsonObjectAllFragments->SetStringField(PromptFragmentTwoKey, SentenceFragments.SentenceFragmentTwo);
			JsonObjectAllFragments->SetStringField(PromptFragmentTwoResponceKey, SentenceFragments.SentenceFragmentTwoResponce);
			JsonObjectAllFragments->SetStringField(PromptFragmentTwoPlayerIdKey, GamePrompts.FragmentTwoPlayerId);
		}

		if (ActNumber == 2) {

			FEncapsule SentenceFragments = GamePrompts.SentenceFragments;
			JsonObjectAllFragments->SetStringField(PromptFragmentOneKey, SentenceFragments.SentenceFragmentThree);
			JsonObjectAllFragments->SetStringField(PromptFragmentOneResponceKey, SentenceFragments.SentenceFragmentThreeResponce);
			JsonObjectAllFragments->SetStringField(PromptFragmentOnePlayerIdKey, GamePrompts.FragmentThreePlayerId);
			JsonObjectAllFragments->SetStringField(PromptFragmentTwoKey, SentenceFragments.SentenceFragmentFour);
			JsonObjectAllFragments->SetStringField(PromptFragmentTwoResponceKey, SentenceFragments.SentenceFragmentFourResponce);
			JsonObjectAllFragments->SetStringField(PromptFragmentTwoPlayerIdKey, GamePrompts.FragmentFourPlayerId);
		}

		if (ActNumber == 3) {


			FEncapsule SentenceFragments = GamePrompts.SentenceFragments;
			JsonObjectAllFragments->SetStringField(PromptFragmentOneKey, SentenceFragments.SentenceFragmentFive[0] + " " + SentenceFragments.SentenceFragmentFive[1]);
			JsonObjectAllFragments->SetStringField(PromptFragmentOneResponceKey, SentenceFragments.SentenceFragmentFiveResponce);
			JsonObjectAllFragments->SetStringField(PromptFragmentOnePlayerIdKey, GamePrompts.FragmentFivePlayerId);
			JsonObjectAllFragments->SetStringField(PromptFragmentTwoKey, SentenceFragments.SentenceFragmentSix[0] + " " + SentenceFragments.SentenceFragmentSix[1]);
			JsonObjectAllFragments->SetStringField(PromptFragmentTwoResponceKey, SentenceFragments.SentenceFragmentSixResponce);
			JsonObjectAllFragments->SetStringField(PromptFragmentTwoPlayerIdKey, GamePrompts.FragmentSixPlayerId);
		}
	}

	GameInstance->SendJsonObject(JsonObjectAllFragments);
}

void ATalkBoxPawn::PromptResponceUserInputPromptOne(TSharedPtr<FJsonObject> JsonObject, FString playerName)
{
	FString userInputPromptOne;
	if (!JsonObject->TryGetStringField(TEXT("userInputPromptOne"), userInputPromptOne))
	{
		return;
	}

	FString promptOneFragmentOne;
	JsonObject->TryGetStringField(TEXT("promptOneFragmentOne"), promptOneFragmentOne);

	FString promptTwoFragmentOne;
	JsonObject->TryGetStringField(TEXT("promptTwoFragmentOne"), promptTwoFragmentOne);

	for (FGamePrompt& GamePrompt : AllGamePrompts) {
		if (ActNumber == 1) {
			if (promptOneFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentOne ||
				promptTwoFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentOne) {
				if (GamePrompt.SentenceFragments.SentenceFragmentOneResponce.IsEmpty()) {
					UE_LOG(LogTemp, Warning, TEXT("userInputPromptOne Submitted"));
					GamePrompt.SentenceFragments.SentenceFragmentOneResponce = userInputPromptOne;
					break;
				}
			}
		}

		if (ActNumber == 2) {
			if (promptOneFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentThree ||
				promptTwoFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentThree) {
				if (GamePrompt.SentenceFragments.SentenceFragmentThreeResponce.IsEmpty()) {
					UE_LOG(LogTemp, Warning, TEXT("userInputPromptOne Submitted"));
					GamePrompt.SentenceFragments.SentenceFragmentThreeResponce = userInputPromptOne;
					break;
				}
			}
		}

		if (ActNumber == 3) {
			if (promptOneFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentFive[0] ||
				promptTwoFragmentOne == GamePrompt.SentenceFragments.SentenceFragmentFive[0]) {
				if (GamePrompt.SentenceFragments.SentenceFragmentFiveResponce.IsEmpty()) {
					UE_LOG(LogTemp, Warning, TEXT("userInputPromptOne Submitted"));
					GamePrompt.SentenceFragments.SentenceFragmentFiveResponce = userInputPromptOne;
					break;
				}
			}
		}
	}

	PromptReadyUp(playerName);
}

void ATalkBoxPawn::PromptResponceUserInputPromptTwo(TSharedPtr<FJsonObject> JsonObject, FString playerName)
{
	FString userInputPromptTwo;
	if (!JsonObject->TryGetStringField(TEXT("userInputPromptTwo"), userInputPromptTwo))
	{
		return;
	}

	FString promptOneFragmentTwo;
	JsonObject->TryGetStringField(TEXT("promptOneFragmentTwo"), promptOneFragmentTwo);

	FString promptTwoFragmentTwo;
	JsonObject->TryGetStringField(TEXT("promptTwoFragmentTwo"), promptTwoFragmentTwo);

	for (FGamePrompt& GamePrompt : AllGamePrompts) {
		

		if (ActNumber == 1) {
			if (promptOneFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentTwo ||
				promptTwoFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentTwo) {
				if (GamePrompt.SentenceFragments.SentenceFragmentTwoResponce.IsEmpty()) {
					UE_LOG(LogTemp, Warning, TEXT("userInputPromptTwo Submitted"));
					GamePrompt.SentenceFragments.SentenceFragmentTwoResponce = userInputPromptTwo;
					break;
				}
			}
		}

		if (ActNumber == 2) {
			if (promptOneFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentFour ||
				promptTwoFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentFour) {
				if (GamePrompt.SentenceFragments.SentenceFragmentFourResponce.IsEmpty()) {
					UE_LOG(LogTemp, Warning, TEXT("userInputPromptTwo Submitted"));
					GamePrompt.SentenceFragments.SentenceFragmentFourResponce = userInputPromptTwo;
					break;
				}
			}
		}

		if (ActNumber == 3) {
			if (promptOneFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentSix[0] ||
				promptTwoFragmentTwo == GamePrompt.SentenceFragments.SentenceFragmentSix[0]) {
				if (GamePrompt.SentenceFragments.SentenceFragmentSixResponce.IsEmpty()) {
					UE_LOG(LogTemp, Warning, TEXT("userInputPromptTwo Submitted"));
					GamePrompt.SentenceFragments.SentenceFragmentSixResponce = userInputPromptTwo;
					break;
				}
			}
		}
	}

	PromptReadyUp(playerName);
}

void ATalkBoxPawn::PromptReadyUp(FString playerName)
{
	ReadyPlayerCount++;

	// ReadyPlayerCount should be twice the number of players due to double submissions
	if (ReadyPlayerCount == GameInstance->AllPlayerInfo.Num() * 2)
	{
		if (!ShowResponcesUserWidget) {
			UE_LOG(LogTemp, Error, TEXT("Failed to create the widget instance."));
			return;
		}

		EndRound();
	}
}

void ATalkBoxPawn::SendPlayerPole()
{
	if (AllGamePrompts.Num() == ShowResponcesWidgetInstance->index) return;

	const FGamePrompt& GamePrompt = AllGamePrompts[ShowResponcesWidgetInstance->index];
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	FEncapsule SentenceFragments = GamePrompt.SentenceFragments;
	if (ActNumber == 1) {
		JsonObject->SetStringField("Option1", SentenceFragments.SentenceFragmentOneResponce);
		JsonObject->SetStringField("Option2", SentenceFragments.SentenceFragmentTwoResponce);
	}
	if (ActNumber == 2) {
		JsonObject->SetStringField("Option1", SentenceFragments.SentenceFragmentThreeResponce);
		JsonObject->SetStringField("Option2", SentenceFragments.SentenceFragmentFourResponce);
	}
	if (ActNumber == 3) {
		JsonObject->SetStringField("Option1", SentenceFragments.SentenceFragmentFiveResponce);
		JsonObject->SetStringField("Option2", SentenceFragments.SentenceFragmentSixResponce);
	}
	GameInstance->SendJsonObject(JsonObject);

	CurrentPoleVoteTotals.Reset();
}

template<typename Type>
void ATalkBoxPawn::ShuffleArray(FRandomStream& Stream, TArray<Type>& Array)
{
	// This is the correct shuffle method but resulted in people getting the same teammate
	const int32 LastIndex = Array.Num() - 1;

	for (int32 i = 0; i <= LastIndex; i += 1) {
		const int32 Index = Stream.RandRange(i, LastIndex);
		if (i == Index) {
			continue;
		}

		Array.Swap(i, Index);
	}
}