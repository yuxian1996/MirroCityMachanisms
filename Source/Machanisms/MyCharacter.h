// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Public/DrawDebugHelpers.h"
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

	UFUNCTION(BlueprintCallable, Category = "Walk")
		void UpdateWalk();

	UFUNCTION(BlueprintCallable, Category = "Jump")
		void UpdateJump();

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

	UFUNCTION(BlueprintPure, Category = "Walk")
		float GetMaxSlope() { return mMaxSlope; };
	UFUNCTION(BlueprintPure, Category = "Walk")
		float GetMaxStepHeight() { return mMaxStepHeight; };

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


	// Normalized Gravity
	FVector mGravityNormal;

	UCharacterMovementComponent* mpMovement;
	UCapsuleComponent* mpCapsule;
	bool bIsBesideWall = false;
	FVector wallNormal;
	FVector mVelocity;

	void MoveTo(FVector iLocation);
	bool TryWalk(FVector& oHitNormal);
	void Accelerate();
};
