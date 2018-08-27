// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MultiCityGenerator.generated.h"

UCLASS()
class MACHANISMS_API AMultiCityGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMultiCityGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	// Actor to be rebuild
	UPROPERTY(EditAnywhere, Category = "Environment", meta = (DisplayName = "CityActor"))
		AActor* mCityActor;

	TArray<AActor*> mCityActorList;
	
	
};
