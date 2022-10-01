// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileComponent.h"

#include "AbilitySystemComponent.h"
#include "AliveWeapon.h"
#include "Alive.h"
#include "Character/AliveCharacter.h"
#include "DrawDebugHelpers.h"
#include "Weapon.h"
#include "Net/UnrealNetwork.h"

UProjectileComponent::UProjectileComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	SetAutoActivate(true);
	
	NetworkDelayTolerance = 200.0f;
	NetworkFluctuationTolerance = 200.0f;
	TargetMaximumVelocity = 6.0f;
}

void UProjectileComponent::InitProjectileComponent()
{
	check(GetWeaponType());
	UpdateInterval = 1.0f / static_cast<float>(GetWeaponType()->UpdateFrequency);
	ProjectileLifespan = GetWeaponType()->Range / GetWeaponType()->Velocity;
	CalculateWaitHitResultFrames();

	GetOwner()->GetWorldTimerManager().SetTimer(
		ProjectileTickTimerHandle, this, &UProjectileComponent::ProjectileTick, UpdateInterval, true);
}

void UProjectileComponent::GenerateProjectileInstance(uint8 ProjrctileID, const FVector& Location, const FVector& Direction)
{
	ProjectileInstances.Emplace(ProjrctileID, Location, Direction);
}

void UProjectileComponent::GenerateProjectileHandle(uint8 ProjectileID, const FGameplayEffectSpecHandle& HitEffectSpecHandle,
                                                    int32 BulletsPerCartridge)
{
	check(GetOwner()->HasAuthority());
	ServerProjectileHandles.Emplace(ProjectileID, HitEffectSpecHandle, BulletsPerCartridge);
}

void UProjectileComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ProjectileTickTimerHandle.IsValid())
	{
		GetOwner()->GetWorldTimerManager().ClearTimer(ProjectileTickTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UProjectileComponent::ProjectileTick()
{
	// A logical frame
	for (auto& Projectile : ProjectileInstances)
	{
		TArray<FHitResult> HitResults;
		UpdateProjectileOneFrame(HitResults, Projectile);
		ProcessHitResults(Projectile, HitResults);
	}
	// Clean up invalid ProjectileInstance
	ProjectileInstances.RemoveAllSwap([](const FProjectileInstance& Projectile)
	{
		return Projectile.bPendingKill;
	});

	if (GetOwner()->HasAuthority())
	{
		for (auto& ProjectileHandle : ServerProjectileHandles)
		{
			++ProjectileHandle.ElapsedFrames;
			// When the server wait times out
			if (ProjectileHandle.ElapsedFrames >= ProjectileLifespan * GetWeaponType()->UpdateFrequency + ServerWaitHitResultFrames)
			{
				ProjectileHandle.bPendingKill = true;
			}
		}
		// Clean up invalid ProjectileHandle
		ServerProjectileHandles.RemoveAllSwap([](const FServerProjectileHandle& Projectile)
		{
			return Projectile.bPendingKill;
		});
	}
}

void UProjectileComponent::TraceAndDrawDebug(OUT TArray<FHitResult>& HitResults, const FVector Start, const FVector End) const
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwnerAsWeapon()->GetOwner());

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ATraceProjectile), false);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);

	// Only the overlap hit results and the first block hit result will be generated.
	GetWorld()->LineTraceMultiByChannel(HitResults, Start, End, ECC_Projectile, Params, FCollisionResponseParams());

#ifdef ENABLE_DRAW_DEBUG
	if (GetWeaponType()->bDrawWeaponTrace)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 3.0f, 0, 2);
	}
#endif
}

void UProjectileComponent::ProcessHitResults(OUT FProjectileInstance& Projectile, const TArray<FHitResult>& HitResults)
{
	for (const auto& HitResult : HitResults)
	{
		if (Cast<AAliveCharacter>(HitResult.GetActor()) || HitResult.bBlockingHit)
		{
			ServerCheckHitResult(Projectile.ProjectileID, ShrinkHitResult(HitResult));
			Projectile.bPendingKill = true;

			GetWeaponType()->K2_OnProjectileHit(HitResult);

			break;
		}
	}
}

FHitResult UProjectileComponent::ShrinkHitResult(const FHitResult& HitResult) const
{
	FHitResult HitTemp = HitResult;
	HitTemp.ImpactPoint = HitTemp.Location;
	HitTemp.ImpactNormal = HitTemp.Normal;
	HitTemp.Item = INDEX_NONE;
	HitTemp.FaceIndex = INDEX_NONE;
	HitTemp.PenetrationDepth = 0.0f;
	HitTemp.ElementIndex = INDEX_NONE;
	return HitTemp;
}

