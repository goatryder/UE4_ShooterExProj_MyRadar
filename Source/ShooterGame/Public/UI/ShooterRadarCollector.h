// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ShooterRadarCollector.generated.h"

class AActor;
class AShooterCharacter;
class AShooterWeapon;
class AShooterPickup;

/*
 * HUD radar actor point to display information container
 */
struct FRadarPoint
{
	/** Actor to display on radar */
	TWeakObjectPtr<AActor> Actor = nullptr;
	/** Displayed actor class */
	TSubclassOf<AActor> ActorClass;

	/** Should HUD Radar draw this radar point on next draw call */
	bool bCanShow = false;
	/** Should HUD Radar draw this radar point on radar border if it's out of radar range radius */
	bool bCanShowIfOutRadarBorder = false;  // logic for this should be handled externally

	/** Should displayed actor position update on ShowTime reset or every draw call */
	bool bUpdatePosOnShowOnly = false;
	/** Utility value to handle logic based on bUpdatePosOnTimerOnly */
	bool bCanUpdatePos = true;

	/** Displayed actor last location */
	FVector LastPos = FVector::ZeroVector;
	/** Time elapsed when radar point is been draw */
	float ShowTime = 0.0f;

	/** Amount of time to draw radar point. If 0.0 it's will be draw permanently */
	float ShowTimeMax = 0.0f;

	/** Update RadarPoint Timer and handle logic on it */
	void Update(float DeltaTime)
	{
		if (!Actor.IsValid())
		{
			return;
		}

		if (bCanUpdatePos)
		{
			// update last location
			LastPos = Actor->GetActorLocation();

			if (bUpdatePosOnShowOnly)  // handle bUpdatePosOnShowOnly param
			{
				bCanUpdatePos = false;  // when bUpdatePosOnShowOnly=true pos will update only after Show() func call
			}
		}

		// update show time
		ShowTime += DeltaTime;
		if (ShowTimeMax > 0.0f && ShowTime >= ShowTimeMax)
		{
			//Show(false);  disabled for dbg
			ShowTime = 0.0f;
		}
	}

	/** Handling Show/Hide Logic. Result is updated bCanShow param */
	void Show(bool bShowOnRadar)
	{
		if (!Actor.IsValid())
		{
			return;
		}

		if (bShowOnRadar && bUpdatePosOnShowOnly)  // handle bUpdatePosOnShowOnly param
		{
			bCanUpdatePos = true;  // update pos gate open
		}

		ShowTime = 0.0f; // reset time count
		bCanShow = bShowOnRadar;
	}
};

/**
 * This Class is collect's and provide Radar Info for ShooterHUD
 * Kinda MVC, FRadarPoint - model, UShooterRadarCollector - controller, AShooterHUD - view
 */
UCLASS()
class SHOOTERGAME_API UShooterRadarCollector : public UObject
{
	GENERATED_BODY()

public:
	UShooterRadarCollector();

	virtual void BeginDestroy() override final;

	/** The HUD that created this widget */
	TWeakObjectPtr<AShooterHUD> OwningHUD;

	/** Enemies array */
	TMap<TWeakObjectPtr<AActor>, FRadarPoint> Enemies;

	/** Pickups array */
	TMap<TWeakObjectPtr<AActor>, FRadarPoint> Pickups;

protected:

	/** Add enemy radar point to Enemies map */
	void AddEnemy(AShooterCharacter* Enemy);
	/** Remove enemy radar point from Enemies map */
	void RemoveEnemy(AShooterCharacter* Enemy);
	/** Call Show(true) for enemy radar point if found in Enemies map */
	void ShowEnemy(AShooterCharacter* Enemy);

	/** Show pickup radar point, add pickup to Pickups map if not found in map*/
	void AddPickup(AShooterPickup* Pickup);
	/** Hide pickup radar point*/
	void RemovePickup(AShooterPickup* Pickup);

	/** Add character to enemies map */
	UFUNCTION()
		void CharacterSpawnedEvent(AShooterCharacter* Character);
	
	/** Calls AddEnemy() */
	UFUNCTION()
		void CharacterKilledEvent(AShooterCharacter* Character);

	/** Calss RemoveEnemy() */
	UFUNCTION()
		void PickupPickEvent(AShooterPickup* Pickup);
	
	/** Calls AddPickup() */
	UFUNCTION()
		void PickupRespawnEvent(AShooterPickup* Pickup);

	/** Calls ShowEnemy() */
	UFUNCTION()
		void CharacterWeaponShotEvent(AShooterCharacter* Character, AShooterWeapon* Weapon);

public:
	/** Calls Update() for radar points in Enemies, Pickups map radar points */
	void UpdateRadarTick(float DeltaTime);

private:
	FDelegateHandle DelegateHandle_CharacterSpawn;
	FDelegateHandle DelegateHandle_CharacterKill;
	FDelegateHandle DelegateHandle_PickupPick;
	FDelegateHandle DelegateHandle_PickupRespawn;
	FDelegateHandle DelegateHandle_CharacterWeaponShot;
	
};
