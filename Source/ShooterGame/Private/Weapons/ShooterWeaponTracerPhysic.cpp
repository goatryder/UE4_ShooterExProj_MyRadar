// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterWeaponTracerPhysic.h"

#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

#include "Weapons/ShooterWeapon.h"
#include "ShooterGame.h"

AShooterWeaponTracerPhysic::AShooterWeaponTracerPhysic()
{
	PrimaryActorTick.bCanEverTick = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SetCanBeDamaged(false);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;

	SphereComp->SetGenerateOverlapEvents(true);
	SphereComp->SetCollisionProfileName(TRACER_PROFILE_NAME);
	SphereComp->InitSphereRadius(5.0f);

	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AShooterWeaponTracerPhysic::OnOverlapBegin);
	SphereComp->OnComponentHit.AddDynamic(this, &AShooterWeaponTracerPhysic::OnHit);

	ParticleSystemComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComp"));
	ParticleSystemComp->SetupAttachment(SphereComp);

	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));

	// projectile movement defaults
	ProjectileComp->ProjectileGravityScale = 1.0f;
	ProjectileComp->InitialSpeed = 30000.0f;
	ProjectileComp->MaxSpeed = 30000.0f;
	ProjectileComp->bRotationFollowsVelocity = true;
	ProjectileComp->bShouldBounce = true;
	ProjectileComp->bBounceAngleAffectsFriction = true;
	ProjectileComp->Bounciness = 0.2f;
	ProjectileComp->Friction = 1.0f;
	ProjectileComp->BounceVelocityStopSimulatingThreshold = 1000.0f;

	ProjectileComp->bAutoActivate = false;
	ProjectileComp->OnProjectileBounce.AddDynamic(this, &AShooterWeaponTracerPhysic::OnProjectileBounce);

	// tracer defaults
	InitialLifeSpan = 0.5f;
	bShouldDestroyOnOverlap = true;
	DestroyOnBounceTime = 0.2f;
	bShouldDestroyOnSecondBounce = true;
	BounceAngleToNormalMin = 75.0f;
	BounceAngleCos = cosf(FMath::DegreesToRadians(BounceAngleToNormalMin));  // initialize reflect cos angle
}

AShooterWeaponTracerPhysic* AShooterWeaponTracerPhysic::SpawnFromWeapon(AShooterWeapon* Weapon, TSubclassOf<AShooterWeaponTracerPhysic> TracerClass, const FHitResult& HitResult)
{
	if (Weapon == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't spawn AShooterWeaponTracerPhysic, weapon %s instance is NULL)"), *AShooterWeapon::StaticClass()->GetName());
		return nullptr;
	}

	FVector MuzzleLoc = Weapon->GetMuzzleLocation();
	FVector ImpactPoint = HitResult.ImpactPoint;

	FVector ShotDirection = ImpactPoint - MuzzleLoc;
	ShotDirection.Normalize();

	// Spawn Deferred
	const FTransform Transform(ShotDirection.Rotation(), MuzzleLoc, FVector::OneVector);

	AShooterWeaponTracerPhysic* TracerSimulated = Weapon->GetWorld()->SpawnActorDeferred<AShooterWeaponTracerPhysic>(
		TracerClass, Transform, Weapon);

	// Do on spawn stuff here
	TracerSimulated->TargetDestination = ImpactPoint;
	TracerSimulated->SphereComp->MoveIgnoreActors.Add(Weapon);
	TracerSimulated->SphereComp->MoveIgnoreActors.Add(Weapon->GetOwner());
	//

	TracerSimulated->FinishSpawning(Transform);
	//	

	return TracerSimulated;
}

void AShooterWeaponTracerPhysic::BeginPlay()
{
	Super::BeginPlay();

	//DbgTestAdjustInitialVelocityToHitTarget();

	TArray<FVector> Velocities;
	ComputeProjectileInitialVelocitiesToHitTarget(TargetDestination, Velocities);

	// debug
	//if (GEngine)
	//	for (auto& Vel : Velocities)
	//		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, *Vel.ToString(), false);

	ProjectileComp->Velocity = (Velocities[0]);

	ProjectileComp->Activate();
}

void AShooterWeaponTracerPhysic::OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	SetLifeSpan(DestroyOnBounceTime);

	if (bShouldDestroyOnSecondBounce && bIsBouncedFirstTime)
	{
		Destroy();
		return;
	}

	bIsBouncedFirstTime = true;
}

