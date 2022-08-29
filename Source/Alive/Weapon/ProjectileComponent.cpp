// Fill out your copyright notice in the Description page of Project Settings.

#include "AliveWeapon.h"
#include "Weapon/ProjectileComponent.h"
#include "Net/UnrealNetwork.h"

UProjectileComponent::UProjectileComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
}

void UProjectileComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


void UProjectileComponent::BeginPlay()
{
	Super::BeginPlay();

	check(GetOwner());
	OwningWeapon = Cast<AAliveWeapon>(GetOwner());
}


void UProjectileComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