void UProjectileComponent::UpdateProjectileOneFrame(OUT TArray<FHitResult>& HitResults,OUT FProjectileInstance& Projectile)
{
	float RealUpdateInterval = FMath::Min(ProjectileLifespan - Projectile.ElapsedTime, UpdateInterval);
	RealUpdateInterval = FMath::Clamp(RealUpdateInterval, 0.0f, UpdateInterval);
	if (RealUpdateInterval < UpdateInterval)
	{
		Projectile.bPendingKill = true; // Time out
	}

	const FVector PreLocation = Projectile.CurrentLocation;
	const float LastElapsedTime = Projectile.ElapsedTime;

	// Calculate current state
	Projectile.ElapsedTime += RealUpdateInterval;

	if (GetWeaponType()->bVelocity)
	{
		Projectile.CurrentLocation += Projectile.InitDirection * 100.0f * GetWeaponType()->Velocity * RealUpdateInterval;
		if (GetWeaponType()->bGravity)
		{
			Projectile.CurrentLocation.Z -= 0.5f * 100.0f * GetWeaponType()->Gravity * (FMath::Square(Projectile.ElapsedTime) -
				FMath::Square(LastElapsedTime));
		}
	}

	TraceAndDrawDebug(HitResults, PreLocation, Projectile.CurrentLocation);
}

void UProjectileComponent::ServerCheckHitResult_Implementation(uint8 ProjectileID, FHitResult HitResult)
{
	for (auto& ProjectileHandle : ServerProjectileHandles)
	{
		if (ProjectileHandle.bPendingKill == false && ProjectileHandle.ProjectileID == ProjectileID)
		{
			// If hit valid target
			if (const AAliveCharacter* TargetCharacter = Cast<AAliveCharacter>(HitResult.GetActor()))
			{
				FGameplayEffectContextHandle Context = ProjectileHandle.HitEffect.Data->GetContext();
				check(Context.IsValid());

				Context.AddHitResult(HitResult);

				Context.GetInstigatorAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(
					*ProjectileHandle.HitEffect.Data, TargetCharacter->GetAbilitySystemComponent());
			}

			MulticastProjectileHit(HitResult);

			--ProjectileHandle.NumPerCartridge;
			if (ProjectileHandle.NumPerCartridge == 0)
			{
				ProjectileHandle.bPendingKill = true;
			}

			break;
		}
	}
}

void UProjectileComponent::MulticastProjectileHit_Implementation(FHitResult HitResult)
{
	// TODO: There is a bug here. Ths way can not work after the owner dead. The Causer may see hit effects twice.
	if (!Cast<AAliveCharacter>(GetOwnerAsWeapon()->GetOwner())
		|| !Cast<AAliveCharacter>(GetOwnerAsWeapon()->GetOwner())->IsLocallyControlled())
	{
		GetWeaponType()->K2_OnProjectileHit(HitResult);
	}
}

void UProjectileComponent::CalculateWaitHitResultFrames()
{
	// Target's initial position error caused by NetworkDelay at the moment the Projectile is fired
	const float PositionError = NetworkDelayTolerance * TargetMaximumVelocity; // mm

	// An extreme situation where the object is dashing toward you.
	const float ServerWaitTime = (PositionError / (GetWeaponType()->Velocity + TargetMaximumVelocity) + NetworkFluctuationTolerance) * 0.001f; // s
	ServerWaitHitResultFrames = static_cast<int32>(ServerWaitTime / UpdateInterval + (1 - KINDA_SMALL_NUMBER));

	// An extreme situation where the object is running away from you.
	// Check if the projectile has a suitable velocity to hit the target
	if (GetWeaponType()->Velocity > 1.2f * TargetMaximumVelocity)
	{
		const float ClientWaitTime = (PositionError / (GetWeaponType()->Velocity - TargetMaximumVelocity) + NetworkFluctuationTolerance) * 0.001f; // s
		ClientWaitHitResultFrames = static_cast<int32>(ClientWaitTime / UpdateInterval + (1 - KINDA_SMALL_NUMBER));
	}
	else
	{
		const float ClientWaitTime = (5.0f * NetworkDelayTolerance + NetworkFluctuationTolerance) * 0.001f; // s
		ClientWaitHitResultFrames = static_cast<int32>(ClientWaitTime / UpdateInterval + (1 - KINDA_SMALL_NUMBER));
	}
}

AWeapon* UProjectileComponent::GetOwnerAsWeapon() const
{
	return Cast<AWeapon>(GetOwner());
}

const UWeaponType* UProjectileComponent::GetWeaponType() const
{
	return GetOwnerAsWeapon()->GetWeaponType();
}
