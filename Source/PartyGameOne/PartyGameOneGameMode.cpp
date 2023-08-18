// Copyright Epic Games, Inc. All Rights Reserved.

#include "PartyGameOneGameMode.h"
#include "PartyGameOneCharacter.h"
#include "UObject/ConstructorHelpers.h"

APartyGameOneGameMode::APartyGameOneGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
