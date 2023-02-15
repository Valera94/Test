// Copyright Epic Games, Inc. All Rights Reserved.

#include "ForJobGameMode.h"
#include "ForJobCharacter.h"
#include "UObject/ConstructorHelpers.h"

AForJobGameMode::AForJobGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
