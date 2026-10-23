// Fill out your copyright notice in the Description page of Project Settings.


#include "InstantPlayerController.h"

#include "Instant/Character/InstantCharacter.h"

void AInstantPlayerController::FlushPressedKeys()
{
	Super::FlushPressedKeys();

	if (AInstantCharacter* ICharacter = Cast<AInstantCharacter>(GetPawn()))
	{
		ICharacter->StopJumping();
		ICharacter->StopSprinting();
	}
}