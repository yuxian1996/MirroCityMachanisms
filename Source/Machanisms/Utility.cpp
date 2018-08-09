// Fill out your copyright notice in the Description page of Project Settings.

#include "Utility.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Public/UObject/UObjectIterator.h"
#include "Public/TimerManager.h"


Utility::Utility()
{
}

Utility::~Utility()
{
}

bool Utility::MTraceShapeByChannel(FHitResult& oHitResult, const FVector& iStart, const FVector& iEnd,
	const FCollisionShape & iShape, const ECollisionChannel& iTraceChannel, const FCollisionQueryParams& iQueryParams,
	const FCollisionResponseParams& iResponseParams)
{
	// re-initialize hit info
	oHitResult = FHitResult(ForceInit);

	// get player controller
	TObjectIterator<APlayerController> playerController;
	if (!playerController) return false;

	// get world
	if (UWorld* world = playerController->GetWorld())
	{
		return world->SweepSingleByChannel(oHitResult, iStart, iEnd, FQuat(), iTraceChannel,
			iShape, iQueryParams, iResponseParams);
	}

	return false;
}

void Utility::MoveBetween2LocationsFunc(FTimerHandle iHandle, AActor* iActor, FVector iStart, FVector iEnd, float iSpeed)
{
	static float length = 0;
	FVector direction = (iEnd - iStart).GetSafeNormal();
	float distance = FVector::Dist(iEnd, iStart);

	if (length < distance)
	{
		float diff = iSpeed * iActor->GetWorldTimerManager().GetTimerRate(iHandle);
		length += diff;
		iActor->SetActorLocation(iActor->GetActorLocation() + direction * diff);
	}
	else
	{
		iActor->SetActorLocation(iActor->GetActorLocation() + direction * (distance - length));
		length = 0;
		iActor->GetWorldTimerManager().ClearTimer(iHandle);
	}

}

void Utility::MoveBetween2Locations(AActor* iActor, FVector iStart, FVector iEnd, float iSpeed)
{
	FTimerHandle handle;
	//FTimerDelegate del = FTimerDelegate::CreateStatic(MoveBetween2LocationsFunc, handle, iActor, iSpeed, iEnd, iSpeed);
	//TBaseDelegate<void, FTimerHandle, AActor*, FVector, FVector, float> del;
	//TFunction<void(AActor*, FVector, FVector, float)>(MoveBetween2LocationsFunc)
	//del.BindStatic(Utility::MoveBetween2LocationsFunc(), handle, iActor, iSpeed, iEnd, iSpeed);
	//del.BindStatic();
	//iActor->GetWorldTimerManager().SetTimer(handle, 0.017f, true, 0);

}


