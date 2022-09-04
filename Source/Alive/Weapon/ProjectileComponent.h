// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Components/ActorComponent.h"
#include "ProjectileComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProjectileHitDelegate,const FHitResult&, HitResult);

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
 * Provides functional Projectiles based on ray detection.
 * Client generates hit result, than Server checks the hit result sent from client and apply damage.
 * It also works at low projectile speeds, but you are advised to use the server for generating hit result in this case.
 */
UCLASS(ClassGroup=(Alive), hidecategories=(Object,LOD,Lighting,Transform,Sockets,TextureStreaming))
class ALIVE_API UProjectileComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectileComponent();

	void GenerateProjectileInstance(uint8 ProjectileID, const FVector& Location, const FVector& Direction);
	// Should only be called on the server. 
	void GenerateProjectileHandle(uint8 ProjectileID, const FGameplayEffectSpecHandle& HitEffectSpecHandle, int32 BulletsPerCartridge = 1);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	class AAliveWeapon* OwningWeapon;

public:
	float GetRange() const { return Range; }

protected:
	/**
	 * Use a logical tick rate. Ensure that the client and server have the same projectile simulation results.
	 * MaxFrequency is decided by tick rate.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Alive", meta = (ClampMin = 1, ClampMax = 60))
	int32 UpdateFrequency;
	UPROPERTY(EditDefaultsOnly, Category = "Alive")
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

	/** The maximum RTT allowed by the server. Used to calculate WaitHitResultFrames (ms) */
	UPROPERTY(EditDefaultsOnly, Category = "Alive|Network", meta = (ClampMin = 10.0f, ClampMax = 1000.0f))
	float NetworkDelayTolerance;
	/** The maximum Fluctuation allowed by the server. Used to calculate WaitHitResultFrames (ms) */
	UPROPERTY(EditDefaultsOnly, Category = "Alive|Network", meta = (ClampMin = 10.0f, ClampMax = 1000.0f))
	float NetworkFluctuationTolerance;
	/** The maximum Velocity of target object you want to shoot at. Used to calculate WaitHitResultFrames (m/s) */
	UPROPERTY(EditDefaultsOnly, Category = "Alive|Network", meta = (ClampMin = 1.0, ClampMax = 20.0f))
	float TargetMaximumVelocity;

	UPROPERTY(BlueprintAssignable)
	FProjectileHitDelegate OnProjectileHit;
private:
	/**
	 * Only Useful on the server.
	 */
	struct FServerProjectileHandle
	{
		FServerProjectileHandle(
			uint8 UniqueProjectileID, const FGameplayEffectSpecHandle& HitEffectSpecHandle, int32 BulletsPerCartridge)
			: ProjectileID(UniqueProjectileID), bPendingKill(false)
			  , HitEffect(HitEffectSpecHandle), NumPerCartridge(BulletsPerCartridge), ElapsedFrames(0)
		{
		}

		// Used to identify the same projectile in the server and client. Generated by client and sent it to server.
		uint8 ProjectileID;
		bool bPendingKill;
		FGameplayEffectSpecHandle HitEffect;
		// How many times does this handle can be trigger? Think about Shotgun.
		int32 NumPerCartridge;
		int32 ElapsedFrames;
	};

	struct FProjectileInstance
	{
		FProjectileInstance(uint8 UniqueProjectileID, const FVector& Location, const FVector& Direction)
			: ProjectileID(UniqueProjectileID), bPendingKill(false), CurrentLocation(Location), InitDirection(Direction)
			  , ElapsedTime(0.0f)
		{
			InitDirection.Normalize();
		}

		// Used to identify the same projectile in the server and client. Generated by client and sent it to server.
		uint8 ProjectileID;
		bool bPendingKill;
		FVector CurrentLocation;
		FVector InitDirection;

		float ElapsedTime;
	};

	TArray<FServerProjectileHandle> ServerProjectileHandles;
	TArray<FProjectileInstance> ProjectileInstances;
	
	void UpdateProjectileOneFrame(FProjectileInstance& Projectile);
	void TraceAndDrawDebug(OUT TArray<FHitResult>& HitResults, const FVector Start, const FVector End) const;
	void ProcessHitResults(FProjectileInstance& Projectile, const TArray<FHitResult>& HitResults);

	// Reduce memory for replicating. Return a copy of HitResult
	FHitResult ShrinkHitResult(const FHitResult& HitResult) const;
	UFUNCTION(Server, Reliable)
	void ServerCheckHitResult(uint8 ProjectileID, FHitResult HitResult);
	void ServerCheckHitResult_Implementation(uint8 ProjectileID, FHitResult HitResult);
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastProjectileHit(FHitResult HitResult);
	void MulticastProjectileHit_Implementation(FHitResult HitResult);
	
	void ProjectileTick();
	FTimerHandle ProjectileTickTimerHandle;

	// Based on UpdateFrequency
	float UpdateInterval;
	// Based on Range and velocity
	float ProjectileLifespan;

	// The following two figures are not far apart when the projectile velocity is much greater than target movement speed.
	// The frames that server's hit result should wait for client's hit result.
	int32 ServerWaitHitResultFrames;
	// The frames that client's hit result should wait for server's hit result.
	int32 ClientWaitHitResultFrames;
	// Called in BeginPlay after initializing other data, like UpdateInterval.
	void CalculateWaitHitResultFrames();
};
