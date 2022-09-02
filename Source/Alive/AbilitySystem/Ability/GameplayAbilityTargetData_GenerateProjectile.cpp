// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayAbilityTargetData_GenerateProjectile.h"

bool FGameplayAbilityTargetData_GenerateProjectile::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Direction.NetSerialize(Ar, Map, bOutSuccess);
	SourceLocation.NetSerialize(Ar, Map, bOutSuccess);
	return true;
}
