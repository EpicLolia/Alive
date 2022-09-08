#pragma once

#include "CoreMinimal.h"

#include "AliveTypes.generated.h"

/**
 * FDamageResult
 * 
 * Show the Damage Number on the screen
 */
USTRUCT(BlueprintType)
struct ALIVE_API FDamageResult
{
	GENERATED_USTRUCT_BODY()

	FDamageResult(): Damage(0.0f), bBlock(false), bCriticalHit(false), bWeakness(false)
	{
	}
	
	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantize OppositeLocation;
	UPROPERTY(BlueprintReadOnly)
	float Damage;
	UPROPERTY(BlueprintReadOnly)
	uint8 bBlock:1;
	UPROPERTY(BlueprintReadOnly)
	uint8 bCriticalHit:1;
	UPROPERTY(BlueprintReadOnly)
	uint8 bWeakness:1;

	/** Optimized serialize function */
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template <>
struct TStructOpsTypeTraits<FDamageResult> : public TStructOpsTypeTraitsBase2<FDamageResult>
{
	enum
	{
		WithNetSerializer = true,
	};
};


