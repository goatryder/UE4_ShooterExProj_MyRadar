// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ShooterRadarCollector.generated.h"

struct FRadarPoint
{
	bool bCanShow = false;
	TWeakObjectPtr<class AActor> Actor = nullptr;
	FVector LastPos = FVector::ZeroVector;
	float ShowTime = 0.0f;

	/** If 0.0, permanent if state not changes */
	float ShowTimeMax = 0.0f;
};

/**
 * This Class is collect's and provide Radar Info for ShooterHUD
 */
UCLASS()
class SHOOTERGAME_API UShooterRadarCollector : public UObject
{
	GENERATED_BODY()

public:
	UShooterRadarCollector();

	virtual void BeginDestroy() override final;

	/** The HUD that created this widget */
	TWeakObjectPtr<class AShooterHUD> OwningHUD;

	/** Enemies array */
	TMap<TWeakObjectPtr<class AShooterCharacter>, FRadarPoint> Enemies;

	/** Pickups array */
	TMap<TWeakObjectPtr<class AShooterPickup>, FRadarPoint> Pickups;

protected:
	void AddEnemy(class AShooterCharacter* Enemy);
	void RemoveEnemy(class AShooterCharacter* Enemy);

	void AddPickup(class AShooterPickup* Pickup);
	void RemovePickup(class AShooterPickup* Pickup);

	UFUNCTION()
		void CharacterSpawnedEvent(class AShooterCharacter* Character);
	
	UFUNCTION()
		void CharacterKilledEvent(class AShooterCharacter* Character);

	UFUNCTION()
		void PickupPickEvent(class AShooterPickup* Pickup);
	
	UFUNCTION()
		void PickupRespawnEvent(class AShooterPickup* Pickup);

public:
	void UpdateRadarTick(float DeltaTime);

private:
	FDelegateHandle DelegateHandle_CharacterSpawn;
	FDelegateHandle DelegateHandle_CharacterKill;
	FDelegateHandle DelegateHandle_PickupPick;
	FDelegateHandle DelegateHandle_PickupRespawn;
	
};
