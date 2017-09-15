// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SnowkamiGameMode.h"
#include "SnowkamiCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASnowkamiGameMode::ASnowkamiGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
