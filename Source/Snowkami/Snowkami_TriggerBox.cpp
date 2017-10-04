// Fill out your copyright notice in the Description page of Project Settings.

#include "Snowkami_TriggerBox.h"

ASnowkami_TriggerBox::ASnowkami_TriggerBox()
{
	GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &ASnowkami_TriggerBox::OnActorEnterTrigger);
	Cast<UBoxComponent>(GetCollisionComponent())->SetBoxExtent(FVector(200.f));

	bDebugTrigger = false;
}

void ASnowkami_TriggerBox::OnConstruction(const FTransform& Transform)
{
	SetActorHiddenInGame(!bDebugTrigger);
}

void ASnowkami_TriggerBox::OnActorEnterTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bDebugTrigger)
		TriggerEventDebug(OtherActor);
}

void ASnowkami_TriggerBox::TriggerEventDebug(AActor* OtherActor)
{
	Debug(FColor::Yellow, " :: Triggered by " + OtherActor->GetName(), 0);
}

void ASnowkami_TriggerBox::Debug(FColor color, FString message, int indentCount)
{
	FString debugMessage;

	if (indentCount > 0)
	{
		for (int i = 0; i < indentCount; i++)
		{
			debugMessage.Append("  ");
		}
	}

	debugMessage.Append(GetName() + message);

	if (GEngine && bDebugTrigger)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, color, debugMessage);
}