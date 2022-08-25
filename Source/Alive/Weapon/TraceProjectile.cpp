// Fill out your copyright notice in the Description page of Project Settings.


#include "TraceProjectile.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogTraceProjectile, Log, All);

// Sets default values
ATraceProjectile::ATraceProjectile()
	: BulletType(EBulletType::Hitscan)
	  , UpdateFrequency(10)
	  , Range(120000.0f)
	  , Velocity(60000.0f)
	  , Gravity(980.0f)
	  , bDrawDebug(false)
	  , ElapsedTimeSinceLastUpdate(0.0f)
	  , UpdateInterval(0.1f)
	  , ProjectileLifespan(1.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	UE_LOG(LogTraceProjectile, Warning, TEXT("[TraceProjectile]: Range is: %f"), Range);
}

void ATraceProjectile::Fire(FVector Location, FRotator Rotation, int32 BulletNum)
{
	while ((BulletNum--) > 0)
	{
		ProjectileInstances.Add(FProjectileInstanceInfo(Location, Rotation));
	}
}

void ATraceProjectile::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTraceProjectile, Warning, TEXT("[TraceProjectile]: Range is: %f"), Range);

	UpdateInterval = 1.0f / static_cast<float>(UpdateFrequency);
	ProjectileLifespan = Range / Velocity;

	OnProjectileHit.AddDynamic(this, &ATraceProjectile::ProjectileHitEvent);
}

void ATraceProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTimeSinceLastUpdate += DeltaTime;

	TArray<FProjectileInstanceInfo> InstancesToRemove;
	while (ElapsedTimeSinceLastUpdate >= UpdateInterval)
	{
		ElapsedTimeSinceLastUpdate -= UpdateInterval;
		for (auto& Projectile : ProjectileInstances)
		{
			Projectile.ElapsedTime += UpdateInterval;
			if (Projectile.ElapsedTime >= ProjectileLifespan)
			{
				Projectile.ElapsedTime = ProjectileLifespan;
				InstancesToRemove.Add(Projectile);
			}
			FVector PreviousLocation = Projectile.LocationWithoutGravity;
			Projectile.LocationWithoutGravity = Projectile.Rotation.Vector() * Velocity * Projectile.ElapsedTime;
			Projectile.GravityOffset = 0.5f * Gravity * FMath::Square(Projectile.ElapsedTime);

			if (BulletType == EBulletType::Projectile)
			{
				TraceAndDrawDebug(PreviousLocation, Projectile.LocationWithoutGravity);
			}
			else if (BulletType == EBulletType::ProjectileAndGravity)
			{
				TraceAndDrawDebug(PreviousLocation, Projectile.LocationWithoutGravity - Projectile.GravityOffset);
			}
		}
	}

	for (const auto& Instance : InstancesToRemove)
	{
		ProjectileInstances.RemoveSingleSwap(Instance);
	}
}

void ATraceProjectile::ProjectileHitEvent_Implementation(const FHitResult HitResult)
{
}

void ATraceProjectile::TraceAndDrawDebug(const FVector Start, const FVector End) const
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetInstigator());

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ATraceProjectile), false);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);

	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByChannel(HitResults, Start, End,
	                                    ECollisionChannel::ECC_Visibility, Params,
	                                    FCollisionResponseParams());
	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, true, 3.0f);
	}
}
