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
	
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->CrouchedHalfHeight = 9.f;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->bCanWalkOffLedges = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 125.f;

	// set our turn rates for input
	BaseTurnRate = 60.f;
	BaseLookUpRate = 60.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 225.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 350.f;
	GetCharacterMovement()->AirControl = 0.2f;
	/*GetCharacterMovement()->GravityScale = 1.f;
	GetCharacterMovement()->BrakingFriction = 0.f;
	GetCharacterMovement()->GroundFriction = 0.f;
	GetCharacterMovement()->BrakingFrictionFactor = 0.f;*/

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 75.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	SnowVFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SnowVFX")); 
	SnowVFX->SetupAttachment(RootComponent);

	SnowText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SnowText"));
	SnowText->SetupAttachment(RootComponent);
	SnowText->SetHiddenInGame(true);

	bIsJumpPressed = false;
	bRunPressed = false;
	bIsSnowPressed = false;
	bCrouchPressed = false;

	SnowJump_Impulse = 250.f;
	SnowJump_AirControlMult = 1.f;
	SnowJump_WalkSpeedMult = 2.f;

	RunningSpeedMult = 1.5f;
	
	currentPlayerSpline = NULL;

	TargetCameraDistance = CameraBoom->TargetArmLength;
	TargetCameraDistance_2D = 160.f;
	TargetCameraFOV_Default = 90.f;
	TargetCameraFOV_Mult_Current = 1.f;
	TargetCameraFOV_Mult_MAX = 1.3f;

	TargetCameraOffset = FVector();
	TargetCameraRotation = FRotator();
	TargetControlRotation_2D = FRotator();

	FOVWarpStartPitch = 0.f;
	FOVWarpEndPitch = 50.f;

	CameraInterpSpeed_2D = 3.f;
	CameraInterpSpeeed_Distance = 4.f;
	CameraInterpSpeed_FOV = 2.f;
	
	bTempUseNon2DControls = false;

	Cast<UShapeComponent>(GetRootComponent())->SetNotifyRigidBodyCollision(true);
	Cast<UShapeComponent>(GetRootComponent())->OnComponentHit.AddDynamic(this, &ASnowkamiCharacter::OnHit);
}

ASnowkamiCharacter* ASnowkamiCharacter::GetDefaultPlayerObject()
{
	return Cast<ASnowkamiCharacter>(GetClass()->GetDefaultObject());
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASnowkamiCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASnowkamiCharacter::OnCrouchPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASnowkamiCharacter::OnCrouchReleased);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASnowkamiCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASnowkamiCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick

	PlayerInputComponent->BindAxis("Turn", this, &ASnowkamiCharacter::UpdateCameraFromMouse_Yaw);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASnowkamiCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ASnowkamiCharacter::UpdateCameraFromMouse_Pitch);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASnowkamiCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Yip", IE_Pressed, this, &ASnowkamiCharacter::OnYipPressed);
	PlayerInputComponent->BindAction("Yip", IE_Released, this, &ASnowkamiCharacter::OnYipReleased);

	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ASnowkamiCharacter::OnRunPressed);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &ASnowkamiCharacter::OnRunReleased);

	PlayerInputComponent->BindAction("Snow", IE_Pressed, this, &ASnowkamiCharacter::OnSnowPressed);
	PlayerInputComponent->BindAction("Snow", IE_Released, this, &ASnowkamiCharacter::OnSnowReleased);
}

void ASnowkamiCharacter::UpdateCameraFromMouse_Pitch(float fVal)
{
	if ((Controller != NULL) && (fVal != 0.0f))
	{
		if (currentPlayerSpline == NULL)
			AddControllerPitchInput(fVal);
	}
}

