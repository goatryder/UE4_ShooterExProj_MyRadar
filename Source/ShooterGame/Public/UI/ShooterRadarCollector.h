// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ShooterRadarCollector.generated.h"

class AActor;
class AShooterCharacter;
class AShooterWeapon;
class AShooterPickup;
class AController;
class UDamageType;
class UPrimitiveComponent;

#define MAX_PLAYER_NAME_LENGTH 16
#define RADAR_ENEMY_DISPLAY_TIME 1.0f
#define RADAR_HIT_MARKER_DISPLAY_TIME 1.0f
#define RADAR_HIT_MARKER_MAX 5

/*
 * HUD radar actor point to display information container
 */
USTRUCT()
struct FRadarPoint
{
	GENERATED_BODY()

	/** Actor to display on radar */
	UPROPERTY()
	AActor* Actor = nullptr;

	/** Should HUD Radar draw this radar point on next draw call */
	bool bCanShow = false;
	/** Should HUD Radar draw this radar point on radar border if it's out of radar range radius */
	bool bCanShowIfOutRadarBorder = false;  // logic for this should be handled externally

	/** Should displayed actor position update on ShowTime reset or every draw call */
	bool bUpdatePosOnShowOnly = false;
	/** Flag value to handle logic based on bUpdatePosOnTimerOnly */
	bool bPosUpdateIsBlocked = false;

	/** Displayed actor last location */
	FVector LastPos = FVector::ZeroVector;
	/** Time elapsed when radar point is been draw */
	float ShowTime = 0.0f;

	/** Amount of time to draw radar point. If 0.0 it's will be draw permanently */
	float ShowTimeMax = 0.0f;

	/** Check if RadarPoint is valid */
	bool IsRadarPointValid()
	{
		return IsValid(Actor);
	}

	/*
	 * Update RadarPoint Timer and handle logic on it 
	 *
	 * @param	DeltaTime	Time since last update
	 * @return	if RadarPoint is valid and update success, false is RadarPoint is invalid and update fails
	 */
	bool Update(float DeltaTime);

	/** Handling Show/Hide Logic. Result is updated bCanShow param */
	void Show(bool bShowOnRadar)
	{
		if (bShowOnRadar && bUpdatePosOnShowOnly)  // handle bUpdatePosOnShowOnly param
		{
			bPosUpdateIsBlocked = false;  // update pos gate open
		}

		ShowTime = 0.0f; // reset time count
		bCanShow = bShowOnRadar;
	}
};

/*
 * Struct to hold and provide information on radar about recent player hits recieved
 */
struct FRadarHitMarkerData
{
	/** current HitFromDirections/HitMarkerShowTimes index */
	uint32 Index = 0;
	
	/** array to store last N hit direction vectors */
	FVector HitFromDirections[RADAR_HIT_MARKER_MAX] = {FVector::ZeroVector};
	/** array to store show time for last N hit direction vectors in HitFromDirections array */
	float HitMarkerShowTimes[RADAR_HIT_MARKER_MAX] = {-1.0f};

	/*
	 *  Get valid hit directions we can draw (valid when time to show is not -1.0f) 
	 *
	 * @param	OutHitFromDirections	array to add relevant hit directions to
	 */
	void GetRelevantHitFromDirections(TArray<FVector> &OutHitFromDirections)
	{
		for (uint32 i = 0; i < RADAR_HIT_MARKER_MAX; i++)
		{
			if (HitMarkerShowTimes[i] == -1.0f || HitFromDirections[i] == FVector::ZeroVector)
			{
				continue;
			}
			OutHitFromDirections.Add(HitFromDirections[i]);
		}
	}

	/*
	 * Change hit direction vector for the very last updated hit direction array position 
	 *
	 * @param HitFromDirection	hit direction to add
	 */
	void AddHitDirection(FVector HitFromDirection)
	{
		HitFromDirections[Index] = HitFromDirection;
		HitMarkerShowTimes[Index] = 0.0f;

		Index = (Index + 1) % RADAR_HIT_MARKER_MAX;
	}

