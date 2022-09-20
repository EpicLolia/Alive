// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSpec.h"

#include "WeaponInventoryComponent.h"

void FWeaponSpecHandle::GenerateNewHandle()
{
	static int32 GHandle = 0;
	Handle = GHandle++;
}

void FWeaponSpec::PostReplicatedChange(const FWeaponSpecContainer& InArraySerializer)
{
	if(InArraySerializer.Owner)
	{
		InArraySerializer.Owner->OnCurrentAmmoChanged.Broadcast(CurrentClipAmmo);
	}
}