void AShooterWeaponTracerPhysic::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner() || OtherActor == GetOwner()->GetOwner())  // ignore weapon and weapon owner
	{
		return;
	}

	// debug
	//DrawDebugSphere(GetWorld(), GetActorLocation(), 10.0f, 12, FColor::Green, false, 3.0f);

	if (bShouldDestroyOnOverlap)
	{
		Destroy();
	}
}

void AShooterWeaponTracerPhysic::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// debug
	//DrawDebugSphere(GetWorld(), GetActorLocation(), 10.0f, 12, FColor::Red, false, 3.0f);

	//if (GEngine)
	//{
	//	FString Msg = FString::Printf(TEXT("On Hit %s with %s %s"), *this->GetName(), *OtherActor->GetName(), *OtherComp->GetName());
	//	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, Msg);
	//}

	if (OtherActor && (OtherActor == GetOwner() || OtherActor == GetOwner()->GetOwner()))  // ignore weapon and weapon owner
	{
		return;
	}

	// handle if tracer should reflect
	if (BounceAngleToNormalMin > 0.0f)
	{
		FVector VelocityNormalized = SphereComp->GetComponentVelocity().GetSafeNormal();
		float NormalAndVelocityDot = FVector::DotProduct(-VelocityNormalized, Hit.ImpactNormal);

		// debug
		// incoming
		//DrawDebugLine(GetWorld(), Hit.Location, VelocityNormalized * -100.0f + Hit.Location, FColor::Red, true);
		// normal
		//DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactNormal * 100.0f + Hit.ImpactPoint, FColor::Green, true);

		// debug angle
		//float HitAngle = FMath::RadiansToDegrees(acosf(NormalAndVelocityDot));
		//UE_LOG(LogTemp, Warning, TEXT("Dot %f, Angle %f, SHOULD BOUNCE WHEN: Dot %f, Angle %f, %s"),
		//	NormalAndVelocityDot, HitAngle, BounceAngleCos, BounceAngleToNormalMin, 
		//	NormalAndVelocityDot <= BounceAngleCos ? *FString("BOUNCE") : *FString("DESTROY"));

		if (NormalAndVelocityDot <= BounceAngleCos)
		{
			ProjectileComp->bShouldBounce = true;
		}
		else
		{
			Destroy();
		}
	}
}

void AShooterWeaponTracerPhysic::ComputeProjectileInitialVelocitiesToHitTarget(FVector ImpactPoint, TArray<FVector>& OutVelocityArray)
{
	float Gravity = ProjectileComp->GetGravityZ();
	float VelMagnitude = ProjectileComp->InitialSpeed;
	FVector P_Delta = GetActorLocation() - ImpactPoint;

	if (Gravity == 0.0f)  // need to shoot straight line
	{
		FVector Velocity_StraightLine = -P_Delta.GetSafeNormal() * VelMagnitude;
		OutVelocityArray = { Velocity_StraightLine };
		return;
	}

	float VelMagnitude_Square = VelMagnitude * VelMagnitude;
	float Gravity_Square = Gravity * Gravity;

	float B = Gravity * P_Delta.Z - VelMagnitude_Square;
	float Discriminant = B * B - Gravity_Square * (P_Delta.Z * P_Delta.Z + P_Delta.X * P_Delta.X + P_Delta.Y * P_Delta.Y);

	if (Discriminant < 0.0f) // no solution, can't reach target, then add 45degree to reach max distance
	{

		FVector Velocity_MaxDist(P_Delta.X, P_Delta.Y, 0.0f);
		Velocity_MaxDist.Normalize();
		Velocity_MaxDist *= -VelMagnitude * 0.707107f;
		Velocity_MaxDist.Z = VelMagnitude * 0.707107f;

		OutVelocityArray = { Velocity_MaxDist };
	}
	else
	{
		float Discriminant_Sqrt = sqrtf(Discriminant);

		float TimeStraight = sqrtf((-B - Discriminant_Sqrt) / (0.5f * Gravity_Square));

		if (TimeStraight == 0.0f) // need to shot straight line
		{
			FVector Velocity_StraightLine = -P_Delta.GetSafeNormal() * VelMagnitude;
			OutVelocityArray = { Velocity_StraightLine };
			return;
		}

		float TimeOverhead = sqrtf((-B + Discriminant_Sqrt) / (0.5f * Gravity_Square));

		// debug
		//FString msg = FString::Printf(TEXT("Gravity=%f, P_Delta.Z=%f, B=%f, sqrt(D)=%f, T1=%f T2=%f"), Gravity, P_Delta.Z, B, Discriminant_Sqrt, TimeStraight, TimeOverhead);
		//if (GEngine)
		//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, *msg);
		

		FVector Velocity_Straight(-P_Delta.X / TimeStraight, -P_Delta.Y / TimeStraight, -P_Delta.Z / TimeStraight - Gravity * TimeStraight * 0.5f);
		FVector Velocity_Overhead(-P_Delta.X / TimeOverhead, -P_Delta.Y / TimeOverhead, -P_Delta.Z / TimeOverhead - Gravity * TimeOverhead * 0.5f);

		OutVelocityArray = { Velocity_Straight, Velocity_Overhead };
	}
}

