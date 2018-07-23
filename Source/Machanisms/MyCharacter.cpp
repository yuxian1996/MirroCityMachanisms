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
	mCntSlope = 0;
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
		mpMovement->Velocity = (iLocation - oldLocation) / world->GetDeltaSeconds();
	}

}

void AMyCharacter::UpdateWalk()
{
	FVector inputVector = mpMovement->GetLastInputVector();
	FVector originalVelocity = inputVector * mMaxWalkSpeed;
	float speed = originalVelocity.Size();

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
	
		// a little bit up
		if (!world->OverlapAnyTestByChannel(newLocation - mGravityNormal * 1, GetActorRotation().Quaternion(), ECollisionChannel::ECC_Visibility,
			FCollisionShape::MakeCapsule(mpCapsule->GetScaledCapsuleRadius(), mpCapsule->GetScaledCapsuleHalfHeight_WithoutHemisphere()),
			queryParams, responseParams))
		{
			FVector end = newLocation + mGravityNormal * (mMaxStepHeight + mpCapsule->GetScaledCapsuleHalfHeight());
			FHitResult hitResult;

			if (world->LineTraceSingleByChannel(hitResult, newLocation, end, ECC_Visibility, queryParams, responseParams))
			{
				// walk on plane or down
				FVector nextLocation = hitResult.Location - mGravityNormal * mpCapsule->GetScaledCapsuleHalfHeight();
				MoveTo(nextLocation);
				UE_LOG(LogTemp, Log, TEXT("Walk to %f, %f, %f"), nextLocation.X, nextLocation.Y, nextLocation.Z);
			}
			else
			{
				// fall
				SetActorLocation(newLocation);
				MoveTo(newLocation);
				UE_LOG(LogTemp, Log, TEXT("Fall to %f, %f, %f"), newLocation.X, newLocation.Y, newLocation.Z);
			}
		}
		else
		{
			FVector originalDirection = originalVelocity.GetSafeNormal();
			FVector bottom = location + mGravityNormal * (mpCapsule->GetScaledCapsuleHalfHeight() - 5);
			FVector end = bottom + originalDirection * mpCapsule->GetScaledCapsuleRadius() + deltaMovement;
			// try to change direction or stop
			FHitResult hitResult;
			if (world->LineTraceSingleByChannel(hitResult, bottom, end, ECollisionChannel::ECC_Visibility, queryParams, responseParams))
			{
				FVector planeVector = FVector::VectorPlaneProject(hitResult.Normal, -mGravityNormal);
				
				float angle = FMath::RadiansToDegrees(FMath::Abs(FMath::Asin(planeVector.Size())));
				//UE_LOG(LogTemp, Log, TEXT("Angle = %f"), angle);

				if (angle < mMaxSlope)
				{
					// walk on slope 
					FVector traceStart = newLocation - mGravityNormal * mMaxStepHeight;
					FVector traceEnd = traceStart + mGravityNormal *(mpCapsule->GetScaledCapsuleHalfHeight() + mMaxStepHeight + 10);
					// find next location
					if (world->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECollisionChannel::ECC_Visibility,
						queryParams, responseParams))
					{
						FVector nextLocation = hitResult.Location - mGravityNormal * mpCapsule->GetScaledCapsuleHalfHeight();
						MoveTo(nextLocation);
						UE_LOG(LogTemp, Log, TEXT("Walk on slope to %f, %f, %f"), nextLocation.X, nextLocation.Y, nextLocation.Z);

					}
				}
				else
				{
					FVector maxBottom = bottom - mGravityNormal * mMaxStepHeight;
					FVector maxEnd = end - mGravityNormal * mMaxStepHeight;
					// walk on stairs
					if (!world->LineTraceSingleByChannel(hitResult, maxBottom, maxEnd, ECollisionChannel::ECC_Visibility,
						queryParams, responseParams))
					{
						world->LineTraceSingleByChannel(hitResult, end - mGravityNormal * mMaxStepHeight, end,
							ECollisionChannel::ECC_Visibility, queryParams, responseParams);
						FVector nextLocation = newLocation - mGravityNormal * (mMaxStepHeight - hitResult.Distance);
						MoveTo(nextLocation);
						UE_LOG(LogTemp, Log, TEXT("Walk on stairs to %f, %f, %f"), nextLocation.X, nextLocation.Y, nextLocation.Z);

					}
				}
			}
			else
			{
				//SetActorLocation(newLocation);
				//UE_LOG(LogTemp, Log, TEXT("Tmp walk to %f, %f, %f"), newLocation.X, newLocation.Y, newLocation.Z);
			}
		}
	}

}

