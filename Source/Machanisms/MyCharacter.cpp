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

bool AMyCharacter::MoveTo(FVector iLocation, FVector& oNormal)
{
	FVector oldLocation = GetActorLocation();

	//SetActorLocation(iLocation);
	if (auto world = GetWorld())
	{
		mpMovement->Velocity = mVelocity;
		//DrawDebugSphere(world, iLocation, 2, 10, FColor::Red, true);

		FHitResult result;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);

		if (world->SweepSingleByChannel(result, iLocation, iLocation, GetActorQuat(), ECC_Visibility,
			mpCapsule->GetCollisionShape(-5), params))
		{
			oNormal = result.ImpactNormal;
			return false;
		}
		else
		{
			SetActorLocation(iLocation);
			return true;
		}
	}

	return false;

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

		//UE_LOG(LogTemp, Log, TEXT("Wall normal : %f, %f, %f"), wallNormal.X , wallNormal.Y, wallNormal.Z);
		//UE_LOG(LogTemp, Log, TEXT("gravity : %f, %f, %f"), mGravityNormal.X, mGravityNormal.Y, mGravityNormal.Z);

		// change direction
		// get right direction of wall
		FVector rightDirection = FVector::CrossProduct(mGravityNormal, wallNormal);
		mVelocity = mVelocity.ProjectOnTo(rightDirection);

		//UE_LOG(LogTemp, Log, TEXT("new direction : %f, %f, %f"), mVelocity.X, mVelocity.Y, mVelocity.Z);

		if (!TryWalk(normal))
		{
			//UE_LOG(LogTemp, Log, TEXT("Player is blocked"));
			//UE_LOG(LogTemp, Log, TEXT("normal : %f, %f, %f"), normal.X, normal.Y, normal.Z);

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
		FVector top = newLocation - mGravityNormal * mMaxStepHeight;
		FVector bottom = newLocation + mGravityNormal * mMaxStepHeight;
		FVector sphereTop = top + mGravityNormal * mpCapsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
		FVector sphereNewLocation = newLocation + mGravityNormal * mpCapsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
		FVector sphereBottom = bottom + mGravityNormal * mpCapsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
		FVector sphereLocation = location + mGravityNormal * mpCapsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();

		// check if next location is blocked
		FCollisionQueryParams queryParams;
		// ignore self
		queryParams.AddIgnoredActor(this);
		FCollisionResponseParams responseParams;
		
		FHitResult hitResult(ForceInit);

		// detect the plane in front of player
		if (!world->SweepSingleByChannel(hitResult, sphereLocation - mGravityNormal, sphereNewLocation - mGravityNormal, FQuat(), ECollisionChannel::ECC_Visibility,
			FCollisionShape::MakeSphere(mpCapsule->GetScaledCapsuleRadius()), queryParams))
		{
			FHitResult hitResult1(ForceInit);

			if (world->SweepSingleByChannel(hitResult1, sphereNewLocation, sphereBottom, FQuat(), ECollisionChannel::ECC_Visibility,
				FCollisionShape::MakeSphere(mpCapsule->GetScaledCapsuleRadius()), queryParams))
			{
				FVector nextLocation = newLocation + mGravityNormal * hitResult1.Distance;
				float maxDis = mVelocity.Size() * world->DeltaTimeSeconds;
				//UE_LOG(LogTemp, Log, TEXT("distance = %f"), hitResult1.Distance);

				// walk on a plane
				if (hitResult1.Distance <= maxDis)
				{
					bIsSteppingDown = false;
					return MoveTo(nextLocation, oHitNormal);
				}
				else
				{
					// step down
					bIsSteppingDown = true;
					//UE_LOG(LogTemp, Log, TEXT("Step down %f"), maxDis);
					return MoveTo(newLocation + mGravityNormal * maxDis, oHitNormal);;
				}
			}
			else
			{
				// fall
				//UE_LOG(LogTemp, Log, TEXT("Fall to %f, %f, %f"), newLocation.X, newLocation.Y, newLocation.Z);
				bIsSteppingDown = false;
				return MoveTo(newLocation, oHitNormal);
			}
		}
		else
		{
			FVector planeVector = FVector::VectorPlaneProject(hitResult.ImpactNormal, -mGravityNormal);
			float angle = FMath::RadiansToDegrees(FMath::Abs(FMath::Asin(planeVector.Size())));	
			//UE_LOG(LogTemp, Log, TEXT("Angle = %f"), angle);

			// get wall normal
			oHitNormal = hitResult.ImpactNormal;

			if (angle < mMaxSlope)
			{
				// walk on slope
				FHitResult hitResult1;

				world->SweepSingleByChannel(hitResult1, sphereTop, sphereNewLocation, GetActorQuat(), ECollisionChannel::ECC_Visibility,
					FCollisionShape::MakeSphere(mpCapsule->GetScaledCapsuleRadius()), queryParams, responseParams);

				FVector nextLocation = top + mGravityNormal * hitResult1.Distance;
				bIsSteppingDown = false;
				FVector normal;
				return MoveTo(top + mGravityNormal * hitResult1.Distance, normal);
			}
			else
			{
				FHitResult hitResult1;

				// calculate height of next stairs
				world->SweepSingleByChannel(hitResult1, sphereTop, sphereNewLocation, GetActorQuat(), ECollisionChannel::ECC_Visibility,
					FCollisionShape::MakeSphere(mpCapsule->GetScaledCapsuleRadius()), queryParams);

				if (hitResult1.Distance >= 0.001f)
				{
					// walk on stairs
					float maxDis = mVelocity.Size() * world->DeltaTimeSeconds;
					FVector normal;

					if (hitResult1.Distance < maxDis)
					{
						bIsSteppingDown = false;
						return MoveTo(newLocation - mGravityNormal * hitResult1.Distance, normal);
					}
					else
					{
						bIsSteppingDown = true;
						return MoveTo(location - mGravityNormal * maxDis, normal);
					}
					//UE_LOG(LogTemp, Log, TEXT("Walk on stairs")); 
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

int AMyCharacter::FindStairs()
{
	if (UWorld* world = GetWorld())
	{
		FVector deltaMovement = mVelocity * world->GetDeltaSeconds();
		FVector location = GetActorLocation();
		FVector newLocation = location + deltaMovement;
		FVector direction = mVelocity.GetSafeNormal();
		// not accurate
		FVector nextStepLocation = location + direction * (mpCapsule->GetScaledCapsuleRadius() + mMaxStepHeight);
		//FVector top = nextStepLocation - mGravityNormal * mMaxStepHeight;
		//FVector bottom = nextStepLocation + mGravityNormal * mMaxStepHeight;

		FHitResult result;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		FCollisionShape shape = FCollisionShape::MakeCapsule(mpCapsule->GetScaledCapsuleRadius(), mpCapsule->GetScaledCapsuleHalfHeight() - 1);
		FCollisionShape box = FCollisionShape::MakeBox(FVector(mpCapsule->GetScaledCapsuleRadius(), mpCapsule->GetScaledCapsuleRadius(),
			mpCapsule->GetScaledCapsuleHalfHeight() - 1));
		
		if (world->SweepSingleByChannel(result, location, nextStepLocation, GetActorQuat(), ECC_Visibility, box, params))
		{
			FVector planeVector = FVector::VectorPlaneProject(result.ImpactNormal, -mGravityNormal);
			float angle = FMath::RadiansToDegrees(FMath::Abs(FMath::Asin(planeVector.Size())));
			UE_LOG(LogTemp, Log, TEXT("normal = %f, %f, %f"), result.ImpactNormal.X, result.ImpactNormal.Y, result.ImpactNormal.Z);
			DrawDebugLine(world, result.ImpactPoint, result.ImpactPoint + result.ImpactNormal * 100, FColor::Red, true);

			
			if (angle > mMaxSlope)
			{
				// up stairs
				FHitResult result2;
				FVector bottom = FVector::VectorPlaneProject(result.ImpactPoint, mGravityNormal) + location.ProjectOnToNormal(mGravityNormal);
				FVector top = bottom - mGravityNormal * mMaxStepHeight;
				world->SweepSingleByChannel(result2, top, bottom, GetActorRotation().Quaternion(), ECC_Visibility, shape, params);
				if (result2.Distance >= 0.001f)
				{
					UE_LOG(LogTemp, Log, TEXT("Up stairs"));
					return 1;
				}
			}
		}

		
		//if (world->SweepSingleByChannel(result, nextStepLocation, bottom, GetActorQuat(), ECC_Visibility, shape, params))
		//{
		//	FVector planeVector = FVector::VectorPlaneProject(result.ImpactNormal, -mGravityNormal);
		//	float angle = FMath::RadiansToDegrees(FMath::Abs(FMath::Asin(planeVector.Size())));

		//	//if(angle )
		//	// down stairs
		//	UE_LOG(LogTemp, Log, TEXT("Down stairs"));
		//	return -1;
		//}

		UE_LOG(LogTemp, Log, TEXT("No stairs"));
		return 0;
	}

	return 0;
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

