// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"
#include "Public/TimerManager.h"
#include "Public/DrawDebugHelpers.h"
#include "Public/UObject/ConstructorHelpers.h"
#include "Utility.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mMaxWalkSpeed = 800.0f;
	mMaxSlope = 45.0f;
	mMaxStepHeight = 70.0f;
	mGravity = FVector(0, 0, -980.0f);
	mAcceleration = 800;
	mMaxLedgeDistance = 100;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	mpMovement = GetCharacterMovement();
	mpCapsule = GetCapsuleComponent();
	mGravityNormal = mGravity.GetSafeNormal();
	mCamera = Cast<UCameraComponent>(GetComponentByClass(UCameraComponent::StaticClass()));
	
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
	if (GetWorldTimerManager().TimerExists(mStairHandle))
	{
		return;
	}

	FVector normal;
	FVector direction = mpMovement->GetLastInputVector();

	Accelerate();

	if (!TryWalk(normal))
	{
		if (!bIsBesideWall && normal != FVector::ZeroVector)
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

		UE_LOG(LogTemp, Log, TEXT("Wall normal : %f, %f, %f"), wallNormal.X , wallNormal.Y, wallNormal.Z);
		UE_LOG(LogTemp, Log, TEXT("gravity : %f, %f, %f"), mGravityNormal.X, mGravityNormal.Y, mGravityNormal.Z);

		// change direction
		// get right direction of wall
		FVector rightDirection = FVector::CrossProduct(mGravityNormal, wallNormal);
		mVelocity = mVelocity.ProjectOnTo(rightDirection);

		UE_LOG(LogTemp, Log, TEXT("new direction : %f, %f, %f"), mVelocity.X, mVelocity.Y, mVelocity.Z);

		if (!TryWalk(normal))
		{
			UE_LOG(LogTemp, Log, TEXT("Player is blocked"));
			UE_LOG(LogTemp, Log, TEXT("normal : %f, %f, %f"), normal.X, normal.Y, normal.Z);

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
		FVector tempVelocity = FVector::VectorPlaneProject(mpMovement->Velocity, mGravityNormal);
		if (FVector::DotProduct(mGravity, mpMovement->Velocity - tempVelocity) < 0)
		{
			mVelocity = mpMovement->Velocity = tempVelocity - (mpMovement->Velocity - tempVelocity) / 5;
		}
	}

	if (bIsSteppingDown)
		return;

	FVector inputVector = mpMovement->GetLastInputVector();

	FVector diff = inputVector * mMaxWalkSpeed;
	FVector newLocation = GetActorLocation() + diff * GetWorld()->DeltaTimeSeconds;

	FHitResult hitResult;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	if (!GetWorld()->SweepSingleByChannel(hitResult, GetActorLocation(), newLocation, GetActorQuat(), ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeCapsule(mpCapsule->GetScaledCapsuleRadius() + 2, mpCapsule->GetScaledCapsuleHalfHeight()), params))
	{
		SetActorLocation(newLocation);
	}
	else
	{
		mVelocity = mpMovement->Velocity.ProjectOnToNormal(mGravityNormal);
		mpMovement->Velocity = mVelocity;
	}
}

