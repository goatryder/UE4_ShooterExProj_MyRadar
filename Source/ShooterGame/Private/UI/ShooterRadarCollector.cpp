// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ShooterRadarCollector.h"

#include "GameFramework/Actor.h"

#include "Player/ShooterCharacter.h"
#include "Pickups/ShooterPickup.h"
#include "Weapons/ShooterWeapon.h"
#include "Pickups/ShooterPickup_Health.h"
#include "Pickups/ShooterPickup_Ammo.h"
#include "Weapons/ShooterWeapon_Projectile.h"

bool FRadarPoint::Update(float DeltaTime)
{
	if (IsRadarPointValid())
	{
		if (!bPosUpdateIsBlocked)
		{
			LastPos = Actor->GetActorLocation();  // update last location

			if (bUpdatePosOnShowOnly)  // handle bUpdatePosOnShowOnly param
			{
				bPosUpdateIsBlocked = true;  // when bUpdatePosOnShowOnly=true pos will update only after Show() func call
			}
		}

		// update show time
		ShowTime += DeltaTime;
		if (ShowTimeMax > 0.0f && ShowTime >= ShowTimeMax)
		{
			Show(false);
			ShowTime = 0.0f;
		}

		return true;
	}
	else
	{
		return false;
	}
}

FRadarPoint RadarPointPickupBase = { nullptr, true };
FRadarPoint RadarPointEnemyBase = { nullptr, true, true, true, true , FVector::ZeroVector, 0.0f, RADAR_ENEMY_DISPLAY_TIME };

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

	const int32 EnemyRadarPointIndex = GetActorRadarPointArrIndex(Enemies, Enemy);
	if (EnemyRadarPointIndex == -1)
	{
		FRadarPoint Point = RadarPointEnemyBase;
		Point.Actor = Enemy;
		Enemies.Add(Point);
	}
}

void UShooterRadarCollector::RemoveEnemy(AShooterCharacter* Enemy)
{
	if (Enemy == nullptr)
	{
		return;
	}
	
	const int32 EnemyRadarPointIndex = GetActorRadarPointArrIndex(Enemies, Enemy);
	if (EnemyRadarPointIndex != -1)
	{
		FRadarPoint& Point = Enemies[EnemyRadarPointIndex];
		Point.Actor = nullptr;  // Pending FRadarPoint element in array to be removed in next Update() call
	}
}

void UShooterRadarCollector::ShowEnemy(AShooterCharacter* Enemy)
{
	if (Enemy == nullptr)
	{
		return;
	}

	const int32 EnemyRadarPointIndex = GetActorRadarPointArrIndex(Enemies, Enemy);
	if (EnemyRadarPointIndex != -1)
	{
		FRadarPoint& Point = Enemies[EnemyRadarPointIndex];
		Point.Show(true);
	}
}

TArray<FRadarPoint>* UShooterRadarCollector::GetProperPickupArr(AShooterPickup* Pickup)
{
	if (Cast<AShooterPickup_Health>(Pickup))
	{
		return &HealthPickups;
	}
	else if (AShooterPickup_Ammo* PickupAmmo = Cast<AShooterPickup_Ammo>(Pickup))
	{
		if (PickupAmmo->IsForWeapon(AShooterWeapon_Projectile::StaticClass()))
		{
			return &GrenadesPickups;
		}
		return &AmmoPickups;
	}

	return nullptr;
}

void UShooterRadarCollector::AddPickup(AShooterPickup* Pickup)
{
	if (Pickup == nullptr)
	{
		return;
	}

	if (TArray<FRadarPoint>* SelectedPickupArr = GetProperPickupArr(Pickup))
	{
		TArray<FRadarPoint>& Pickups = *SelectedPickupArr;
		
		const int32 PickupRadarPointIndex = GetActorRadarPointArrIndex(Pickups, Pickup);
		if (PickupRadarPointIndex == -1)
		{
			FRadarPoint PointToAdd = RadarPointPickupBase;
			PointToAdd.Actor = Pickup;

			const uint32 Index = Pickups.Num();
			Pickups.Add(PointToAdd);

			FRadarPoint& Point = Pickups[Index];
			Point.Show(true);
		}
		else
		{
			FRadarPoint& Point = Pickups[PickupRadarPointIndex];
			Point.Show(true);
		}
	}
}

void UShooterRadarCollector::RemovePickup(AShooterPickup* Pickup)
{
	if (Pickup == nullptr)
	{
		return;
	}

	if (TArray<FRadarPoint>* SelectedPickupArr = GetProperPickupArr(Pickup))
	{
		TArray<FRadarPoint>& Pickups = *SelectedPickupArr;

		const int32 PickupRadarPointIndex = GetActorRadarPointArrIndex(Pickups, Pickup);
		if (PickupRadarPointIndex != -1)
		{
			FRadarPoint& Point = Pickups[PickupRadarPointIndex];
			Point.Show(false);
		}
	}
}

const int32 UShooterRadarCollector::GetActorRadarPointArrIndex(TArray<FRadarPoint>& RadarPointArr, AActor* Actor)
{
	if (Actor == nullptr)
	{
		return -1;
	}

	for (int32 i = 0; i < RadarPointArr.Num(); i++)
	{
		if (RadarPointArr[i].IsRadarPointValid() && RadarPointArr[i].Actor == Actor)
		{
			return i;
		}
	}
	
	return -1;
}

void UShooterRadarCollector::UpdateRadarPointArr(TArray<FRadarPoint>& RadarPointArr, float DeltaTime)
{
	for (int32 Index = RadarPointArr.Num() - 1; Index >= 0; --Index)
	{
		FRadarPoint& RadarPoint = RadarPointArr[Index];
		if (!RadarPoint.Update(DeltaTime))
		{
			RadarPointArr.RemoveAtSwap(Index, 1, false);
		}
	}
	
	RadarPointArr.Shrink();
}

void UShooterRadarCollector::CharacterSpawnedEvent(AShooterCharacter* Character)
{
	if (Character == nullptr 
		|| Character == TrackedTakeDamageCharacter)
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
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("TrackedCharacterTakePointDamageEvent"));

	if (DamagedActor && DamagedActor == TrackedTakeDamageCharacter)
	{
		AddHitMarker(ShotFromDirection);
	}
}

void UShooterRadarCollector::UpdateRadarTick(float DeltaTime)
{
	// update pickups radar points
	UpdateRadarPointArr(HealthPickups, DeltaTime);
	UpdateRadarPointArr(AmmoPickups, DeltaTime);
	UpdateRadarPointArr(GrenadesPickups, DeltaTime);

	UpdateRadarPointArr(Enemies, DeltaTime);  // update enemies radar points
	
	RadarHitMarkerData.Update(DeltaTime);     // update radar hit markers
}

void UShooterRadarCollector::SetTrackedTakeDamageCharacter(AShooterCharacter* ShooterCharacter)
{
	if (ShooterCharacter)
	{
		ShooterCharacter->OnTakePointDamage.AddDynamic(this, &UShooterRadarCollector::TrackedCharacterTakePointDmgEvent);
		TrackedTakeDamageCharacter = ShooterCharacter;
	}
}