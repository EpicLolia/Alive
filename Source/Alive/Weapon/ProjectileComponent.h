// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectileComponent.generated.h"


USTRUCT()
struct ALIVE_API FProjectileInstance
{
	GENERATED_BODY()

public:
	FProjectileInstance(const FVector& Location = FVector(), const FRotator& Rotation = FRotator())
		: CurrentLocation(Location), InitRotation(Rotation), ElapsedTime(0.0f), PendingKill(false)
	{
	}

	FVector CurrentLocation;
	FRotator InitRotation;
	float ElapsedTime;
	bool PendingKill;
};

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	/** return the hit result instantly while Firing. */
	Hitscan,
	/** Consider the velocity of the Projectile. */
	Velocity,
	/** Consider both the velocity and gravity of the Projectile . */
	VelocityAndGravity
};

/**
 * Provides functional Projectiles based on ray detection
 */
UCLASS(ClassGroup=(Alive), hidecategories=(Object,LOD,Lighting,Transform,Sockets,TextureStreaming))
class ALIVE_API UProjectileComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectileComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class AAliveWeapon* OwningWeapon;

protected:
	/**
	 * Use a logical tick rate. Ensure that the client and server have the same projectile simulation results.
	 * MaxFrequency is decided by tick rate.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive", meta = (ClampMin = 1, ClampMax = 60))
	int32 UpdateFrequency;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive")
	bool bDrawDebug;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Projectile")
	EProjectileType ProjectileType;
	/** How far the projectile can go. (m) */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Projectile", meta = (ClampMin = 1.0f, ClampMax = 3000.0f))
	float Range;
	/** Will not work while the ProjectileType is hitscan. (m/s) */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Projectile", meta = (ClampMin = 1.0f, ClampMax = 1200.0f))
	float Velocity;
	/** Only work while ProjectileType is ProjectileAndGravity. (m/s^2) */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Projectile", meta = (ClampMin = 0.1f, ClampMax = 100.0f))
	float Gravity;

	// Override in blueprint to show visual effects.
	UFUNCTION(BlueprintImplementableEvent)
	void OnProjectileHit(const FHitResult HitResult);
	
private:
	TArray<FProjectileInstance> ProjectileInstances;

	void TraceAndDrawDebug(TArray<FHitResult>& HitResults, const FVector Start, const FVector End) const;

	// A temp
	float ElapsedTimeSinceLastUpdate = 0.0f;
	// Calculated by UpdateFrequency
	float UpdateInterval;
	// Calculated by Range and velocity
	float ProjectileLifespan;
};