FVector AMyCharacter::DetectLedge()
{
	FVector eyeStart = mCamera->GetComponentLocation();
	FVector eyeEnd = eyeStart + GetActorForwardVector() * mMaxLedgeDistance;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	FHitResult hitResult;

	if (UWorld* world = GetWorld())
	{
		if (world->LineTraceSingleByChannel(hitResult, eyeStart, eyeEnd, ECollisionChannel::ECC_Visibility, params))
		{
			FVector bottom = hitResult.ImpactPoint + GetActorForwardVector();
			FVector top = bottom + GetActorUpVector() * mMaxLedgeHeight;

			if (world->LineTraceSingleByChannel(hitResult, bottom, top, ECollisionChannel::ECC_Visibility))
			{
				return hitResult.ImpactPoint;
			}
		}
	}
	
	return FVector::ZeroVector;
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
		axis = -GetActorRightVector();
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
	FVector end = start - GetActorUpVector() * 2;

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
	FVector end = start + GetActorUpVector() * 2;

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
				mpCapsule->GetCollisionShape(), queryParams, responseParams))
			{
				// walk on a plane
				if (newHit.Distance <= 1 )
				{
					MoveTo(newLocation + mGravityNormal * newHit.Distance);
					UE_LOG(LogTemp, Log, TEXT("Walk to %f, %f, %f"), newLocation.X, newLocation.Y, newLocation.Z);
				}
				else
				{
					// step down
					FVector end = newLocation + mGravityNormal * newHit.Distance + originalVelocity.GetSafeNormal() * 5;
					FVector direction = (end - GetActorLocation()).GetSafeNormal();
					float dis = FVector::Dist(GetActorLocation(), end);
					mStairDel.BindUFunction(this, "MoveOnStairs", direction, dis, mMaxWalkSpeed);
					bIsSteppingDown = true;
					GetWorldTimerManager().SetTimer(mStairHandle, mStairDel, 0.01f, true);
					mpMovement->Velocity = originalVelocity;
					UE_LOG(LogTemp, Log, TEXT("Stop down %f"), newHit.Distance);
				}
			}
			else
			{
				// fall
				MoveTo(newLocation);
				UE_LOG(LogTemp, Log, TEXT("Fall to %f, %f, %f"), newLocation.X, newLocation.Y, newLocation.Z);
			}

			return true;
		}
		else
		{
			FVector planeVector = FVector::VectorPlaneProject(hitResult.ImpactNormal, -mGravityNormal);
			float angle = FMath::RadiansToDegrees(FMath::Abs(FMath::Asin(planeVector.Size())));	
			UE_LOG(LogTemp, Log, TEXT("Angle = %f"), angle);

			if (angle < mMaxSlope)
			{
				// walk on slope
				FHitResult hitResult1;
				FVector top = newLocation - mGravityNormal * mMaxStepHeight;
				world->SweepSingleByChannel(hitResult1, top, newLocation, GetActorRotation().Quaternion(), ECollisionChannel::ECC_Visibility,
					FCollisionShape::MakeCapsule(mpCapsule->GetScaledCapsuleRadius(), mpCapsule->GetScaledCapsuleHalfHeight()), queryParams, responseParams);

				MoveTo(top + mGravityNormal * hitResult1.Distance);
				UE_LOG(LogTemp, Log, TEXT("Walk on slope"));
				return true;
			}
			else
			{
				FHitResult hitResult2;
				FVector top = newLocation - mGravityNormal * mMaxStepHeight;
				world->SweepSingleByChannel(hitResult2, top, newLocation, GetActorRotation().Quaternion(), ECollisionChannel::ECC_Visibility,
					FCollisionShape::MakeCapsule(mpCapsule->GetScaledCapsuleRadius(), mpCapsule->GetScaledCapsuleHalfHeight()), queryParams, responseParams);

				// calculate angle and height of next stairs
				FVector nextPlaneVector = FVector::VectorPlaneProject(hitResult2.ImpactNormal, -mGravityNormal);
				float nextAngle = FMath::RadiansToDegrees(FMath::Abs(FMath::Asin(nextPlaneVector.Size())));
				if (nextAngle < mMaxSlope && hitResult2.Distance >= 0.001f)
				{
					// walk on stairs
					FVector end = top + mGravityNormal * hitResult2.Distance + originalVelocity.GetSafeNormal() * 20;
					FVector direction = (end - GetActorLocation()).GetSafeNormal();
					float dis = FVector::Dist(GetActorLocation(), end);
					mStairDel.BindUFunction(this, "MoveOnStairs", direction, dis, mMaxWalkSpeed);
					bIsSteppingDown = true;
					GetWorldTimerManager().SetTimer(mStairHandle, mStairDel, 0.01f, true);
					mpMovement->Velocity = originalVelocity;
					UE_LOG(LogTemp, Log, TEXT("Walk on stairs")); 
					return true;
				}
				else
				{
					oHitNormal = hitResult.ImpactNormal;
					UE_LOG(LogTemp, Log, TEXT("Move beside wall")); 
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

void AMyCharacter::MoveOnStairs(FVector iDirection, float iDistance, float iSpeed)
{
	static float length = 0;
	if (length < iDistance)
	{
		float diff = iSpeed * GetWorldTimerManager().GetTimerRate(mStairHandle);
		SetActorLocation(GetActorLocation() + diff * iDirection);
		length += diff;
	}
	else
	{
		SetActorLocation(GetActorLocation() + (iDistance - length) * iDirection);
		length = 0;
		GetWorldTimerManager().ClearTimer(mStairHandle);
		bIsSteppingDown = false;
	}

}

