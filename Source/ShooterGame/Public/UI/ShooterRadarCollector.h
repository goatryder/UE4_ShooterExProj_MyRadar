// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ShooterRadarCollector.generated.h"

/**
 * This Class is collect's and provide Radar Info for ShooterHUD
 */
UCLASS()
class SHOOTERGAME_API UShooterRadarCollector : public UObject
{
	GENERATED_BODY()

	UShooterRadarCollector();

	virtual void BeginDestroy() override final;
	
};
