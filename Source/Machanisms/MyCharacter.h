// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
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

	UFUNCTION(BlueprintCallable, Category = "Walk")
		void UpdateWalk();



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure)
	float GetMaxWalkSpeed() { return mMaxWalkSpeed; };
	UFUNCTION(BlueprintCallable)
	void SetMaxWalkSpeed(float iMaxWalkSpeed) { mMaxWalkSpeed = iMaxWalkSpeed; };

	UFUNCTION(BlueprintGetter, Category = "Walk")
		float GetMaxStepHeight() { return mMaxStepHeight; };
	UFUNCTION(BlueprintSetter, Category = "Walk")
		void SetMaxStepHeight(float iMaxStepHeight) { mMaxStepHeight = iMaxStepHeight; };

	UFUNCTION(BlueprintGetter, Category = "Movement")
		FVector GetGravity() { return mGravity; };
	UFUNCTION(BlueprintSetter, Category = "Movement")
		void SetGravity(FVector iGravity) { mGravity = iGravity; };

	UFUNCTION(BlueprintGetter, Category = "Walk")
		float GetMaxSlope() { return mMaxSlope; };
	UFUNCTION(BlueprintSetter, Category = "Walk")
		void SetMaxSlope(float iMaxSlope) { mMaxSlope = iMaxSlope; };

private:
	UCharacterMovementComponent* mpMovement;
	UCapsuleComponent* mpCapsule;

	// Max speed when walking
	UPROPERTY(BlueprintGetter = GetMaxWalkSpeed, BlueprintSetter = SetMaxWalkSpeed, Category = "Walk")
	float mMaxWalkSpeed;
	// Max step height
	float mMaxStepHeight;
	// Max slope angle
	float mMaxSlope;
	// Current Gravity
	FVector mGravity;
	// Normalized Gravity
	FVector mGravityNormal;
	
};
