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

	UFUNCTION(BlueprintPure, Category ="Walk" )
	float GetMaxWalkSpeed() { return mMaxWalkSpeed; };
	UFUNCTION(BlueprintCallable, Category = "Walk")
	void SetMaxWalkSpeed(float iMaxWalkSpeed) { mMaxWalkSpeed = iMaxWalkSpeed; };

	UFUNCTION(BlueprintPure, Category = "Walk")
		float GetMaxStepHeight() { return mMaxStepHeight; };
	//UFUNCTION(BlueprintCallable, Category = "Walk")
		//void SetMaxStepHeight(float iMaxStepHeight) { mMaxStepHeight = iMaxStepHeight; };

	UFUNCTION(BlueprintPure, Category = "Movement")
		FVector GetGravity() { return mGravity; };
	UFUNCTION(BlueprintCallable, Category = "Movement")
		void SetGravity(FVector iGravity) { mGravity = iGravity; mGravityNormal = mGravity.GetSafeNormal(); };
	UFUNCTION(BlueprintPure, Category = "Movement")
		FVector GetGravityNormal() { return mGravityNormal; };

	UFUNCTION(BlueprintPure, Category = "Walk")
		float GetMaxSlope() { return mMaxSlope; };
	//UFUNCTION(BlueprintCallable, Category = "Walk")
		//void SetMaxSlope(float iMaxSlope) { mMaxSlope = iMaxSlope; };

private:
	UCharacterMovementComponent* mpMovement;
	UCapsuleComponent* mpCapsule;

	// Max speed when walking
	UPROPERTY(EditDefaultsOnly,  BlueprintGetter = GetMaxWalkSpeed, BlueprintSetter = SetMaxWalkSpeed, Category = "Walk")
	float mMaxWalkSpeed;

	// Max step height
	UPROPERTY(EditDefaultsOnly, BlueprintGetter = GetMaxStepHeight, Category = "Walk" )
	float mMaxStepHeight;

	// Max slope angle
	UPROPERTY(EditDefaultsOnly, BlueprintGetter = GetMaxSlope, Category = "Walk")
	float mMaxSlope;

	// Current Gravity
	UPROPERTY(EditDefaultsOnly, BlueprintGetter = GetGravity, BlueprintSetter = SetGravity, Category = "Movement", meta=(DisplayName = "Gravity"))
	FVector mGravity;
	// Normalized Gravity
	FVector mGravityNormal;
	
};