	/* 
	 * Calculate new HitMarkerShowTime for each hit direction in HitFromDirections array
	 *
	 * @param DeltaTime time since last RadarHitMarkerData update
	 */
	void Update(float DeltaTime)
	{
		for (float& HitMarkerShowTime : HitMarkerShowTimes)
		{
			if (HitMarkerShowTime == -1.0f)
			{
				continue;
			}

			HitMarkerShowTime += DeltaTime;

			if (HitMarkerShowTime >= RADAR_HIT_MARKER_DISPLAY_TIME)
			{
				HitMarkerShowTime = -1.0f;
			}
		}
	}
};

/**
 * This Class is collect's and provide Radar Info for ShooterHUD
 * Kinda MVC, FRadarPoint, FRadarHitMarkerData - model, UShooterRadarCollector - controller, AShooterHUD - view
 */
UCLASS()
class SHOOTERGAME_API UShooterRadarCollector : public UObject
{
	GENERATED_BODY()

public:
	UShooterRadarCollector();

	virtual void BeginDestroy() override final;

	/** Enemies array */
	UPROPERTY()
	TArray<FRadarPoint> Enemies;

	/** Health pickups array */
	UPROPERTY()
	TArray<FRadarPoint> HealthPickups;

	/** Ammo pickups array */
	UPROPERTY()
	TArray<FRadarPoint> AmmoPickups;

	/** Grenade launcher ammo pickups array */
	UPROPERTY()
	TArray<FRadarPoint> GrenadesPickups;

	/** Radar hit marker data */
	FRadarHitMarkerData RadarHitMarkerData;

protected:

	/** Add enemy radar point to Enemies map */
	void AddEnemy(AShooterCharacter* Enemy);
	/** Remove enemy radar point from Enemies map */
	void RemoveEnemy(AShooterCharacter* Enemy);
	/** Call Show(true) for enemy radar point if found in Enemies map */
	void ShowEnemy(AShooterCharacter* Enemy);

	/** Get proper pickup array depends on Pickup actual final class */
	TArray<FRadarPoint>* GetProperPickupArr(AShooterPickup* Pickup);
	/** Show pickup radar point, add pickup to Pickups map if not found in map*/
	void AddPickup(AShooterPickup* Pickup);
	/** Hide pickup radar point*/
	void RemovePickup(AShooterPickup* Pickup);

	/*
	* Traverse RadarPointArr and try to get array index of RadarPoint with selected Actor value.
	*
	* @param RadarPointArr	RadarPoint array where to find if selected actor is in RadarPoint struct.
	* @param Actor			Actor ptr to find in RadarPoint struct
	*
	* @returns				-1 if actor not found in any RadarPoint for selected array, 
							or if actor is nullptr, else return RadarPoint index in array
	*/
	const int32 GetActorRadarPointArrIndex(TArray<FRadarPoint> &RadarPointArr, AActor* Actor);

	/*
	* Call Update() for RadarPoint in array if RadarPoint.Actor != nullptr, else remove RadarPoint from array
	*
	* @param RadarPointArr	RadarPoint array where to call Update for each element
	* @param DeltaTime		Time since last RadarPointArr update;
	*/
	void UpdateRadarPointArr(TArray<FRadarPoint>& RadarPointArr, float DeltaTime);

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

	/** Character to detect hits from to provide radar hit marker info */
	AShooterCharacter* TrackedTakeDamageCharacter;
	
	/**  */
	void AddHitMarker(FVector HitFromDirection);

	/**  */
	UFUNCTION()
		void TrackedCharacterTakePointDmgEvent(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser);

public:
	/** Calls Update() for radar points in Enemies, Pickups map radar points */
	void UpdateRadarTick(float DeltaTime);
	
	/**  */
	void SetTrackedTakeDamageCharacter(AShooterCharacter* ShooterCharacter);

private:
	FDelegateHandle DelegateHandle_CharacterSpawn;
	FDelegateHandle DelegateHandle_CharacterKill;
	FDelegateHandle DelegateHandle_PickupPick;
	FDelegateHandle DelegateHandle_PickupRespawn;
	FDelegateHandle DelegateHandle_CharacterWeaponShot;
};
