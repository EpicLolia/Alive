// Fill out your copyright notice in the Description page of Project Settings.

#include "AliveWeapon.h"

AAliveWeapon::AAliveWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAliveWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AAliveWeapon::NotifyActorBeginOverlap(AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	
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


