// Fill out your copyright notice in the Description page of Project Settings.

#include "Utility.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Public/UObject/UObjectIterator.h"


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

