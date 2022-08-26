// Fill out your copyright notice in the Description page of Project Settings.


#include "TraceProjectile.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogTraceProjectile, Log, All);

// Sets default values
ATraceProjectile::ATraceProjectile()
	: ClipAmmo(30)
	  , BulletType(EBulletType::ProjectileAndGravity)
	  , UpdateFrequency(10)
	  , bDrawDebug(true)
	  , Range(1200.0f)
	  , Velocity(600.0f)
	  , Gravity(9.8f)
{
	ElapsedTimeSinceLastUpdate = 0.0f;
	UpdateInterval = 0.1f;
	ProjectileLifespan = 1.0f;
	PrimaryActorTick.bCanEverTick = true;
}

void ATraceProjectile::FireOnce(FVector Location, FRotator Rotation, int32 ProjectileID)
{
	if (ProjectileID > ProjectileInstances.Num() || ProjectileID < 1)
	{
		UE_LOG(LogTraceProjectile, Warning, TEXT("[TraceProjectile] FireOnce: ProjectileID Error!"));
		return;
	}
	ProjectileInstances[ProjectileID - 1].Activate(Location, Rotation);
}

void ATraceProjectile::BeginPlay()
{
	Super::BeginPlay();

	ClipAmmo = FMath::Clamp(ClipAmmo, 1, 999);
	ProjectileInstances.SetNum(ClipAmmo);
	ProjectileInstances.Shrink();

	UpdateInterval = 1.0f / static_cast<float>(UpdateFrequency);
	ProjectileLifespan = Range / Velocity;

	OnProjectileHit.AddDynamic(this, &ATraceProjectile::ProjectileHitEvent);
}

void ATraceProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTimeSinceLastUpdate += DeltaTime;
	// Only loop once in theory
	while (ElapsedTimeSinceLastUpdate >= UpdateInterval)
	{
		ElapsedTimeSinceLastUpdate -= UpdateInterval;

		for (auto& Projectile : ProjectileInstances)
		{
			if (Projectile.bIsActivate)
			{
				UpdateProjectileOneStep(Projectile, GetAppropriateTimeStep(Projectile, UpdateInterval));
				CheckProjectileResult(Projectile);
			}
		}
	}
}

void ATraceProjectile::ProjectileHitEvent_Implementation(const FHitResult HitResult)
{
}

void ATraceProjectile::UpdateProjectileWithTime(FProjectileInstanceInfo& Projectile, float ElapsedTime)
{
	while (ElapsedTime > 0.0f)
	{
		UpdateProjectileOneStep(Projectile, GetAppropriateTimeStep(Projectile, ElapsedTime));
		ElapsedTime -= UpdateInterval;
	}
}

float ATraceProjectile::GetAppropriateTimeStep(const FProjectileInstanceInfo& Projectile, float ElapsedTime) const
{
	return FMath::Clamp(FMath::Min(ProjectileLifespan - Projectile.ElapsedTime, ElapsedTime), 0.0f, UpdateInterval);
}

void ATraceProjectile::UpdateProjectileOneStep(FProjectileInstanceInfo& Projectile, float TimeStep)
{
	const FVector PreLocationWithoutGravity = Projectile.LocationWithoutGravity;
	const FVector PreLocationWithGravity = Projectile.LocationWithoutGravity - FVector(
		0.0f, 0.0f, Projectile.GravityOffset);

	Projectile.ElapsedTime += TimeStep;
	Projectile.LocationWithoutGravity += Projectile.Rotation.Vector() * 100.0f * Velocity * TimeStep;
	Projectile.GravityOffset = 0.5f * 100.0f * Gravity * FMath::Square(Projectile.ElapsedTime);

	TArray<FHitResult> HitResults;
	if (BulletType == EBulletType::Projectile)
	{
		TraceAndDrawDebug(HitResults, PreLocationWithoutGravity, Projectile.LocationWithoutGravity);
	}
	else if (BulletType == EBulletType::ProjectileAndGravity)
	{
		TraceAndDrawDebug(HitResults, PreLocationWithGravity,
		                  Projectile.LocationWithoutGravity - FVector(
			                  0.0f, 0.0f, Projectile.GravityOffset));
	}
	Projectile.HitResults.Append(HitResults);
}

void ATraceProjectile::CheckProjectileResult(FProjectileInstanceInfo& Projectile)
{
	// Projectile hit something
	for (const auto& HitResult : Projectile.HitResults)
	{
		OnProjectileHit.Broadcast(HitResult);
		Projectile.bIsActivate = false;
		UE_LOG(LogTraceProjectile, Warning, TEXT("[TraceProjectile] CheckProjectileResult: Hit!"))

	}

	// Projectile time out
	if (Projectile.ElapsedTime >= ProjectileLifespan - KINDA_SMALL_NUMBER)
	{
		Projectile.bIsActivate = false;
	}
}

void ATraceProjectile::TraceAndDrawDebug(TArray<FHitResult>& HitResults, const FVector Start, const FVector End) const
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetInstigator());

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ATraceProjectile), false);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);

	// Only the overlap hit results and the first block hit result will be generated.
	GetWorld()->LineTraceMultiByChannel(HitResults, Start, End,
	                                    ECollisionChannel::ECC_Visibility, Params,
	                                    FCollisionResponseParams());
	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 3.0f, 0, 5);
	}
}
