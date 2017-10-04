// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SnowkamiCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"

//////////////////////////////////////////////////////////////////////////
// ASnowkamiCharacter

ASnowkamiCharacter::ASnowkamiCharacter()
{
	//PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	bIsSnowPressed = false;

	currentPlayerSpline = NULL;
	currentPlayerSpline_Distance = 0.f;

	TargetCameraDistance = 130.f;

	CameraInterpSpeed_2D = 3.f;
	CameraInterpSpeeed_Distance = 4.f;
	
	bTempUseNon2DControls = false;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASnowkamiCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASnowkamiCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASnowkamiCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ASnowkamiCharacter::UpdateCameraFromMouse_Yaw);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASnowkamiCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ASnowkamiCharacter::UpdateCameraFromMouse_Pitch);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASnowkamiCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Snow", IE_Pressed, this, &ASnowkamiCharacter::OnSnowPressed);
	PlayerInputComponent->BindAction("Snow", IE_Released, this, &ASnowkamiCharacter::OnSnowReleased);
}

void ASnowkamiCharacter::UpdateCameraFromMouse_Pitch(float fVal)
{
	if ((Controller != NULL) && (fVal != 0.0f))
	{
		if (currentPlayerSpline != NULL)
		{

		}
		else
			AddControllerPitchInput(fVal);
	}
}

void ASnowkamiCharacter::UpdateCameraFromMouse_Yaw(float fVal)
{
	if ((Controller != NULL) && (fVal != 0.0f))
	{
		if (currentPlayerSpline != NULL)
		{

		}
		else
			AddControllerYawInput(fVal);
	}
}

void ASnowkamiCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASnowkamiCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASnowkamiCharacter::MoveForward(float Value)
{
	if ((Controller != NULL))
	{
		if ((Value != 0.0f) && currentPlayerSpline == NULL)
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
		else if (currentPlayerSpline != NULL && bTempUseNon2DControls)
		{
			if (FMath::Abs(Value) > 0.f)
				AddMovementInput(GetVelocity().ProjectOnToNormal(currentPlayerSpline->GetPlayerSpline()->FindTangentClosestToWorldLocation(currentPlayerSpline->GetPlayerSpline()->FindLocationClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World), ESplineCoordinateSpace::World)), FMath::Abs(Value));
			else
				bTempUseNon2DControls = false;
		}
	}
}

void ASnowkamiCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ASnowkamiCharacter::Jump()
{
	Super::Jump();

	if(CanJump() && bIsSnowPressed)
		GetCharacterMovement()->AddImpulse(FVector(0.f, 0.f, 400.f), true);
}

void ASnowkamiCharacter::OnSnowPressed()
{
	bIsSnowPressed = true;
	lastSnowPressedTime = GetGameTimeSinceCreation();
}

void ASnowkamiCharacter::OnSnowReleased()
{
	bIsSnowPressed = false;
	lastSnowReleasedTime = GetGameTimeSinceCreation();
}

void ASnowkamiCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (currentPlayerSpline != NULL && currentPlayerSpline->GetPlayerSpline() != NULL)
	{
		FVector closestLocation = currentPlayerSpline->GetPlayerSpline()->FindLocationClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World);

		DrawDebugSphere(GetWorld(), closestLocation, 30.f, 4, FColor::Purple, false, 0.03f, (uint8)'\000', 1.f);

		SetActorLocation(FMath::VInterpConstantTo(GetActorLocation(), FVector(closestLocation.X, closestLocation.Y, GetActorLocation().Z), DeltaTime, 500.f));

		Target2DCameraRotation = (-currentPlayerSpline->GetPlayerSpline()->FindRightVectorClosestToWorldLocation(closestLocation, ESplineCoordinateSpace::World)).Rotation();

		GetController()->SetControlRotation(FMath::RInterpTo(GetControlRotation(), Target2DCameraRotation, DeltaTime, CameraInterpSpeed_2D));
	}

	UpdateCamera(DeltaTime);
}

void ASnowkamiCharacter::UpdateCamera(float DeltaTime)
{
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetCameraDistance, DeltaTime, CameraInterpSpeeed_Distance);
	//DebugPlayer(FColor::Yellow, FString::SanitizeFloat(CameraBoom->TargetArmLength), 0);
}

void ASnowkamiCharacter::SetNewPlayerCameraMode(bool bUse2D)
{
	if (bUse2D)
	{
		TargetCameraDistance = 250.f;
		bTempUseNon2DControls = true;
	}
	else
	{
		TargetCameraDistance = 130.f;
		bTempUseNon2DControls = false;
	}
}

void ASnowkamiCharacter::DebugPlayer(FColor color, FString message, int indentCount)
{
	FString debugMessage;

	if (indentCount > 0)
	{
		for (int i = 0; i < indentCount; i++)
		{
			debugMessage.Append("  ");
		}
	}

	debugMessage.Append("Player :: " + message);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, color, debugMessage);
}

void ASnowkamiCharacter::SetCurrentPlayerSpline(ASnowkami_2DSpline* NewSpline)
{
	currentPlayerSpline = NewSpline;

	if (currentPlayerSpline)
		SetNewPlayerCameraMode(true);
	else
		SetNewPlayerCameraMode(false);
}
