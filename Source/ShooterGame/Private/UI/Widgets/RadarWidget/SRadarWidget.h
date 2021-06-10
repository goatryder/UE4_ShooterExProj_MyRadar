// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"

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
 *
 */
class SRadarWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRadarWidget) {}

	SLATE_ARGUMENT(TWeakObjectPtr<class AShooterHUD>, OwningHUD)

	SLATE_END_ARGS()

	/** every widget needs a construction function */
	void Construct(const FArguments& InArgs);

	FReply OnQuitClicked() const;

	/** The HUD that created this widget */
	TWeakObjectPtr<class AShooterHUD> OwningHUD;

	/** Enemies array */
	TMap<TWeakObjectPtr<class AShooterCharacter>, FRadarPoint> Enemies;

	/** Pickups array */
	TMap<TWeakObjectPtr<class AShooterPickup>, FRadarPoint> Pickups;

protected:

	/** Can't rotate this overlay */
	//TSharedRef<class SOverlay, ESPMode::NotThreadSafe> OverlayStaticRef;
	
	/** Can rotate this overlay*/
	//TSharedRef<class SOverlay, ESPMode::NotThreadSafe> OverlayDynamicRef;

	void AddEnemy(class AShooterCharacter* Enemy);
	void RemoveEnemy(class AShooterCharacter* Enemy);

	void AddPickup(class AShooterPickup* Pickup);
	void RemovePickup(class AShooterPickup* Pickup);

	void CharacterSpawnedEvent(class AShooterCharacter* Character);
	void CharacterKilledEvent(class AShooterCharacter* Character);

	void PickupPickEvent(class AShooterPickup* Pickup);
	void PickupRespawnEvent(class AShooterPickup* Pickup);

public:

	void UpdateRadarTick(float DeltaTime);

private:

	FDelegateHandle DelegateHandle_CharacterSpawn;
	FDelegateHandle DelegateHandle_CharacterKill;
};
