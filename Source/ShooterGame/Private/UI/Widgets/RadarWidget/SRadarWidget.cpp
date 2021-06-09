// Fill out your copyright notice in the Description page of Project Settings.


#include "SRadarWidget.h"
#include "ShooterHUD.h"
#include "Player/ShooterCharacter.h"
#include "Pickups/ShooterPickup.h"

#define LOCTEXT_NAMESPACE "RadarWidget"

void SRadarWidget::Construct(const FArguments& InArgs)
{
	OwningHUD = InArgs._OwningHUD;

	AShooterCharacter::NotifyShooterCharacterSpawn.AddRaw(this, &SRadarWidget::CharacterSpawnedEvent);
	AShooterCharacter::NotifyShooterCharacterKill.AddRaw(this, &SRadarWidget::CharacterKilledEvent);

	AShooterPickup::NotifyPickupPick.AddRaw(this, &SRadarWidget::PickupPickEvent);
	AShooterPickup::NotifyPickupRespawn.AddRaw(this, &SRadarWidget::PickupRespawnEvent);

	//ChildSlot
	//	[
	//		// black bg
	//		//OverlayStaticRef
	//		SNew(SOverlay)
	//
	//		+ SOverlay::Slot()
	//		.HAlign(HAlign_Fill)
	//		.VAlign(VAlign_Fill)
	//		[
	//			SNew(SImage)
	//			.ColorAndOpacity(FColor(0.0f, 0.0f, 0.0f, 0.25f))
	//		]
	//	];

	//auto Auto = ChildSlot.GetChildAt(0);
	//OverlayDynamicRef(ChildSlot.GetChildAt(1));
	//SWidget* Widget = ChildSlot.GetChildAt(0);
	// auto WidgetRef
	//OverlayDynamicRef = ChildSlot.GetChildAt(1).Get();
}

FReply SRadarWidget::OnQuitClicked() const
{
	if (OwningHUD.IsValid())
	{
		OwningHUD->HideRadar();
	}

	return FReply::Handled();
}

void SRadarWidget::AddEnemy(AShooterCharacter* Enemy)
{
	if (!Enemies.Contains(Enemy))
	{
		FRadarPoint Point;
		Point.Actor = Enemy;
		Point.ShowTimeMax = 3.0f;
		Point.bCanShow = true;

		Enemies.Add(Enemy, Point);
	}
}

void SRadarWidget::RemoveEnemy(AShooterCharacter* Enemy)
{
	Enemies.Remove(Enemy);
}

void SRadarWidget::AddPickup(AShooterPickup* Pickup)
{
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

void SRadarWidget::RemovePickup(AShooterPickup* Pickup)
{
	if (Pickups.Contains(Pickup))
	{
		FRadarPoint& Point = Pickups[Pickup];
		Point.bCanShow = false;
	}
}

void SRadarWidget::CharacterSpawnedEvent(AShooterCharacter* Character)
{
	if (Character == nullptr || Character->IsLocallyControlled())
	{
		return;
	}

	AddEnemy(Character);
}

void SRadarWidget::CharacterKilledEvent(AShooterCharacter* Character)
{
	if (Character == nullptr)
	{
		return;
	}
	
	RemoveEnemy(Character);
}

void SRadarWidget::PickupPickEvent(AShooterPickup* Pickup)
{
	if (Pickup == nullptr)
	{
		return;
	}

	RemovePickup(Pickup);
}

void SRadarWidget::PickupRespawnEvent(AShooterPickup* Pickup)
{
	if (Pickup == nullptr)
	{
		return;
	}

	AddPickup(Pickup);
}

void SRadarWidget::UpdateRadarTick(float DeltaTime)
{

	for (auto& Elem : Enemies)
	{
		TWeakObjectPtr<AActor> Actor = Elem.Value.Actor;
		if (Actor.IsValid())
		{
			Elem.Value.LastPos = Actor->GetActorLocation();
			Elem.Value.ShowTime += DeltaTime;
		}
	}

	for (auto& Elem : Pickups)
	{
		TWeakObjectPtr<AActor> Actor = Elem.Value.Actor;
		if (Actor.IsValid())
		{
			Elem.Value.LastPos = Actor->GetActorLocation();
			Elem.Value.ShowTime += DeltaTime;
		}
	}

}

#undef LOCTEXT_NAMESPACE