void AShooterWeaponTracerPhysic::DbgTestAdjustInitialVelocityToHitTarget()
{

	// [TEST DATA] initial data
	float SimTime = 1.0f; // setup time
	FVector InitLocation = GetActorLocation();
	float VelMagnitude = ProjectileComp->InitialSpeed;
	FVector InitVelocity = SphereComp->GetForwardVector() * VelMagnitude;
	float Gravity = ProjectileComp->GetGravityZ();
	float Radius = SphereComp->GetScaledSphereRadius();
	float DrawDebugTime = 15.0f;
	//

	// [TEST CHECK] use engine simulation to check if correct formula
	FPredictProjectilePathParams PredictParams;
	PredictParams.StartLocation = InitLocation;
	PredictParams.LaunchVelocity = InitVelocity;
	PredictParams.bTraceWithCollision = true;
	PredictParams.ProjectileRadius = Radius;
	PredictParams.MaxSimTime = SimTime;
	PredictParams.TraceChannel = COLLISION_TRACER;
	PredictParams.SimFrequency = 10.0f;
	PredictParams.OverrideGravityZ = Gravity;
	PredictParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PredictParams.DrawDebugTime = DrawDebugTime;

	FPredictProjectilePathResult PredictResult;

	UGameplayStatics::PredictProjectilePath(GetWorld(), PredictParams, PredictResult);
	//

	// [Projectile Movement Formulas]:
	// compute velocity:
	// v = v0 + a*t

	// compute movement delta
	// Velocity Verlet integration (http://en.wikipedia.org/wiki/Verlet_integration#Velocity_Verlet)
	// The addition of p0 is done outside this method, we are just computing the delta.
	// p = p0 + v0*t + 1/2*a*t^2

	// We use ComputeVelocity() here to infer the acceleration, to make it easier to apply custom velocities.
	// p = p0 + v0*t + 1/2*((v1-v0)/t)*t^2
	// p = p0 + v0*t + 1/2*((v1-v0))*t

	// [My Calculations]
	//
	// p = p0 +v0*t + 0.5*((v0 + a*t - v0)*t; p = p0 + v0*t + 0.5*a*t*t; p = p0 + t*(v0 + 0.5*a*t)
	// First (Worked just fine)
	// FVector OneSecVelocity = InitVelocity + FVector(0.0f, 0.0f, Gravity) * SimTime;
	// FVector OneSecLocation = InitLocation + InitVelocity * SimTime + (OneSecVelocity - InitVelocity) * (0.5f * SimTime);


	// [TEST] Optimized Location Find When Velocity and Time Is known
	FVector OneSecLocation = InitLocation + (InitVelocity + FVector(0.0f, 0.0f, Gravity * SimTime * 0.5f)) * SimTime;

	DrawDebugSphere(GetWorld(), OneSecLocation, Radius * 1.5f, 12, FColor::Red, false, DrawDebugTime);
	//

	// [TEST] Optimized Time Find When Velocity and Location Is known
	// float a = 0.5f * Gravity;
	float b = InitVelocity.Z - InitVelocity.X;

	FVector P_Delta = InitLocation - OneSecLocation;
	// float c = P_Delta.Z - P_Delta.X;
	float Descr_Sqrt = sqrtf(b * b - 2.0f * Gravity * (P_Delta.Z - P_Delta.X));
	float ProjectileTime1 = (-b + Descr_Sqrt) / Gravity;
	float ProjectileTime2 = (-b - Descr_Sqrt) / Gravity;

	DrawDebugString(GetWorld(), OneSecLocation, *FString::Printf(TEXT("Time1: %f Time2: %f"), ProjectileTime1, ProjectileTime2), (AActor*)0, FColor::Red, DrawDebugTime, true);
	//

	// [My Calculations] To Find Velocity (X,Y,Z) When Velocity Magnitude and Projectile Last Location known:
	// Quadratic equation in vector form:
	// 1/2*a*t^2 + v0*t + (p0-p) = 0 also v0^2 = v0.x^2 + v0.y^2 + v0.z^2

	// Kramer method:
	// {
	//   (1) 0.5*a.x*t^2 + v0.x*t + (p0-p).x = 0,		t-?  v0.x-? v0.y-? v0.z-?
	//   (2) 0.5*a.y*t^2 + v0.y*t + (p0-p).y = 0,		a.x == 0 and a.y == 0 (gravity has z only)
	//   (3) 0.5*a.z*t^2 + v0.z*t + (p0-p).z = 0,
	//   (4) v0.x^2 + v0.y^2 + v0.z^2 = v0^2
	// }

	// Ax^4 + Bx^2 + C = 0; D=B^2-4*A*C; x^2 = (-B +- sqrt(D))/(2*A); has four solutions
	// 1/4*a.z^2*t^4 + (a.z*(p0-p).z-v0^2)*t^2 + (p0-p).z^2+(p0-p).y^2+(p0-p).x^2 = 0
	// t^2 = ( -(a.z*(p0-p).z-v0^2) +- sqrt((a.z*(p0-p).z-v0^2)^2 - a.z^2*((p0-p).z^2+(p0-p).y^2+(p0-p).x^2) / 0.5*a.z^2
	// t = +-sqrt(t^2)
	// we have four t roots, 2 negatives, smaller positive - straight trajectory, bigger positive - overhead trajectory
	// v0.x = -(p0-p).x/t;  v0.y = -(p0-p).y/t;  v0.z = -(p0-p).z/t - 1/2*a.z*t;

	// [TEST] Optimized Time And Velocity Find (X,Y,Z) When Velocity Magnitude and Location known:

	float VelMagnitude_Square = VelMagnitude * VelMagnitude;
	float B = Gravity * P_Delta.Z - VelMagnitude_Square;
	float Gravity_Square = Gravity * Gravity;
	float Discriminant_Sqrt = sqrtf(B * B - Gravity_Square * (P_Delta.Z * P_Delta.Z + P_Delta.X * P_Delta.X + P_Delta.Y * P_Delta.Y));
	float A_Mult2 = 0.5f * Gravity_Square;

	float Time1 = sqrtf((-B + Discriminant_Sqrt) / A_Mult2);
	float Time2 = sqrtf((-B - Discriminant_Sqrt) / A_Mult2);

	float VelocityX1 = -P_Delta.X / Time1;
	float VelocityX2 = -P_Delta.X / Time2;

	float VelocityY1 = -P_Delta.Y / Time1;
	float VelocityY2 = -P_Delta.Y / Time2;

	float VelocityZ1 = -P_Delta.Z / Time1 - Gravity * Time1 * 0.5f;
	float VelocityZ2 = -P_Delta.Z / Time2 - Gravity * Time2 * 0.5f;

	float DBG_VelMagnitude1 = sqrtf(VelocityX1 * VelocityX1 + VelocityZ1 * VelocityZ1 + VelocityY1 * VelocityY1);
	float DBG_VelMagnitude2 = sqrtf(VelocityX2 * VelocityX2 + VelocityZ2 * VelocityZ2 + VelocityY2 * VelocityY2);

	FString LastTestData = FString::Printf(
		TEXT(" t1=%f vx1=%f vy1=%f vz1=%f |v1|=%f] \n t2=%f vx2=%f vy2=%f vz2=%f |v2|=%f "),
		Time1, VelocityX1, VelocityY1, VelocityZ1, DBG_VelMagnitude1,
		Time2, VelocityX2, VelocityY2, VelocityZ2, DBG_VelMagnitude2
	);

	FString CheckData = FString::Printf(
		TEXT("t=%f vx=%f, vy=%f, vz=%f |v|=%f"),
		SimTime, InitVelocity.X, InitVelocity.Y, InitVelocity.Z, VelMagnitude
	);

	DrawDebugString(GetWorld(), OneSecLocation + FVector(0.0f, 0.0f, -100.0f), *CheckData, (AActor*)0, FColor::Yellow, DrawDebugTime, true);
	DrawDebugString(GetWorld(), OneSecLocation + FVector(0.0f, 0.0f, -200.0f), *LastTestData, (AActor*)0, FColor::Cyan, DrawDebugTime, true);
	//
}
