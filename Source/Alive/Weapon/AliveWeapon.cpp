// Fill out your copyright notice in the Description page of Project Settings.

#include "AliveWeapon.h"
#include "Net/UnrealNetwork.h"

AAliveWeapon::AAliveWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAliveWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAliveWeapon, PrimaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AAliveWeapon, MaxPrimaryClipAmmo, COND_OwnerOnly);
}

void AAliveWeapon::NotifyActorBeginOverlap(AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	// TODO: 碰到武器时拾取
}

void AAliveWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AAliveWeapon::OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo)
{
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo,PrimaryClipAmmo);
}

void AAliveWeapon::OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo)
{
	OnMaxPrimaryClipAmmoChanged.Broadcast(OldMaxPrimaryClipAmmo, MaxPrimaryClipAmmo);
}


