// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbilitySpec.h"
#include "Equipment.generated.h"

struct FNetSerializeScriptStructCache;
class UGameplayAbility;
class UAbilitySystemComponent;

/**
 * The base class of all equipment.
 * e.g. Weapon, Armor, Helmet ... 
 */
UCLASS(Abstract)
class ALIVE_API UEquipment : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSoftObjectPtr<USkeletalMesh> EquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesGrantedToOwner;
};


USTRUCT()
struct FRepEquipment
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FGameplayTag, float> Magnitudes;
	UPROPERTY()
	TArray<FGameplayTag> Tags;
};


USTRUCT()
struct FEquipmentSpec
{
	GENERATED_BODY()

	FEquipmentSpec() { return; }
	FEquipmentSpec(const TSubclassOf<UEquipment> InEquipmentType): EquipmentType(InEquipmentType) { return; }
	virtual ~FEquipmentSpec() { return; }

	UPROPERTY()
	TSubclassOf<UEquipment> EquipmentType;

	void GrantAbilities(UAbilitySystemComponent& AbilitySystemComponent);
	void RemoveAbilities(UAbilitySystemComponent& AbilitySystemComponent);

protected:
	UEquipment* GetEquipmentDefaultObject() const;

private:
	// Record the skills given by this weapon. Only useful on the server because we should only add/remove with authority.
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
};

/**
 * TODO: Replicate custom information at the beginning of the game and load the corresponding resource.
 */
USTRUCT()
struct FEquipmentPerformance
{
	GENERATED_BODY()
	virtual ~FEquipmentPerformance() { return; }

	/** Returns the serialization data, must always be overridden */
	virtual UScriptStruct* GetScriptStruct() const
	{
		return FEquipmentPerformance::StaticStruct();
	}

	virtual void SetEquipmentPerformance(USkeletalMesh& EquipmentMesh) { return; }
};

USTRUCT()
struct FEquipmentPerformanceHandle
{
	GENERATED_BODY()

	FEquipmentPerformanceHandle() { return; }
	FEquipmentPerformanceHandle(FEquipmentPerformance* DataPtr): Data(TSharedPtr<FEquipmentPerformance>(DataPtr)) { return; }
	FEquipmentPerformanceHandle(FEquipmentPerformanceHandle&& Other) noexcept: Data(MoveTemp(Other.Data)) { return; }
	FEquipmentPerformanceHandle(const FEquipmentPerformanceHandle& Other): Data(Other.Data) { return; }

	FEquipmentPerformanceHandle& operator=(FEquipmentPerformanceHandle&& Other) noexcept
	{
		Data = MoveTemp(Other.Data);
		return *this;
	}

	FEquipmentPerformanceHandle& operator=(const FEquipmentPerformanceHandle& Other)
	{
		Data = Other.Data;
		return *this;
	}

	/** Comparison operator */
	bool operator==(const FEquipmentPerformanceHandle& Other) const { return Data.Get() == Other.Get(); }
	bool operator!=(const FEquipmentPerformanceHandle& Other) const { return !(operator==(Other)); }

	bool IsValid() const { return Data.IsValid(); }
	/** Returns data, or nullptr if invalid */
	const FEquipmentPerformance* Get() const { return Data.IsValid() ? Data.Get() : nullptr; }
	void Set(FEquipmentPerformance* DataPtr) { Data = TSharedPtr<FEquipmentPerformance>(DataPtr); }
	void Reset() { Data.Reset(); }

	/** Serialize for networking, handles polymorphism */
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

private:
	// See UAbilitySystemGlobals.h
	FNetSerializeScriptStructCache GetEquipmentPerformanceStructCache() const;

	TSharedPtr<FEquipmentPerformance> Data;
	
	//void InitScriptStructs();
	//static TArray<UScriptStruct*> ScriptStructs;
};

template <>
struct TStructOpsTypeTraits<FEquipmentPerformanceHandle> : public TStructOpsTypeTraitsBase2<FEquipmentPerformanceHandle>
{
	enum
	{
		WithCopy = true,
		// Necessary so that TSharedPtr<FEquipmentPerformance> Data is copied around
		WithNetSerializer = true,
		WithIdenticalViaEquality = true,
	};
};
