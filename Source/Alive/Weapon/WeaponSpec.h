// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponType.h"
#include "WeaponSpec.generated.h"

class UWeaponInventoryComponent;
/**
 * FWeaponSpecHandle
 * Used to find a weapon spec in a WeaponSpecContainer(WeaponInventoryComponent)
 */
USTRUCT(BlueprintType)
struct FWeaponSpecHandle : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FWeaponSpecHandle(): Handle(INDEX_NONE) { return; }

	/** True if GenerateNewHandle was called on this handle */
	bool IsValid() const { return Handle != INDEX_NONE; }

	/** True if GenerateNewHandle was called on this handle */
	void Reset() { Handle = INDEX_NONE; }

	/** Sets this to a valid handle */
	void GenerateNewHandle();

	bool operator==(const FWeaponSpecHandle& Other) const { return Handle == Other.Handle; }
	bool operator!=(const FWeaponSpecHandle& Other) const { return Handle != Other.Handle; }
	friend uint32 GetTypeHash(const FWeaponSpecHandle& SpecHandle) { return ::GetTypeHash(SpecHandle.Handle); }
	FString ToString() const { return IsValid() ? FString::FromInt(Handle) : TEXT("Invalid"); }

private:
	UPROPERTY()
	int32 Handle;
};

/**
 * FWeaponSpec
 */
USTRUCT(BlueprintType)
struct FWeaponSpec : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FWeaponSpec(): CurrentClipAmmo(0) { return; }

	FWeaponSpec(const UWeaponType* InWeaponType): WeaponType(InWeaponType)
	{
		WeaponSpecHandle.GenerateNewHandle();
		if (WeaponType)
		{
			CurrentClipAmmo = WeaponType->MaxClipAmmo;
		}
		else
		{
			CurrentClipAmmo = 0;
		}
	}

	bool operator==(const FWeaponSpec& Other) const { return GetWeaponSpecHandle() == Other.GetWeaponSpecHandle(); }

	FWeaponSpecHandle GetWeaponSpecHandle() const { return WeaponSpecHandle; }

	/** Always the ClassDefaultObject */
	UPROPERTY()
	const UWeaponType* WeaponType;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentClipAmmo;
	
	void PostReplicatedAdd(const struct FWeaponSpecContainer& InArraySerializer);
	void PreReplicatedRemove(const struct FWeaponSpecContainer& InArraySerializer);
	void PostReplicatedChange(const struct FWeaponSpecContainer& InArraySerializer);

private:
	/** Guarantee uniqueness, the same weapon (not the same type) will have the same handle on both the server and client. */
	UPROPERTY()
	FWeaponSpecHandle WeaponSpecHandle;
};

/**
 * FWeaponSpecContainer
 */
USTRUCT()
struct FWeaponSpecContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FWeaponSpecContainer(): Owner(nullptr) { return; }

	UPROPERTY()
	TArray<FWeaponSpec> Items;

	/** Component that owns this list */
	UPROPERTY()
	UWeaponInventoryComponent* Owner;

	/** Initializes Owner variable */
	void RegisterWithOwner(UWeaponInventoryComponent* InOwner) { Owner = InOwner; }

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FWeaponSpec, FWeaponSpecContainer>(Items, DeltaParms, *this);
	}
};

template <>
struct TStructOpsTypeTraits<FWeaponSpecContainer> : public TStructOpsTypeTraitsBase2<FWeaponSpecContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
