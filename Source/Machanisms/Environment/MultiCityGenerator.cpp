// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiCityGenerator.h"
#include "Components/SceneComponent.h"


// Sets default values
AMultiCityGenerator::AMultiCityGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = root;
}

// Called when the game starts or when spawned
void AMultiCityGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMultiCityGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

