// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ShooterRadarCollector.h"

#include "GameFramework/Actor.h"

#include "Player/ShooterCharacter.h"
#include "Pickups/ShooterPickup.h"
#include "Weapons/ShooterWeapon.h"

FRadarPoint RadarPointPickupBase = { nullptr, AShooterPickup::StaticClass(), true };
FRadarPoint RadarPointEnemyBase = { nullptr,  AShooterCharacter::StaticClass(), true, true, true, true , FVector::ZeroVector, 0.0f, RADAR_ENEMY_DISPLAY_TIME };

UShooterRadarCollector::UShooterRadarCollector()
{
	RadarHitMarkerData = FRadarHitMarkerData();

	if (this != StaticClass()->GetDefaultObject())  // if cdo, do not subs to delegate
	{
		// Subs delegates
		DelegateHandle_CharacterSpawn =               AShooterCharacter::NotifyShooterCharacterSpawn.AddUObject(this, &UShooterRadarCollector::CharacterSpawnedEvent);
		DelegateHandle_CharacterKill =                AShooterCharacter::NotifyShooterCharacterKill.AddUObject(this, &UShooterRadarCollector::CharacterKilledEvent);
		DelegateHandle_PickupPick =                   AShooterPickup::NotifyPickupPick.AddUObject(this, &UShooterRadarCollector::PickupPickEvent);
		DelegateHandle_PickupRespawn =                AShooterPickup::NotifyPickupRespawn.AddUObject(this, &UShooterRadarCollector::PickupRespawnEvent);
		DelegateHandle_CharacterWeaponShot =          AShooterWeapon::NotifyShooterCharacterWeaponShot.AddUObject(this, &UShooterRadarCollector::CharacterWeaponShotEvent);
	}
}

void UShooterRadarCollector::BeginDestroy()
{
	Super::BeginDestroy();

	// Unsubs delegates
	if (DelegateHandle_CharacterSpawn.IsValid())      AShooterCharacter::NotifyShooterCharacterSpawn.Remove(DelegateHandle_CharacterSpawn);
	if (DelegateHandle_CharacterKill.IsValid())       AShooterCharacter::NotifyShooterCharacterKill.Remove(DelegateHandle_CharacterKill);
	if (DelegateHandle_PickupPick.IsValid())          AShooterPickup::NotifyPickupPick.Remove(DelegateHandle_PickupPick);
	if (DelegateHandle_PickupRespawn.IsValid())       AShooterPickup::NotifyPickupRespawn.Remove(DelegateHandle_PickupRespawn);
	if (DelegateHandle_CharacterWeaponShot.IsValid()) AShooterWeapon::NotifyShooterCharacterWeaponShot.Remove(DelegateHandle_CharacterWeaponShot);
}

void UShooterRadarCollector::AddEnemy(AShooterCharacter* Enemy)
{
	if (Enemy == nullptr)
	{
		return;
	}

	if (!Enemies.Contains(Enemy))
	{
		FRadarPoint Point = RadarPointEnemyBase;
		Point.Actor = Enemy;
		Enemies.Add(Enemy, Point);
	}
}

void UShooterRadarCollector::RemoveEnemy(AShooterCharacter* Enemy)
{
	if (Enemy == nullptr)
	{
		return;
	}

	Enemies.Remove(Enemy);
}

void UShooterRadarCollector::ShowEnemy(AShooterCharacter* Enemy)
{
	if (Enemy == nullptr)
	{
		return;
	}

	if (Enemies.Contains(Enemy))
	{
		FRadarPoint& Point = Enemies[Enemy];
		Point.Show(true);
	}
}

void UShooterRadarCollector::AddPickup(AShooterPickup* Pickup)
{
	if (Pickup == nullptr)
	{
		return;
	}

	if (!Pickups.Contains(Pickup))
	{
		FRadarPoint Point = RadarPointPickupBase;
		Point.Actor = Pickup;
		Pickups.Add(Pickup, Point);
	}
	
	FRadarPoint& Point = Pickups[Pickup];
	Point.Show(true);
}

void UShooterRadarCollector::RemovePickup(AShooterPickup* Pickup)
{
	if (Pickup == nullptr)
	{
		return;
	}

	if (Pickups.Contains(Pickup))
	{
		FRadarPoint& Point = Pickups[Pickup];
		Point.Show(false);
	}
}

void UShooterRadarCollector::CharacterSpawnedEvent(AShooterCharacter* Character)
{
	if (Character == nullptr || (Character->IsLocallyControlled() && Character->IsPlayerControlled()))
	{
		return;
	}

	AddEnemy(Character);
}

void UShooterRadarCollector::CharacterKilledEvent(AShooterCharacter* Character)
{
	if (Character == nullptr)
	{
		return;
	}

	RemoveEnemy(Character);
}

void UShooterRadarCollector::PickupPickEvent(AShooterPickup* Pickup)
{
	if (Pickup == nullptr)
	{
		return;
	}

	RemovePickup(Pickup);
}

void UShooterRadarCollector::PickupRespawnEvent(AShooterPickup* Pickup)
{
	if (Pickup == nullptr)
	{
		return;
	}

	AddPickup(Pickup);
}

void UShooterRadarCollector::CharacterWeaponShotEvent(AShooterCharacter* Character, AShooterWeapon* Weapon)
{
	if (Character == nullptr)
	{
		return;
	}

	ShowEnemy(Character);
}

void UShooterRadarCollector::AddHitMarker(FVector HitFromDirection)
{
	RadarHitMarkerData.AddHitDirection(HitFromDirection);
}

void UShooterRadarCollector::TrackedCharacterTakePointDmgEvent(AActor* DamagedActor, float Damage, AController* InstigatedBy, 
	FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, 
	FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	if (DamagedActor && DamagedActor == TrackedTakeDamageCharacter)
	{
		AddHitMarker(ShotFromDirection);

		/*// Debug
		if (GEngine)
		{
			FString HitMarkersDbgText;
			for (FVector& HitMarker : RadarHitMarkerData.HitFromDirections)
			{
				HitMarkersDbgText.Append("  ");
				HitMarkersDbgText.Append(HitMarker.ToString());
			}
			FString Msg = FString::Printf(TEXT("[HitMarkers] %s"), *HitMarkersDbgText);
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, Msg);
		}*/
	}
}

void UShooterRadarCollector::UpdateRadarTick(float DeltaTime)
{
	// update enemies radar points
	for (auto& Elem : Enemies)
	{
		if (Elem.Key.IsValid())
		{
			Elem.Value.Update(DeltaTime);
		}
	}

	// update pickups radar points
	for (auto& Elem : Pickups)
	{
		if (Elem.Key.IsValid())
		{
			Elem.Value.Update(DeltaTime);
		}
	}

	// update radar hit markers
	RadarHitMarkerData.Update(DeltaTime);
}

void UShooterRadarCollector::SetTrackedTakeDamageCharacter(AShooterCharacter* ShooterCharacter)
{
	if (ShooterCharacter)
	{
		ShooterCharacter->OnTakePointDamage.AddDynamic(this, &UShooterRadarCollector::TrackedCharacterTakePointDmgEvent);
		TrackedTakeDamageCharacter = ShooterCharacter;
	}
}