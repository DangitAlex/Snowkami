// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Snowkami_2DSpline.h"
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
public:
	ASnowkamiCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	void UpdateCameraFromMouse_Pitch(float fVal);
	void UpdateCameraFromMouse_Yaw(float fVal);

	void Jump();

	void OnSnowPressed();
	void OnSnowReleased();

	void OnRunPressed();
	void OnRunReleased();

	void Tick(float DeltaTime) override;
	void UpdatePlayer2D(float DeltaTime);
	void UpdateCamera(float DeltaTime);

	void SetNewPlayerCameraMode(bool bUse2D);
	void DebugPlayer(FColor color, FString message, int indentCount);

	UPROPERTY(BlueprintReadOnly, Category = Snow)
		bool bWantsToRun;

	float RunningSpeedMult;

	UPROPERTY(BlueprintReadOnly, Category = Snow)
		bool bIsSnowPressed;

	float SnowJump_Impulse;
	float SnowJump_AirControlMult;
	float SnowJump_WalkSpeedMult;

	float lastSnowPressedTime;
	float lastSnowReleasedTime;

	ASnowkami_2DSpline* currentPlayerSpline;

	FRotator Target2DCameraRotation;
	float CameraInterpSpeed_2D;
	
	float TargetCameraDistance;
	float TargetCameraDistance_2D;
	float CameraInterpSpeeed_Distance;

	float TargetCameraFOV_Default;
	float TargetCameraFOV_Mult_Current;
	float TargetCameraFOV_Mult_MAX;
	float CameraInterpSpeed_FOV;

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

