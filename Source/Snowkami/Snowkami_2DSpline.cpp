// Fill out your copyright notice in the Description page of Project Settings.

#include "Snowkami_2DSpline.h"


// Sets default values
ASnowkami_2DSpline::ASnowkami_2DSpline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PlayerSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PlayerSpline"));
	RootComponent = PlayerSpline;
	CameraDirIndicator = CreateDefaultSubobject<UArrowComponent>(TEXT("CameraDirectionIndicator"));
	CameraDirIndicator->ArrowSize = 1.f;
	CameraDirIndicator->SetWorldLocation(PlayerSpline->GetComponentLocation());
}
