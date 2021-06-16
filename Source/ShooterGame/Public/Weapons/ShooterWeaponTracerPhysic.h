// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeaponTracerPhysic.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class AShooterWeapon;

UCLASS()
class SHOOTERGAME_API AShooterWeaponTracerPhysic : public AActor
{
	GENERATED_BODY()

public:
	AShooterWeaponTracerPhysic();

	static AShooterWeaponTracerPhysic* SpawnFromWeapon(AShooterWeapon* Weapon, TSubclassOf<AShooterWeaponTracerPhysic> TracerClass, const FHitResult& HitResult);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
		USphereComponent* SphereComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
		UProjectileMovementComponent* ProjectileComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
		UParticleSystemComponent* ParticleSystemComp;

	/** If projectile hit something that overlap instead of block, then destroy */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tracer")
		bool bShouldDestroyOnOverlap;

	/** Time to destroy after bounce (Overrides LifeSpan), if < 0 then destroy immediately */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tracer")
		float DestroyOnBounceTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tracer")
		bool bShouldDestroyOnSecondBounce;

	bool bIsBouncedFirstTime;

	FVector TargetDestination;

	/** will ignore projectile movement ShouldBounce
	  * max allowed bounce angle to hit normal
	  * zero mean no clamp bounce angle,
	  * in world of tanks default is 75 degrees
	  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tracer", meta = (ClampMin = 0.0f, ClampMax = 90.0f))
		float BounceAngleToNormalMin;

	/* Actual Value That Will Be Check if Bounce or not, Calculated from BounceAngleToNormalMin */
	float BounceAngleCos;

	UFUNCTION()
		void OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Adjust Initial Launch Angle relatively Gravity force to hit in right place */
	void DbgTestAdjustInitialVelocityToHitTarget();

	/**  */
	void ComputeProjectileInitialVelocitiesToHitTarget(FVector ImpactPoint, TArray<FVector>& OutVelocityArray);
};
