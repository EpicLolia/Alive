// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSpec.h"

#include "WeaponInventoryComponent.h"

void FWeaponSpecHandle::GenerateNewHandle()
{
	static int32 GHandle = 0;
	Handle = GHandle++;
}

void FWeaponSpec::PostReplicatedAdd(const FWeaponSpecContainer& InArraySerializer)
{
	if(InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnWeaponInventoryAdd.Broadcast();
	}
}

void FWeaponSpec::PreReplicatedRemove(const FWeaponSpecContainer& InArraySerializer)
{
	if(InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnWeaponInventoryRemove.Broadcast();
	}
}

void FWeaponSpec::PostReplicatedChange(const FWeaponSpecContainer& InArraySerializer)
{
	if(InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnCurrentAmmoChanged.Broadcast(CurrentClipAmmo);
	}
}
