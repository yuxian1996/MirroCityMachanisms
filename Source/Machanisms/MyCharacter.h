// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Engine/World.h"
#include "MyCharacter.generated.h"

UCLASS()
class MACHANISMS_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to update velocity when player is walking
	UFUNCTION(BlueprintCallable, Category = "Walk")
		void UpdateWalk();

	// Called to update velocity when player is jumping
	UFUNCTION(BlueprintCallable, Category = "Jump")
		void UpdateJump();

	// Called to update player location when player is holding a ledge
	UFUNCTION(BlueprintCallable, Category = "Hold")
		FVector DetectLedge();

	// Called to start to change gravity by an angular speed (degree/second)
	UFUNCTION(BlueprintCallable, Category = "Movement")
		void ChangeGravity(FVector iGravity, float iAngularSpeed, FVector iAxis = FVector::ZeroVector);

	// Called to determine if player is on ground
	UFUNCTION(BlueprintPure, Category = "Movement")
		bool IsOnGround();

	// Called to determine if player is touching ceil
	UFUNCTION(BlueprintPure, Category = "Movement")
		bool IsTouchingCeil();

	// Called to get the closest ledge
	UFUNCTION(BlueprintCallable, Category = "HoldOnLedge")
		AActor* GetLedge();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Getter and Setter of max walk speed
	UFUNCTION(BlueprintPure, Category ="Walk" )
	float GetMaxWalkSpeed() { return mMaxWalkSpeed; };
	UFUNCTION(BlueprintCallable, Category = "Walk")
	void SetMaxWalkSpeed(float iMaxWalkSpeed) { mMaxWalkSpeed = iMaxWalkSpeed; };

	// Getter and Setter of gravity
	UFUNCTION(BlueprintPure, Category = "Movement")
		FVector GetGravity() { return mGravity; };
	UFUNCTION(BlueprintCallable, Category = "Movement")
		void SetGravity(FVector iGravity) { mGravity = iGravity; mGravityNormal = mGravity.GetSafeNormal(); };
	UFUNCTION(BlueprintPure, Category = "Movement")
		FVector GetGravityNormal() { return mGravityNormal; };

	// Getter and Setter of acceleration
	UFUNCTION(BlueprintPure, Category = "Walk")
		float GetAcceleration() { return mAcceleration; };
	UFUNCTION(BlueprintCallable, Category = "Walk")
		void SetAcceleration(float iAcc) { mAcceleration = iAcc; };

	// Getter and Setter of AirControl
	UFUNCTION(BlueprintPure, Category = "Jump")
		float GetAirControl() { return mAirControl; };
	UFUNCTION(BlueprintCallable, Category = "Jump")
		void SetAirControl(float iAirControl) { mAirControl = iAirControl; };

	// Getter and Setter of max slope
	UFUNCTION(BlueprintPure, Category = "Walk")
		float GetMaxSlope() { return mMaxSlope; };
	UFUNCTION(BlueprintPure, Category = "Walk")
		float GetMaxStepHeight() { return mMaxStepHeight; };

	// Getter of bIsChangingGravity
	UFUNCTION(BlueprintPure, Category = "Movement")
		bool IsChangingGravity() { return bIsChangingGravity; };

	//
	UFUNCTION(BlueprintPure, Category = "Walk")
		bool IsSteppingDown() { return bIsSteppingDown; };

	// Setter of mLedgeDirection (normal vector)
	UFUNCTION(BlueprintCallable, Category = "HoldLedge")
		void SetLedgeDirection(FVector iDirection) { mLedgeDirection = iDirection; };

private:
	// Max speed when walking
	UPROPERTY(EditDefaultsOnly, Category = "Walk", meta = (DisplayName = "Max Walk Speed"))
		float mMaxWalkSpeed;

	// Max step height
	UPROPERTY(EditDefaultsOnly, Category = "Walk", meta = (DisplayName = "Max Step Height"))
		float mMaxStepHeight;

	// Max slope angle
	UPROPERTY(EditDefaultsOnly,Category = "Walk", meta = (DisplayName = "Max Slope"))
		float mMaxSlope;

	// Current Gravity
	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (DisplayName = "Gravity"))
		FVector mGravity;

	// Acceleration
	UPROPERTY(EditDefaultsOnly, Category = "Walk", meta = (DisplayName = "Acceleration"))
		float mAcceleration;

	// Air Control When Jumping
	UPROPERTY(EditDefaultsOnly, Category = "Jump", meta = (DisplayName = "Air Control"))
		float mAirControl;

	// Is Changing Gravity
	UPROPERTY(VisibleAnywhere, Category = "Movement", meta = (DisplayName = "Is Changing Gravity"))
		bool bIsChangingGravity;

	// Direction of the Ledge Held By Player
	UPROPERTY(VisibleAnywhere, Category = "HoldLedge", meta = (DisplayName = "Ledge Direction"))
		FVector mLedgeDirection;

	UPROPERTY(VisibleAnywhere, Category = "Walk", meta = (DisplayName = "Is Stepping Down"))
		bool bIsSteppingDown;


	// Normalized Gravity
	FVector mGravityNormal;

	UCharacterMovementComponent* mpMovement;
	UCapsuleComponent* mpCapsule;
	UCameraComponent* mCamera;

	bool bIsBesideWall = false;
	FVector wallNormal;
	FVector mVelocity;
	FTimerHandle mGravityHandle;
	FTimerDelegate mGravityDel;
	FTimerHandle mStairHandle;
	FTimerDelegate mStairDel;
	float mMaxLedgeDistance;
	float mMaxLedgeHeight;
	
	void MoveTo(FVector iLocation);
	bool TryWalk(FVector& oHitNormal);
	void Accelerate();

	UFUNCTION()
		void ChangeGravityFunc(float iSpeed, float iRoll, FVector iRotateAxis);

	UFUNCTION()
		void MoveOnStairs(FVector iDirection, float iDistance, float iSpeed);
};
