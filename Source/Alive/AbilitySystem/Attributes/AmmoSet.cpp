// Copyright Epic Games, Inc. All Rights Reserved.

#include "AmmoSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UAmmoSet::UAmmoSet()
{
	RifleAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rifle"));
	PistolAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Pistol"));
}

void UAmmoSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoSet, RifleReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoSet, MaxRifleReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoSet, PistolReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoSet, MaxPistolReserveAmmo, COND_None, REPNOTIFY_Always);
}

void UAmmoSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UAmmoSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetRifleReserveAmmoAttribute())
	{
		float Ammo = GetRifleReserveAmmo();
		SetRifleReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxRifleReserveAmmo()));
	}
	else if(Data.EvaluatedData.Attribute == GetPistolReserveAmmoAttribute())
	{
		float Ammo = GetPistolReserveAmmo();
		SetPistolReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxPistolReserveAmmo()));
	}
}

FGameplayAttribute UAmmoSet::GetReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag)
{
	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rifle")))
	{
		return GetRifleReserveAmmoAttribute();
	}
	else if(PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Pistol")))
	{
		return GetPistolReserveAmmoAttribute();
	}
	
	return FGameplayAttribute();
}

FGameplayAttribute UAmmoSet::GetMaxReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag)
{
	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rifle")))
	{
		return GetMaxRifleReserveAmmoAttribute();
	}
	else if(PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Pistol")))
	{
		return GetMaxPistolReserveAmmoAttribute();
	}
	
	return FGameplayAttribute();
}

void UAmmoSet::OnRep_RifleReserveAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoSet, RifleReserveAmmo, OldValue);
}

void UAmmoSet::OnRep_MaxRifleReserveAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoSet, MaxRifleReserveAmmo, OldValue);
}

void UAmmoSet::OnRep_PistolReserveAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoSet, PistolReserveAmmo, OldValue);
}

void UAmmoSet::OnRep_MaxPistolReserveAmmo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoSet, MaxPistolReserveAmmo, OldValue);
}
