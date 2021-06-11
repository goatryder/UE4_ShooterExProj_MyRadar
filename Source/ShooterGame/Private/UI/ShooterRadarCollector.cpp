// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "UI/ShooterRadarCollector.h"

#include "Player/ShooterCharacter.h"
#include "Pickups/ShooterPickup.h"

UShooterRadarCollector::UShooterRadarCollector()
{
	AShooterCharacter::NotifyShooterCharacterSpawn.AddUObject(this, &UShooterRadarCollector::CharacterSpawnedEvent);
	AShooterCharacter::NotifyShooterCharacterKill.AddUObject(this, &UShooterRadarCollector::CharacterKilledEvent);

	AShooterPickup::NotifyPickupPick.AddUObject(this, &UShooterRadarCollector::PickupPickEvent);
	AShooterPickup::NotifyPickupRespawn.AddUObject(this, &UShooterRadarCollector::PickupRespawnEvent);
}

void UShooterRadarCollector::BeginDestroy()
{
	Super::BeginDestroy();

	if (DelegateHandle_PickupPick.IsValid()) 
		AShooterPickup::NotifyPickupPick.Remove(DelegateHandle_PickupPick);
	
	if (DelegateHandle_PickupRespawn.IsValid()) 
		AShooterPickup::NotifyPickupRespawn.Remove(DelegateHandle_PickupRespawn);
	
	if (DelegateHandle_CharacterSpawn.IsValid()) 
		AShooterCharacter::NotifyShooterCharacterSpawn.Remove(DelegateHandle_CharacterSpawn);
	
	if (DelegateHandle_CharacterKill.IsValid()) 
		AShooterCharacter::NotifyShooterCharacterKill.Remove(DelegateHandle_CharacterKill);
}

void UShooterRadarCollector::AddEnemy(AShooterCharacter* Enemy)
{
	if (Enemy == nullptr)
	{
		return;
	}

	if (!Enemies.Contains(Enemy))
	{
		FRadarPoint Point;
		Point.Actor = Enemy;
		Point.ShowTimeMax = 3.0f;
		Point.bCanShow = true;

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
		FRadarPoint Point;
		Point.Actor = Pickup;
		Point.ShowTimeMax = 0.0f;
		Point.bCanShow = true;

		Pickups.Add(Pickup, Point);
	}
	else
	{
		FRadarPoint& Point = Pickups[Pickup];
		Point.bCanShow = true;
	}
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
		Point.bCanShow = false;
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

void UShooterRadarCollector::UpdateRadarTick(float DeltaTime)
{

	for (auto& Elem : Enemies)
	{
		if (Elem.Key.IsValid())
		{
			TWeakObjectPtr<AActor> Actor = Elem.Value.Actor;
			if (Actor.IsValid())
			{
				Elem.Value.LastPos = Actor->GetActorLocation();
				Elem.Value.ShowTime += DeltaTime;
			}
		}
	}

	for (auto& Elem : Pickups)
	{
		if (Elem.Key.IsValid())
		{
			TWeakObjectPtr<AActor> Actor = Elem.Value.Actor;
			if (Actor.IsValid())
			{
				Elem.Value.LastPos = Actor->GetActorLocation();
				Elem.Value.ShowTime += DeltaTime;
			}
		}
	}

}