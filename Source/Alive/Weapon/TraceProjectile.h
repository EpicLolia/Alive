// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TraceProjectile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProjectileHitDelegate, const FHitResult, HitResult);

USTRUCT()
struct ALIVE_API FProjectileInstanceInfo
{
	GENERATED_BODY()
public:
	FVector LocationWithoutGravity;
	FRotator Rotation;

	TArray<FHitResult> HitResults;

	float GravityOffset;
	float ElapsedTime;

	bool bIsActivate;

public:
	FProjectileInstanceInfo(): LocationWithoutGravity(FVector(0.0f, 0.0f, 0.0f))
	                           , Rotation(FRotator(0.0f, 0.0f, 0.0f))
	                           , GravityOffset(0.0f), ElapsedTime(0.0f), bIsActivate(false)
	{
	}

	bool operator ==(const FProjectileInstanceInfo& rhs) const
	{
		return this->Rotation.Equals(rhs.Rotation) &&
			this->LocationWithoutGravity.Equals(rhs.LocationWithoutGravity) &&
			FMath::Abs(this->GravityOffset - rhs.GravityOffset) <= KINDA_SMALL_NUMBER &&
			FMath::Abs(this->ElapsedTime - rhs.ElapsedTime) <= KINDA_SMALL_NUMBER;
	}

	void Activate(FVector InLocation, FRotator InRotator)
	{
		LocationWithoutGravity = InLocation;
		Rotation = InRotator;
		HitResults.Reset();
		GravityOffset = 0.0f;
		ElapsedTime = 0.0f;
		bIsActivate = true;
	}
};

UENUM(BlueprintType)
enum class EBulletType:uint8
{
	/** return the hit result instantly while Firing. */
	Hitscan,
	/** Consider the velocity of the bullet. */
	Projectile,
	/** Consider both the velocity and gravity of the bullet . */
	ProjectileAndGravity
};


UCLASS()
class ALIVE_API ATraceProjectile : public AActor
{
	GENERATED_BODY()

public:
	ATraceProjectile();

	/**
	 * Register a bullet for ticking
	 *
	 * @param ProjectileID Which projectile did you fire in the clip
	 */
	UFUNCTION(BlueprintCallable)
	void FireOnce(FVector Location, FRotator Rotation, int32 ProjectileID);

	UPROPERTY(BlueprintAssignable)
	FProjectileHitDelegate OnProjectileHit;

protected:
	virtual void BeginPlay() override;

	/** This determines the maximum number of bullets that can be updated at the same time */
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = true))
	int32 ClipAmmo;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive")
	EBulletType BulletType;
	/** MaxFrequency is decided by tick rate. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive", meta = (ClampMin = 1, ClampMax = 60))
	int32 UpdateFrequency;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive")
	bool bDrawDebug;
	/** How far the bullet can go. (m) */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Projectile", meta = (ClampMin = 1.0f, ClampMax = 3000.0f))
	float Range;
	/** Will not work while the BulletType is hitscan. The Speed of the bullet. (m/s) */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Projectile", meta = (ClampMin = 1.0f, ClampMax = 1200.0f))
	float Velocity;
	/** Only work while BulletType is ProjectileAndGravity. (m/s^2) */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Projectile", meta = (ClampMin = 0.1f, ClampMax = 100.0f))
	float Gravity;
	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
	void ProjectileHitEvent(const FHitResult HitResult);
	virtual void ProjectileHitEvent_Implementation(const FHitResult HitResult);

private:
	TArray<FProjectileInstanceInfo> ProjectileInstances;

	float ElapsedTimeSinceLastUpdate;
	// Calculated by UpdateFrequency
	float UpdateInterval;
	// Calculated by Range and velocity
	float ProjectileLifespan;

	void TraceAndDrawDebug(TArray<FHitResult>& HitResults, const FVector Start, const FVector End) const;

	// Take UpdateInterval, ElapsedTime and ProjectileLifspan in consideration.
	float GetAppropriateTimeStep(const FProjectileInstanceInfo& Projectile, float ElapsedTime)const;
	void UpdateProjectileWithTime(FProjectileInstanceInfo& Projectile, float ElapsedTime);
	void UpdateProjectileOneStep(FProjectileInstanceInfo& Projectile, float TimeStep);
	void CheckProjectileResult(FProjectileInstanceInfo& Projectile);
};

