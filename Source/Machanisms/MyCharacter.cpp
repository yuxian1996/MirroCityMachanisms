// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	mpMovement = GetCharacterMovement();
	mpCapsule = GetCapsuleComponent();
	
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMyCharacter::UpdateWalk()
{
	FVector inputVector = mpMovement->GetLastInputVector();
	FVector originalVelocity = inputVector * mMaxWalkSpeed;
	if (UWorld* world = GetWorld())
	{
		FVector deltaMovement = originalVelocity * world->TimeSeconds;
		FVector location = GetActorLocation();
		FVector newLocation = location + deltaMovement;

		// check if next location is blocked
		FCollisionQueryParams queryParams;
		// ignore self
		queryParams.AddIgnoredActor(this);
		FCollisionResponseParams responseParams;
		// a little bit up
		if (!world->OverlapAnyTestByChannel(newLocation - mGravity.Normalize() * 5, GetActorRotation().Quaternion(), ECollisionChannel::ECC_Visibility,
			FCollisionShape::MakeCapsule(mpCapsule->GetScaledCapsuleRadius(), mpCapsule->GetScaledCapsuleHalfHeight()),
			queryParams, responseParams))
		{
			mpMovement->Velocity = originalVelocity;
			mpMovement->UpdateBasedMovement(world->TimeSeconds);
			//SetActorLocation(newLocation);
		}
		else
		{
			FVector originalDirection = originalVelocity.GetSafeNormal();
			FVector bottom = location + mGravity.GetSafeNormal() * (mpCapsule->GetScaledCapsuleHalfHeight() - 1.0f);
			FVector end = bottom + originalDirection * mpCapsule->GetScaledCapsuleRadius() + deltaMovement;
			// try to change direction or stop
			FHitResult hitResult;
			if (world->LineTraceSingleByChannel(hitResult, bottom, end, ECollisionChannel::ECC_Visibility, queryParams, responseParams))
			{
				FVector planeVector = FVector::VectorPlaneProject(hitResult.Normal, -mGravityNormal);
				float angle = FMath::Acos(planeVector.Size());

				if (angle < mMaxSlope)
				{
					// walk on slope 
					FVector axis = FVector::CrossProduct(originalDirection, -mGravityNormal);
					FVector newVelocity = originalVelocity.RotateAngleAxis(angle, axis);
					mpMovement->Velocity = newVelocity;
					mpMovement->UpdateBasedMovement(world->TimeSeconds);
				}
				else
				{
					FVector maxBottom = bottom - mGravityNormal * mMaxStepHeight;
					FVector maxEnd = end - mGravityNormal * mMaxStepHeight;
					// walk on stairs
					if (!world->LineTraceSingleByChannel(hitResult, maxBottom, maxEnd, ECollisionChannel::ECC_Visibility, 
						queryParams, responseParams))
					{
						world->LineTraceSingleByChannel(hitResult, end, end + mGravityNormal * mMaxStepHeight,
							ECollisionChannel::ECC_Visibility, queryParams, responseParams);
						mpMovement->Velocity = originalVelocity;
						SetActorLocation(location - mGravityNormal * (mMaxStepHeight - hitResult.Distance));
					}

				}
			}

		}

		

	}

}