void ASnowkamiCharacter::UpdateCameraFromMouse_Yaw(float fVal)
{
	if ((Controller != NULL) && (fVal != 0.0f))
	{
		if (currentPlayerSpline == NULL)
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
	if (Controller != NULL)
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

void ASnowkamiCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
 	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling() && bIsJumpPressed)
	{
		//Hit.Normal();
		//DebugPlayer(FColor::White, "HIT", 0);
		bIsPerformingWallAction = true;
		GetCharacterMovement()->AddImpulse(FVector(0.f, 0.f, 9.f), true);
	}
}

void ASnowkamiCharacter::Jump()
{
	Super::Jump();

	bIsJumpPressed = true;

	if (CanJump() && bIsSnowPressed)
	{
		GetCharacterMovement()->AirControl = SnowJump_AirControlMult;
		GetCharacterMovement()->AddImpulse(FVector(0.f, 0.f, SnowJump_Impulse), true);
		GetCharacterMovement()->MaxWalkSpeed = GetDefaultPlayerObject()->GetCharacterMovement()->MaxWalkSpeed * SnowJump_WalkSpeedMult;
		SnowVFX->SetActive(false);
	}
}

void ASnowkamiCharacter::StopJumping()
{
	Super::StopJumping();

	bIsJumpPressed = false;
}

void ASnowkamiCharacter::OnCrouchPressed() { if(CanCrouch()) Crouch(false); }
void ASnowkamiCharacter::OnCrouchReleased() { UnCrouch(false); }

void ASnowkamiCharacter::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);

	bCrouchPressed = true;

	GetCharacterMovement()->MaxWalkSpeed = GetDefaultPlayerObject()->GetCharacterMovement()->MaxWalkSpeed;
}

void ASnowkamiCharacter::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);

	bCrouchPressed = false;

	if (bRunPressed)
		OnRunPressed();

}

void ASnowkamiCharacter::OnYipPressed()
{
	Yip();
}

void ASnowkamiCharacter::OnYipReleased()
{
	SnowText->SetHiddenInGame(true);
}

void ASnowkamiCharacter::Yip()
{
	SnowText->SetHiddenInGame(false);
}

void ASnowkamiCharacter::OnRunPressed()
{
	bRunPressed = true;

	if (!bCrouchPressed)
	{
		GetCharacterMovement()->MaxWalkSpeed = GetDefaultPlayerObject()->GetCharacterMovement()->MaxWalkSpeed * RunningSpeedMult;
		/*GetCharacterMovement()->MaxAcceleration = 1024;
		GetCharacterMovement()->BrakingFriction = 0.f;
		GetCharacterMovement()->GroundFriction = 0.f;
		GetCharacterMovement()->BrakingFrictionFactor = 0.f;*/
	}
}

void ASnowkamiCharacter::OnRunReleased()
{
	bRunPressed = false;
	GetCharacterMovement()->MaxWalkSpeed = GetDefaultPlayerObject()->GetCharacterMovement()->MaxWalkSpeed;
}

void ASnowkamiCharacter::OnSnowPressed()
{
	bIsSnowPressed = true;
	lastSnowPressedTime = GetGameTimeSinceCreation();
	SnowVFX->SetActive(true);
}

void ASnowkamiCharacter::OnSnowReleased()
{
	bIsSnowPressed = false;
	lastSnowReleasedTime = GetGameTimeSinceCreation();
	SnowVFX->SetActive(false);
}

void ASnowkamiCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	switch (GetCharacterMovement()->MovementMode)
	{
		case EMovementMode::MOVE_Walking:
			GetCharacterMovement()->AirControl = GetDefaultPlayerObject()->GetCharacterMovement()->AirControl;
			GetCharacterMovement()->MaxWalkSpeed = GetDefaultPlayerObject()->GetCharacterMovement()->MaxWalkSpeed;
			bIsPerformingWallAction = false;

			if (bRunPressed)
				OnRunPressed();

			if(bIsSnowPressed)
				SnowVFX->SetActive(true);
			break;

		case EMovementMode::MOVE_Falling:
			GetCharacterMovement()->MaxWalkSpeed = ASnowkamiCharacter::GetCharacterMovement()->MaxWalkSpeed;
			break;
		case EMovementMode::MOVE_Swimming:

			break;
	}
}

void ASnowkamiCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (currentPlayerSpline != NULL && currentPlayerSpline->GetPlayerSpline() != NULL)
		Tick_UpdatePlayer2D(DeltaTime);

	if (GetCharacterMovement()->IsFalling())
		Tick_CheckForWallAction(DeltaTime);

	Tick_UpdateCamera(DeltaTime);

	if (SnowText && !SnowText->bHiddenInGame)
		SnowText->SetWorldRotation((FollowCamera->GetComponentLocation() - SnowText->GetComponentLocation()).Rotation());
}

void ASnowkamiCharacter::Tick_UpdatePlayer2D(float DeltaTime)
{
	if (currentPlayerSpline == NULL || currentPlayerSpline->GetPlayerSpline() == NULL)
		return;

	FVector closestLocation = currentPlayerSpline->GetPlayerSpline()->FindLocationClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World);
	//DrawDebugSphere(GetWorld(), closestLocation, 30.f, 4, FColor::Purple, false, 0.03f, (uint8)'\000', 1.f);

	SetActorLocation(FMath::VInterpConstantTo(GetActorLocation(), FVector(closestLocation.X, closestLocation.Y, GetActorLocation().Z), DeltaTime, 500.f));
	TargetControlRotation_2D = (-currentPlayerSpline->GetPlayerSpline()->FindRightVectorClosestToWorldLocation(closestLocation, ESplineCoordinateSpace::World)).Rotation();
	GetController()->SetControlRotation(FMath::RInterpTo(GetControlRotation(), TargetControlRotation_2D, DeltaTime, CameraInterpSpeed_2D));
}

void ASnowkamiCharacter::Tick_CheckForWallAction(float DeltaTime)
{

}

void ASnowkamiCharacter::Tick_UpdateCamera(float DeltaTime)
{
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetCameraDistance, DeltaTime, CameraInterpSpeeed_Distance);
	CameraBoom->TargetOffset = FMath::VInterpTo(CameraBoom->TargetOffset, GetDefaultPlayerObject()->CameraBoom->TargetOffset + TargetCameraOffset, DeltaTime, CameraInterpSpeed_Offset);
	FollowCamera->RelativeRotation = FMath::RInterpTo(FollowCamera->RelativeRotation, GetDefaultPlayerObject()->FollowCamera->RelativeRotation + TargetCameraRotation, DeltaTime, CameraInterpSpeed_Offset);

	// Warp FOV when looking up
	if (GetControlRotation().Pitch > FOVWarpStartPitch && GetControlRotation().Pitch <= 90.f)
	{
		TargetCameraFOV_Mult_Current = FMath::Lerp(1.f, TargetCameraFOV_Mult_MAX, (FMath::Clamp(((GetControlRotation().Pitch - FOVWarpStartPitch) / (FOVWarpEndPitch - FOVWarpStartPitch)), 0.f, 1.f)));
		//DebugPlayer(FColor::Yellow, FString::SanitizeFloat(FMath::Clamp(((GetControlRotation().Pitch - FOVWarpStartPitch) / (FOVWarpEndPitch - FOVWarpStartPitch)), 0.f, 1.f)) + " :: " + FString::SanitizeFloat(TargetCameraFOV_Mult_Current), 0);
	}
	else
		TargetCameraFOV_Mult_Current = 1.f;

	FollowCamera->FieldOfView = FMath::FInterpTo(FollowCamera->FieldOfView, (TargetCameraFOV_Default * TargetCameraFOV_Mult_Current), DeltaTime, CameraInterpSpeed_FOV);
}

void ASnowkamiCharacter::SetNewPlayerCameraMode(bool bUse2D)
{
	if (bUse2D)
	{
		TargetCameraDistance = TargetCameraDistance_2D;
		bTempUseNon2DControls = true;
	}
	else
	{
		TargetCameraDistance = GetDefaultPlayerObject()->TargetCameraDistance;
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
