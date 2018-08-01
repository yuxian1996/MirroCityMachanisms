// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"
#include "Public/TimerManager.h"
#include "Public/DrawDebugHelpers.h"
#include "Public/UObject/ConstructorHelpers.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mMaxWalkSpeed = 600.0f;
	mMaxSlope = 45.0f;
	mMaxStepHeight = 50.0f;
	mGravity = FVector(0, 0, -980.0f);
	mAcceleration = 800;

	auto finder = ConstructorHelpers::FClassFinder<AActor>(TEXT("/Game/FirstPersonBP/Blueprints/Ledge"));
	mLedgeClass = finder.Class;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	mpMovement = GetCharacterMovement();
	mpCapsule = GetCapsuleComponent();
	mCone = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
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
		// get right direction of wall
		FVector rightDirection = FVector::CrossProduct(mGravityNormal, wallNormal);
		mVelocity = mVelocity.ProjectOnToNormal(rightDirection.GetSafeNormal());

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
	if (IsTouchingCeil())
	{
		//UE_LOG(LogTemp, Log, TEXT("Player is touching ceil"));
		//set up speed to 0
		mVelocity = mVelocity.ProjectOnToNormal(mGravityNormal);
		mpMovement->Velocity = mVelocity;
	}

	FVector inputVector = mpMovement->GetLastInputVector();

	FVector diff = inputVector * mMaxWalkSpeed;
	FVector newLocation = GetActorLocation() + diff * GetWorld()->DeltaTimeSeconds;

	FHitResult hitResult;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	if(!GetWorld()->SweepSingleByChannel(hitResult, newLocation, newLocation, GetActorQuat(), ECollisionChannel::ECC_Visibility,
		mpCapsule->GetCollisionShape(), params))
		SetActorLocation(newLocation);
}

void AMyCharacter::ChangeGravity(FVector iGravity, float iAngularSpeed, FVector iAxis)
{
	FVector normal = iGravity.GetSafeNormal();
	FVector axis;
	float roll = 0;
	if (normal.Equals(mGravityNormal))
		return;
	else if (normal.Equals(-mGravityNormal))
	{
		roll = 180;
		axis = GetActorRightVector();
	}
	else
	{
		roll = FMath::RadiansToDegrees(FMath::Acos(iGravity.ProjectOnTo(mGravityNormal).Size() / iGravity.Size()));
		axis = FVector::CrossProduct(-GetActorUpVector(), normal);
	}

	if (!iAxis.Equals(FVector::ZeroVector))
	{
		axis = iAxis;
	}

	//UE_LOG(LogTemp, Log, TEXT("axis = %f"), );
	SetGravity(iGravity);

	// set timer if it's not set
	if (!GetWorldTimerManager().TimerExists(mGravityHandle))
	{
		mGravityDel.BindUFunction(this, FName("ChangeGravityFunc"), iAngularSpeed, roll, axis);
		GetWorldTimerManager().SetTimer(mGravityHandle, mGravityDel, 0.001f, true, 0);
	}
	
}

bool AMyCharacter::IsOnGround()
{
	FVector start = GetActorLocation() - GetActorUpVector() * mpCapsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
	FVector end = start - GetActorUpVector();

	FHitResult hitResult;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	//DrawDebugSphere(GetWorld(), start, mpCapsule->GetScaledCapsuleRadius(), 50, FColor::Blue);
	//DrawDebugSphere(GetWorld(), end, mpCapsule->GetScaledCapsuleRadius(), 50, FColor::Blue);

	return GetWorld()->SweepSingleByChannel(hitResult, start, end, GetActorQuat(), ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeSphere(mpCapsule->GetScaledCapsuleRadius()), params);

}

bool AMyCharacter::IsTouchingCeil()
{
	FVector start = GetActorLocation() + GetActorUpVector() * mpCapsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
	FVector end = start + GetActorUpVector();

	FHitResult hitResult;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	//DrawDebugSphere(GetWorld(), start, mpCapsule->GetScaledCapsuleRadius(), 50, FColor::Blue);
	//DrawDebugSphere(GetWorld(), end, mpCapsule->GetScaledCapsuleRadius(), 50, FColor::Blue);

	return GetWorld()->SweepSingleByChannel(hitResult, start, end, GetActorQuat(), ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeSphere(mpCapsule->GetScaledCapsuleRadius() - 1), params);
}

AActor* AMyCharacter::GetLedge()
{
	TArray<AActor*> overlappingActors;
	mCone->GetOverlappingActors(overlappingActors, mLedgeClass);

	if (overlappingActors.Num() != 0)
	{
		return overlappingActors[0];
	}

	return nullptr;
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

void AMyCharacter::ChangeGravityFunc(float iAngularSpeed, float iRoll, FVector iRotateAxis)
{
	static float roll = 0;
	if (!bIsChangingGravity)
	{
		// init
		bIsChangingGravity = true;
		mpMovement->SetMovementMode(MOVE_Custom, 1);
		roll = 0;
	}
	else
	{
		// 
		FVector downVector = -GetActorUpVector();
		float angle = FMath::Acos(downVector.ProjectOnTo(mGravityNormal).Size());

		//UE_LOG(LogTemp, Log, TEXT(" iRoll = %f"), iRoll);

		if (roll  < iRoll || angle > 0.01f)
		{
			// update
			float deltaRoll = GetWorldTimerManager().GetTimerRate(mGravityHandle) * iAngularSpeed;
			roll += deltaRoll;

			auto deltaQuat = FQuat(iRotateAxis, FMath::DegreesToRadians(deltaRoll));
		
			//UE_LOG(LogTemp, Log, TEXT(" Roll = %f"), deltaRoll);

			AddActorWorldRotation(deltaQuat);
		}
		else
		{
			// fix rotation
			auto deltaQuat = FQuat(iRotateAxis, angle);
			AddActorWorldRotation(deltaQuat);

			// clear data
			roll = 0;
			bIsChangingGravity = false;
			GetWorldTimerManager().ClearTimer(mGravityHandle);

			// set movement mode
			if (IsOnGround())
			{
				mpMovement->SetMovementMode(MOVE_Custom, 0);
			}
			else
			{
				mpMovement->SetMovementMode(MOVE_Custom, 1);
			}
		}
	}
}

