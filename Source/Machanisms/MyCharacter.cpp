// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mMaxWalkSpeed = 600.0f;
	mMaxSlope = 45.0f;
	mMaxStepHeight = 50.0f;
	mGravity = FVector(0, 0, -980.0f);
	mAcceleration = 1000;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	mpMovement = GetCharacterMovement();
	mpCapsule = GetCapsuleComponent();
	mGravityNormal = mGravity.GetSafeNormal();
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

void AMyCharacter::MoveTo(FVector iLocation)
{
	FVector oldLocation = GetActorLocation();
	SetActorLocation(iLocation);
	if (auto world = GetWorld())
	{
		//mpMovement->Velocity = (iLocation - oldLocation) / world->GetDeltaSeconds();
		mpMovement->Velocity = mVelocity;
	}

}

void AMyCharacter::UpdateWalk()
{
	FVector normal;
	FVector direction = mpMovement->GetLastInputVector();

	Accelerate();

	if (!TryWalk(normal))
	{
		if (!bIsBesideWall)
		{
			// enter a wall
			bIsBesideWall = true;
			wallNormal = normal;
		}

		if ( (wallNormal - normal).SizeSquared() > 1)
		{
			// leave a wall
			bIsBesideWall = false;
		}

		// change direction
		mVelocity = FVector::VectorPlaneProject(mVelocity, mGravityNormal);
		mVelocity = FVector::VectorPlaneProject(mVelocity, wallNormal);

		if (!TryWalk(normal))
		{
			//UE_LOG(LogTemp, Log, TEXT("Player is blocked"));
		}
	}
	else
		bIsBesideWall = false;
}

void AMyCharacter::UpdateJump()
{
	FVector inputVector = mpMovement->GetLastInputVector();

	FVector diff = inputVector * mMaxWalkSpeed;
	SetActorLocation(GetActorLocation() + diff * GetWorld()->DeltaTimeSeconds);
}

bool AMyCharacter::TryWalk(FVector& oHitNormal)
{
	//FVector inputVector = iDirection;
	if (mpMovement->GetLastInputVector().Size() <= 0.001f)
	{
		mVelocity = mpMovement->Velocity = FVector::ZeroVector;
		return true;
	}

	FVector originalVelocity = mVelocity;

	if (UWorld* world = GetWorld())
	{
		FVector deltaMovement = originalVelocity * world->GetDeltaSeconds();
		FVector location = GetActorLocation();
		FVector newLocation = location + deltaMovement;		
		// check if next location is blocked
		FCollisionQueryParams queryParams;
		// ignore self
		queryParams.AddIgnoredActor(this);
		FCollisionResponseParams responseParams;
		
		FHitResult hitResult(ForceInit);

		// a little bit up
		if (!world->SweepSingleByChannel(hitResult, location - mGravityNormal, newLocation - mGravityNormal, GetActorRotation().Quaternion(), 
			ECollisionChannel::ECC_Visibility, FCollisionShape::MakeCapsule(mpCapsule->GetScaledCapsuleRadius(), 
			mpCapsule->GetScaledCapsuleHalfHeight()), queryParams, responseParams))
		{
			FVector bottom = newLocation + mGravityNormal * mMaxStepHeight;
			FHitResult newHit;

			if (world->SweepSingleByChannel(newHit, newLocation, bottom, GetActorRotation().Quaternion(), ECollisionChannel::ECC_Visibility,
				FCollisionShape::MakeCapsule(mpCapsule->GetScaledCapsuleRadius(), mpCapsule->GetScaledCapsuleHalfHeight()), queryParams, responseParams))
			{
				// step down or walk on a plane
				MoveTo(newLocation + mGravityNormal * newHit.Distance);
				//UE_LOG(LogTemp, Log, TEXT("Walk to %f, %f, %f"), newLocation.X, newLocation.Y, newLocation.Z);
			}
			else
			{
				// fall
				MoveTo(newLocation);
				//UE_LOG(LogTemp, Log, TEXT("Fall to %f, %f, %f"), newLocation.X, newLocation.Y, newLocation.Z);
			}

			return true;
		}
		else
		{
			FVector planeVector = FVector::VectorPlaneProject(hitResult.ImpactNormal, -mGravityNormal);
			float angle = FMath::RadiansToDegrees(FMath::Abs(FMath::Asin(planeVector.Size())));	
			//UE_LOG(LogTemp, Log, TEXT("Angle = %f"), angle);

			if (angle < mMaxSlope)
			{
				// walk on slope
				FVector top = newLocation - mGravityNormal * mMaxStepHeight;
				world->SweepSingleByChannel(hitResult, top, newLocation, GetActorRotation().Quaternion(), ECollisionChannel::ECC_Visibility,
					FCollisionShape::MakeCapsule(mpCapsule->GetScaledCapsuleRadius(), mpCapsule->GetScaledCapsuleHalfHeight()), queryParams, responseParams);

				MoveTo(top + mGravityNormal * hitResult.Distance);
				//UE_LOG(LogTemp, Log, TEXT("Walk on slope"));
				return true;
			}
			else
			{
				FVector top = newLocation - mGravityNormal * mMaxStepHeight;
				world->SweepSingleByChannel(hitResult, top, newLocation, GetActorRotation().Quaternion(), ECollisionChannel::ECC_Visibility,
					FCollisionShape::MakeCapsule(mpCapsule->GetScaledCapsuleRadius(), mpCapsule->GetScaledCapsuleHalfHeight()), queryParams, responseParams);

				// calculate angle and height of next stairs
				FVector nextPlaneVector = FVector::VectorPlaneProject(hitResult.ImpactNormal, -mGravityNormal);
				float nextAngle = FMath::RadiansToDegrees(FMath::Abs(FMath::Asin(nextPlaneVector.Size())));
				if (nextAngle < mMaxSlope && hitResult.Distance >= 0.001f)
				{
					// walk on stairs
					SetActorLocation(top + mGravityNormal * hitResult.Distance);
					mpMovement->Velocity = originalVelocity;
					//UE_LOG(LogTemp, Log, TEXT("Walk on stairs")); 
					return true;
				}
				else
				{
					oHitNormal = hitResult.ImpactNormal;
					//UE_LOG(LogTemp, Log, TEXT("Move beside wall")); 
					return false;
				}
			}
		}
	}

	return false;
}

void AMyCharacter::Accelerate()
{
	mVelocity = mpMovement->Velocity;
	mVelocity += mAcceleration * mpMovement->GetLastInputVector();

	if (mVelocity.SizeSquared() > mMaxWalkSpeed * mMaxWalkSpeed)
	{
		mVelocity = mVelocity.GetSafeNormal() * mMaxWalkSpeed;
	}
}

