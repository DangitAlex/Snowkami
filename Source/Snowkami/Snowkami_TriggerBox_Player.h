// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Snowkami_TriggerBox.h"
#include "SnowkamiCharacter.h"
#include "Snowkami_TriggerBox_Player.generated.h"

/**
 * 
 */
UCLASS()
class SNOWKAMI_API ASnowkami_TriggerBox_Player : public ASnowkami_TriggerBox
{
	GENERATED_BODY()
	
protected:
	UFUNCTION(Category = Overlap)
	void OnActorEnterTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UFUNCTION(Category = Overlap)
	virtual void OnPlayerHitTrigger(ASnowkamiCharacter* player);
	
public:
	ASnowkami_TriggerBox_Player();
};
