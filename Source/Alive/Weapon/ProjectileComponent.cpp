// Fill out your copyright notice in the Description page of Project Settings.

#include "AliveWeapon.h"
#include "Weapon/ProjectileComponent.h"
#include "Alive.h"
#include "Character/AliveCharacter.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

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
}

void UProjectileComponent::BeginPlay()
{
	Super::BeginPlay();

	// Owner must be an AliveWeapon
	OwningWeapon = Cast<AAliveWeapon>(GetOwner());
	check(OwningWeapon);
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

		}

		// Clean up invalid ProjectileInstance
		ProjectileInstances.RemoveAllSwap([](const FProjectileInstance& Projectile)
		{
			return Projectile.PendingKill;
		});
		
		ElapsedTimeSinceLastUpdate -= UpdateInterval;
	}
}

void UProjectileComponent::TraceAndDrawDebug(TArray<FHitResult>& HitResults, const FVector Start, const FVector End) const
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwningWeapon->GetOwningCharacter());

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ATraceProjectile), false);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);

	// Only the overlap hit results and the first block hit result will be generated.
	GetWorld()->LineTraceMultiByChannel(HitResults, Start, End,
	                                    ECC_Projectile, Params,
	                                    FCollisionResponseParams());
	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 3.0f, 0, 5);
	}
}
