// Fill out your copyright notice in the Description page of Project Settings.

#include "Snowkami_TriggerBox_Player.h"


ASnowkami_TriggerBox_Player::ASnowkami_TriggerBox_Player() {}

void ASnowkami_TriggerBox_Player::OnActorEnterTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnActorEnterTrigger(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (OtherActor->IsA(ASnowkamiCharacter::StaticClass()))
		OnPlayerHitTrigger(Cast<ASnowkamiCharacter>(OtherActor));
}

void ASnowkami_TriggerBox_Player::OnPlayerHitTrigger(ASnowkamiCharacter* player) {}