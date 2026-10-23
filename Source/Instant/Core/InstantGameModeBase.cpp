// Fill out your copyright notice in the Description page of Project Settings.


#include "InstantGameModeBase.h"

#include "Instant/Character/InstantCharacter.h"
#include "InstantPlayerController.h"

AInstantGameModeBase::AInstantGameModeBase()
{
	DefaultPawnClass = AInstantCharacter::StaticClass();
	PlayerControllerClass = AInstantPlayerController::StaticClass();
}