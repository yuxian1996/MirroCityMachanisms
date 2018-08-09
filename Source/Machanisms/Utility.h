// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Public/CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Public/TimerManager.h"


/**
 * 
 */

class AActor;

class MACHANISMS_API Utility
{
public:
	Utility();
	~Utility();

	static FORCEINLINE bool MTraceShapeByChannel(FHitResult& oHitResult, const FVector& iStart, const FVector& iEnd, 
		const FCollisionShape & iShape, const ECollisionChannel& iTraceChannel, const FCollisionQueryParams& iQueryParams, 
		const FCollisionResponseParams& iResponseParams);

	static FORCEINLINE void MoveBetween2Locations(AActor* iActor, FVector iStart, FVector iEnd, float iSpeed);

	UFUNCTION()
	static FORCEINLINE void MoveBetween2LocationsFunc(FTimerHandle iHandle, AActor* iActor, FVector iStart, FVector iEnd, float iSpeed);

};
