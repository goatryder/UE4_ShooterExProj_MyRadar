// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "ARadarCollector.h"

// Sets default values
AARadarCollector::AARadarCollector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AARadarCollector::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AARadarCollector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

