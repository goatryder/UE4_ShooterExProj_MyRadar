// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "UI/ShooterRadarCollector.h"

#include "Player/ShooterCharacter.h"
#include "Pickups/ShooterPickup.h"
#include "Weapons/ShooterWeapon.h"

FRadarPoint RadarPointPickupBase = { nullptr, AShooterPickup::StaticClass(), true };
FRadarPoint RadarPointEnemyBase = { nullptr,  AShooterCharacter::StaticClass(), true, true, true, true , FVector::ZeroVector, 0.0f, RADAR_ENEMY_DISPLAY_TIME };

UShooterRadarCollector::UShooterRadarCollector()
{
	// Subs delegates
	AShooterCharacter::NotifyShooterCharacterSpawn.AddUObject(this, &UShooterRadarCollector::CharacterSpawnedEvent);
	AShooterCharacter::NotifyShooterCharacterKill.AddUObject(this, &UShooterRadarCollector::CharacterKilledEvent);
	AShooterPickup::NotifyPickupPick.AddUObject(this, &UShooterRadarCollector::PickupPickEvent);
	AShooterPickup::NotifyPickupRespawn.AddUObject(this, &UShooterRadarCollector::PickupRespawnEvent);
	AShooterWeapon::NotifyShooterCharacterWeaponShot.AddUObject(this, &UShooterRadarCollector::CharacterWeaponShotEvent);

	// todo add subs Delegate character damaged (to display hit indicator on radar)
}

void UShooterRadarCollector::BeginDestroy()
{
	Super::BeginDestroy();

	// Unsubs delegates
	if (DelegateHandle_PickupPick.IsValid()) AShooterPickup::NotifyPickupPick.Remove(DelegateHandle_PickupPick);
	if (DelegateHandle_PickupRespawn.IsValid()) AShooterPickup::NotifyPickupRespawn.Remove(DelegateHandle_PickupRespawn);
	if (DelegateHandle_CharacterSpawn.IsValid()) AShooterCharacter::NotifyShooterCharacterSpawn.Remove(DelegateHandle_CharacterSpawn);
	if (DelegateHandle_CharacterKill.IsValid()) AShooterCharacter::NotifyShooterCharacterKill.Remove(DelegateHandle_CharacterKill);
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
	// Todo: fix two times event fire
	// Debug
	if (GEngine)
	{
		FString Msg = FString::Printf(TEXT("[RadarCollector] Character %s Shot from %s"), *Character->GetName(), *Weapon->GetName());
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, Msg);
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
}