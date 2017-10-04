// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"
#include "Snowkami_2DSpline.generated.h"

UCLASS()
class SNOWKAMI_API ASnowkami_2DSpline : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASnowkami_2DSpline();

protected:

	UPROPERTY(EditAnywhere)
		USplineComponent* PlayerSpline;

	UPROPERTY(EditAnywhere)
		UArrowComponent* CameraDirIndicator;

public:	
	FORCEINLINE const FRotator GetSplineTargetCameraDirection() { return CameraDirIndicator->GetComponentRotation(); };
	FORCEINLINE const USplineComponent* GetPlayerSpline() { return PlayerSpline; };
	
};
