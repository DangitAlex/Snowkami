// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Snowkami_2DSpline.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/TextRenderComponent.h"
#include "SnowkamiCharacter.generated.h"

UCLASS(config=Game)
class ASnowkamiCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UParticleSystemComponent* SnowVFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UTextRenderComponent* SnowText;

public:
	ASnowkamiCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:
	// Camera Inputs
	void MoveForward(float Value);
	void MoveRight(float Value);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void UpdateCameraFromMouse_Pitch(float fVal);
	void UpdateCameraFromMouse_Yaw(float fVal);

	// Hit Event
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Jumping
	void Jump() override;
	void StopJumping() override;

	UPROPERTY(BlueprintReadOnly, Category = Jumping)
		bool bIsJumpPressed;

	// Crouching
	void OnCrouchPressed();
	void OnCrouchReleased();

	void Crouch(bool bClientSimulation = false) override;
	void UnCrouch(bool bClientSimulation = false) override;


	UPROPERTY(BlueprintReadOnly, Category = Crouch)
		bool bCrouchPressed;

	// Yipping
	void OnYipPressed();
	void OnYipReleased();

	void Yip();

	// Snow Powers
	void OnSnowPressed();
	void OnSnowReleased();

	UPROPERTY(BlueprintReadOnly, Category = Snow)
		bool bIsSnowPressed;

	float SnowJump_Impulse;
	float SnowJump_AirControlMult;
	float SnowJump_WalkSpeedMult;

	float lastSnowPressedTime;
	float lastSnowReleasedTime;

	// Running
	void OnRunPressed();
	void OnRunReleased();

	UPROPERTY(BlueprintReadOnly, Category = Running)
		bool bRunPressed;

	float RunningSpeedMult;

	// Tick Functions
	void Tick(float DeltaTime) override;
	void Tick_UpdatePlayer2D(float DeltaTime);
	void Tick_UpdateCamera(float DeltaTime);
	void Tick_CheckForWallAction(float DeltaTime);

	void SetNewPlayerCameraMode(bool bUse2D);
	void DebugPlayer(FColor color, FString message, int indentCount);

	bool bIsPerformingWallAction;

	ASnowkami_2DSpline* currentPlayerSpline;

	FRotator TargetControlRotation_2D;
	float CameraInterpSpeed_2D;
	
	float TargetCameraDistance;
	float TargetCameraDistance_2D;
	float CameraInterpSpeeed_Distance;

	float TargetCameraFOV_Default;
	float TargetCameraFOV_Mult_Current;
	float TargetCameraFOV_Mult_MAX;
	float CameraInterpSpeed_FOV;

	FVector TargetCameraOffset;
	FRotator TargetCameraRotation;

	float CameraInterpSpeed_Offset;
	float CameraInterpSpeed_Rotation;

	float FOVWarpStartPitch;
	float FOVWarpEndPitch;

	bool bTempUseNon2DControls;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	ASnowkamiCharacter* GetDefaultPlayerObject();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE class ASnowkami_2DSpline* GetCurrentPlayerSpline() const { return currentPlayerSpline;  }

	UFUNCTION()
	void SetCurrentPlayerSpline(ASnowkami_2DSpline* NewSpline);
};

