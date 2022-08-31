// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayAbilityTargetData_GenerateProjectile.generated.h"

/** Target data used to sent the projectile info from client to server */
USTRUCT()
struct FGameplayAbilityTargetData_GenerateProjectile : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	FGameplayAbilityTargetData_GenerateProjectile()
		: ProjectileID(0), Direction(FVector_NetQuantize())
	{ }

	/** ID to allow the identification of the same projectile in the server and client. */
	UPROPERTY()
	int16 ProjectileID;

	/** An unit vector to indicate projectile's Direction. */
	UPROPERTY()
	FVector_NetQuantizeNormal Direction;
	
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_GenerateProjectile::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_GenerateProjectile> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_GenerateProjectile>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

