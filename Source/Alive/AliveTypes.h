#pragma once

#include "CoreMinimal.h"

#include "AliveTypes.generated.h"

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	Warmup,
	Playing,
	Finish,
	Evaluate
};

USTRUCT()
struct FTransformWithVelocity
{
	GENERATED_BODY()

	FTransformWithVelocity(const FTransform& InTransform = FTransform(), FVector InVelocity = FVector())
		: Location(InTransform.GetLocation())
		  , Rotation(InTransform.GetRotation())
		  , Velocity(InVelocity)
	{
	}

	UPROPERTY()
	FVector_NetQuantize Location;
	UPROPERTY()
	FQuat Rotation;
	UPROPERTY()
	FVector_NetQuantize Velocity;

	FTransform ToTransform() const { return FTransform(Rotation, Location); }

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template <>
struct TStructOpsTypeTraits<FTransformWithVelocity> : public TStructOpsTypeTraitsBase2<FTransformWithVelocity>
{
	enum
	{
		WithNetSerializer = true,
	};
};

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
