// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileComponent.h"

#include "AbilitySystemComponent.h"
#include "AliveWeapon.h"
#include "Alive.h"
#include "Character/AliveCharacter.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogProjectile, Log, All);

UProjectileComponent::UProjectileComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	UpdateFrequency = 10;
	bDrawDebug = false;
	ProjectileType = EProjectileType::VelocityAndGravity;
	Range = 1200.0f;
	Velocity = 600.0f;
	Gravity = 9.8f;

	NetworkDelayTolerance = 200.0f;
	NetworkFluctuationTolerance = 200.0f;
	TargetMaximumVelocity = 6.0f;
}

void UProjectileComponent::FireOneProjectile(uint8 ProjrctileID, const FVector& Location, const FVector& Direction,
                                             const FGameplayEffectSpecHandle& HitEffectSpecHandle)
{
	ProjectileInstances.Emplace(ProjrctileID, Location, Direction, HitEffectSpecHandle);
}

void UProjectileComponent::BeginPlay()
{
	Super::BeginPlay();

	// Owner must be an AliveWeapon
	OwningWeapon = Cast<AAliveWeapon>(GetOwner());
	check(OwningWeapon);

	UpdateInterval = 1.0f / static_cast<float>(UpdateFrequency);
	ProjectileLifespan = Range / Velocity;
	CalculateWaitHitResultFrames();
}


void UProjectileComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ElapsedTimeSinceLastUpdate += DeltaTime;
	// Only loop once in theory when the tick rate is higher than UpdateFrequency
	while (ElapsedTimeSinceLastUpdate >= UpdateInterval)
	{
		// A logical frame
		for (auto& Projectile : ProjectileInstances)
		{
			UpdateProjectileOneFrame(Projectile);
		}

		// Clean up invalid ProjectileInstance
		ProjectileInstances.RemoveAllSwap([](const FProjectileInstance& Projectile)
		{
			return Projectile.bPendingKill;
		});

		ElapsedTimeSinceLastUpdate -= UpdateInterval;
	}
}

void UProjectileComponent::TraceAndDrawDebug(OUT TArray<FHitResult>& HitResults, const FVector Start, const FVector End) const
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwningWeapon->GetOwningCharacter());

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ATraceProjectile), false);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);

	// Only the overlap hit results and the first block hit result will be generated.
	GetWorld()->LineTraceMultiByChannel(HitResults, Start, End, ECC_Projectile, Params, FCollisionResponseParams());

#ifdef ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 3.0f, 0, 5);
	}
#endif
}

void UProjectileComponent::ProcessHitResults(FProjectileInstance& Projectile, const TArray<FHitResult>& HitResults)
{
	for (const auto& HitResult : HitResults)
	{
		if (Cast<AAliveCharacter>(HitResult.GetActor()))
		{
			if (!Projectile.HitEffect.IsValid() || Projectile.HitEffect.Data->GetContext().IsLocallyControlled())
			{
				ServerCheckHitResult(Projectile.ProjectileID, HitResult);
				Projectile.bPendingKill = true;
			}
		}
		else if (HitResult.bBlockingHit)
		{
			Projectile.bPendingKill = true;
		}
	}
}

void UProjectileComponent::UpdateProjectileOneFrame(FProjectileInstance& Projectile)
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
	switch (ProjectileType)
	{
	case EProjectileType::Velocity:
		Projectile.CurrentLocation += Projectile.InitDirection * 100.0f * Velocity * RealUpdateInterval;
		break;
	case EProjectileType::VelocityAndGravity:
		Projectile.CurrentLocation += Projectile.InitDirection * 100.0f * Velocity * RealUpdateInterval;
		Projectile.CurrentLocation.Z -= 0.5f * 100.0f * Gravity * (FMath::Square(Projectile.ElapsedTime) - FMath::Square(LastElapsedTime));
		break;
	default:
		UE_LOG(LogProjectile, Error, TEXT("Projectile Component has an error Projectiletype!"));
		break;
	}

	TArray<FHitResult> HitResults;
	TraceAndDrawDebug(HitResults, PreLocation, Projectile.CurrentLocation);

	ProcessHitResults(Projectile, HitResults);
}

void UProjectileComponent::ServerCheckHitResult_Implementation(uint8 ProjectileID, FHitResult HitResult)
{
	for (const auto Projectile : ProjectileInstances)
	{
		if (Projectile.ProjectileID == ProjectileID)
		{
			FGameplayEffectContextHandle Context = Projectile.HitEffect.Data->GetContext();
			check(Context.IsValid());

			Context.AddHitResult(HitResult);

			Context.GetInstigatorAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(
				*Projectile.HitEffect.Data, Cast<AAliveCharacter>(HitResult.GetActor())->GetAbilitySystemComponent());
		}
	}
}

void UProjectileComponent::CalculateWaitHitResultFrames()
{
	// Target's initial position error caused by NetworkDelay at the moment the Projectile is fired
	const float PositionError = NetworkDelayTolerance * TargetMaximumVelocity; // mm

	// An extreme situation where the object is dashing toward you.
	const float ServerWaitTime = (PositionError / (Velocity + TargetMaximumVelocity) + NetworkFluctuationTolerance) * 0.001f; // s
	ServerWaitHitResultFrames = static_cast<int32>(ServerWaitTime / UpdateInterval + (1 - KINDA_SMALL_NUMBER));

	// An extreme situation where the object is running away from you.
	// Check if the projectile has a suitable velocity to hit the target
	if (Velocity > 1.2f * TargetMaximumVelocity)
	{
		const float ClientWaitTime = (PositionError / (Velocity - TargetMaximumVelocity) + NetworkFluctuationTolerance) * 0.001f; // s
		ClientWaitHitResultFrames = static_cast<int32>(ClientWaitTime / UpdateInterval + (1 - KINDA_SMALL_NUMBER));
	}
	else
	{
		const float ClientWaitTime = (5.0f * NetworkDelayTolerance + NetworkFluctuationTolerance) * 0.001f; // s
		ClientWaitHitResultFrames = static_cast<int32>(ClientWaitTime / UpdateInterval + (1 - KINDA_SMALL_NUMBER));
	}
}
