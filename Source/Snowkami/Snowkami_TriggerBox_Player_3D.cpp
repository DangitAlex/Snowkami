// Fill out your copyright notice in the Description page of Project Settings.

#include "Snowkami_TriggerBox_Player_3D.h"

ASnowkami_TriggerBox_Player_3D::ASnowkami_TriggerBox_Player_3D() {}

void ASnowkami_TriggerBox_Player_3D::OnPlayerHitTrigger(ASnowkamiCharacter* player)
{
	Super::OnPlayerHitTrigger(player);

	// Convert to 3D camera logic
	if (player->GetCurrentPlayerSpline() != NULL)
	{
		player->SetCurrentPlayerSpline(NULL);
	}
}

void ASnowkami_TriggerBox_Player_3D::TriggerEventDebug(AActor* OtherActor)
{
	if(Cast<ASnowkamiCharacter>(OtherActor)->GetCurrentPlayerSpline() != NULL)
		Debug(FColor::Orange, " :: Transitioning " + OtherActor->GetName() + " to 3D...", 1);
}