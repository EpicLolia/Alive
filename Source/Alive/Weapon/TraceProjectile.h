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

	FProjectileInstanceInfo(): LocationWithoutGravity(FVector(0.0f, 0.0f, 0.0f))
	                           , Rotation(FRotator(0.0f, 0.0f, 0.0f))
	                           , GravityOffset(0.0f), ElapsedTime(0.0f)
	{
	}

	FProjectileInstanceInfo(FVector InLocation, FRotator InRotator)
		: LocationWithoutGravity(InLocation), Rotation(InRotator)
		  , GravityOffset(0.0f), ElapsedTime(0.0f)
	{
	}

	FVector LocationWithoutGravity;
	FRotator Rotation;

	float GravityOffset;
	float ElapsedTime;

	bool operator ==(const FProjectileInstanceInfo& rhs) const
	{
		return this->Rotation.Equals(rhs.Rotation) &&
			this->LocationWithoutGravity.Equals(rhs.LocationWithoutGravity) &&
				FMath::Abs(this->GravityOffset-rhs.GravityOffset) <= KINDA_SMALL_NUMBER&&
				FMath::Abs(this->ElapsedTime-rhs.ElapsedTime) <= KINDA_SMALL_NUMBER;
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

	UFUNCTION(BlueprintCallable)
	void Fire(FVector Location, FRotator Rotation, int32 BulletNum = 1);

	UPROPERTY(BlueprintAssignable)
	FProjectileHitDelegate OnProjectileHit;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = true))
	EBulletType BulletType;
	/** MaxFrequency is decided by tick rate. (/s) */
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = true, ClampMin = 1, ClampMax = 60))
	int32 UpdateFrequency;
	/** How far the bullet can go. (cm)*/
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = true, ClampMin = 200.0f, ClampMax = 999999.9f))
	float Range;
	/** Will not work while the BulletType is hitscan. The Speed of the bullet. (cm/s)*/
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = true, ClampMin = 200.0f, ClampMax = 999999.9f))
	float Velocity;
	/** Only work while BulletType is ProjectileAndGravity. (cm/s^2) */
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = true, ClampMin = 100.0f, ClampMax = 999999.9f))
	float Gravity;
	/** Only work while BulletType is ProjectileAndGravity. */
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = true))
	bool bDrawDebug;

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

	void TraceAndDrawDebug(const FVector Start, const FVector End) const;
};
