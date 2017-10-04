// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/BoxComponent.h"
#include "Snowkami_TriggerBox.generated.h"

/**
 * 
 */
UCLASS()
class SNOWKAMI_API ASnowkami_TriggerBox : public ATriggerBox
{
	GENERATED_BODY()

protected:
	void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(Category = Overlap)
	virtual void OnActorEnterTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void Debug(FColor color, FString message, int indentCount);

	UFUNCTION()
	virtual void TriggerEventDebug(AActor* OtherActor);

	UPROPERTY(EditAnywhere)
	bool bDebugTrigger;

public:
	ASnowkami_TriggerBox();
	
};