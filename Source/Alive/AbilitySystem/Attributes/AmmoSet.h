// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AliveAttributeSet.h"
#include "NativeGameplayTags.h"
#include "AmmoSet.generated.h"

/**
 * UAmmoSet
 * 
 *	Class that defines attributes that are cost by weapon.
 *	Attribute examples include: RifleAmmo, healing, attack power, and shield penetrations.
 */
UCLASS(BlueprintType)
class UAmmoSet : public UAliveAttributeSet
{
	GENERATED_BODY()

public:
	UAmmoSet();
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

public:
	static FGameplayAttribute GetReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag);
	static FGameplayAttribute GetMaxReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag);

protected:
	// Cache tags
	FGameplayTag RifleAmmoTag;
	FGameplayTag PistolAmmoTag;

public:
	ATTRIBUTE_ACCESSORS(UAmmoSet, RifleReserveAmmo);
	ATTRIBUTE_ACCESSORS(UAmmoSet, MaxRifleReserveAmmo);
	ATTRIBUTE_ACCESSORS(UAmmoSet, PistolReserveAmmo);
	ATTRIBUTE_ACCESSORS(UAmmoSet, MaxPistolReserveAmmo);
	
protected:
	UFUNCTION()
	void OnRep_RifleReserveAmmo(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxRifleReserveAmmo(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_PistolReserveAmmo(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxPistolReserveAmmo(const FGameplayAttributeData& OldValue);

private:
	// Amount of Rifle's Ammo owner by player. Excluding Ammo in the weapon magazine.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RifleReserveAmmo, Category = "Alive|Ammo", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData RifleReserveAmmo;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxRifleReserveAmmo, Category = "Alive|Ammo", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxRifleReserveAmmo;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PistolReserveAmmo, Category = "Alive|Ammo", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData PistolReserveAmmo;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPistolReserveAmmo, Category = "Alive|Ammo", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxPistolReserveAmmo;
};
