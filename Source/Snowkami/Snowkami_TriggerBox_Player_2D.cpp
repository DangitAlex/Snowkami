// Fill out your copyright notice in the Description page of Project Settings.

#include "Snowkami_TriggerBox_Player_2D.h"


ASnowkami_TriggerBox_Player_2D::ASnowkami_TriggerBox_Player_2D() {}

void ASnowkami_TriggerBox_Player_2D::OnPlayerHitTrigger(ASnowkamiCharacter* player) 
{
	Super::OnPlayerHitTrigger(player);

	// Convert to 2D camera logic
	if (player->GetCurrentPlayerSpline() != playerSpline)
	{
		player->SetCurrentPlayerSpline(playerSpline);


	}
}

void ASnowkami_TriggerBox_Player_2D::TriggerEventDebug(AActor* OtherActor)
{
	if (Cast<ASnowkamiCharacter>(OtherActor)->GetCurrentPlayerSpline() == NULL)
		Debug(FColor::Purple, " :: Transitioning " + OtherActor->GetName() + " to 2D...", 1);
}


