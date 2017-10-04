// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Snowkami_TriggerBox_Player.h"
#include "Snowkami_TriggerBox_Player_3D.generated.h"

/**
 * 
 */
UCLASS()
class SNOWKAMI_API ASnowkami_TriggerBox_Player_3D : public ASnowkami_TriggerBox_Player
{
	GENERATED_BODY()
	
protected:
	UFUNCTION()
		void OnPlayerHitTrigger(ASnowkamiCharacter* player) override;

	UFUNCTION()
		void TriggerEventDebug(AActor* OtherActor) override;

public:
	ASnowkami_TriggerBox_Player_3D();
	
	
};
