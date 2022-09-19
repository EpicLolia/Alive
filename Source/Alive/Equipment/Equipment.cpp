// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment.h"

#include "AliveLogChannels.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbilitySpec.h"

void FEquipmentSpec::GrantAbilities(UAbilitySystemComponent& AbilitySystemComponent)
{
	check(AbilitySystemComponent.GetOwnerRole() == ROLE_Authority);
	check(EquipmentType);
	check(AbilitySpecHandles.Num() == 0);

	for (const TSubclassOf<UGameplayAbility>& Ability : GetEquipmentDefaultObject()->AbilitiesGrantedToOwner)
	{
		AbilitySpecHandles.Add(AbilitySystemComponent.GiveAbility(
			FGameplayAbilitySpec(Ability)));
	}
}

void FEquipmentSpec::RemoveAbilities(UAbilitySystemComponent& AbilitySystemComponent)
{
	check(AbilitySystemComponent.GetOwnerRole() == ROLE_Authority);

	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
	{
		AbilitySystemComponent.ClearAbility(SpecHandle);
	}
	AbilitySpecHandles.Reset();
}

UEquipment* FEquipmentSpec::GetEquipmentDefaultObject() const
{
	check(EquipmentType);
	return EquipmentType.GetDefaultObject();
}

struct FEquipmentPerformanceDeleter
{
	FORCEINLINE void operator()(FEquipmentPerformance* Object) const
	{
		check(Object);
		UScriptStruct* ScriptStruct = Object->GetScriptStruct();
		check(ScriptStruct);
		ScriptStruct->DestroyStruct(Object);
		FMemory::Free(Object);
	}
};

// Copy from GameplayTargetData
bool FEquipmentPerformanceHandle::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	TCheckedObjPtr<UScriptStruct> ScriptStruct = Data.IsValid() ? Data->GetScriptStruct() : nullptr;
	
	Ar << ScriptStruct.Get(); // TODO: Check Safe
	
	if (ScriptStruct.IsValid())
	{
		if (Ar.IsLoading())
		{
			// For now, just always reset/reallocate the data when loading.
			// Longer term if we want to generalize this and use it for property replication, we should support
			// only reallocating when necessary
			check(!Data.IsValid());

			FEquipmentPerformance* NewData = (FEquipmentPerformance*)FMemory::Malloc(ScriptStruct->GetStructureSize());
			ScriptStruct->InitializeStruct(NewData);

			Data = TSharedPtr<FEquipmentPerformance>(NewData, FEquipmentPerformanceDeleter());
		}

		void* ContainerPtr = Data.Get();

		if (ScriptStruct->StructFlags & STRUCT_NetSerializeNative)
		{
			ScriptStruct->GetCppStructOps()->NetSerialize(Ar, Map, bOutSuccess, Data.Get());
		}
		else
		{
			// This won't work since FStructProperty::NetSerializeItem is deprecrated.
			//	1) we have to manually crawl through the topmost struct's fields since we don't have a FStructProperty for it (just the UScriptProperty)
			//	2) if there are any UStructProperties in the topmost struct's fields, we will assert in FStructProperty::NetSerializeItem.

			UE_LOG(LogAlive, Warning,
			       TEXT("FEquipmentPerformanceHandle::NetSerialize called on data struct %s without a native NetSerialize"),
			       *ScriptStruct->GetName());

			for (TFieldIterator<FProperty> It(ScriptStruct.Get()); It; ++It)
			{
				if (It->PropertyFlags & CPF_RepSkip)
				{
					continue;
				}

				void* PropertyData = It->ContainerPtrToValuePtr<void*>(ContainerPtr);

				It->NetSerializeItem(Ar, Map, PropertyData);
			}
		}
	}
	else if (ScriptStruct.IsError())
	{
		UE_LOG(LogAlive, Warning, TEXT("FEquipmentPerformanceHandle::NetSerialize: Bad ScriptStruct serialized, can't recover."));
		Ar.SetError();
		bOutSuccess = false;
		return false;
	}

	bOutSuccess = true;
	return true;
}

FNetSerializeScriptStructCache FEquipmentPerformanceHandle::GetEquipmentPerformanceStructCache() const
{
	FNetSerializeScriptStructCache EquipmentPerformanceStructCache;
	if (EquipmentPerformanceStructCache.ScriptStructs.Num() == 0)
	{
		//EquipmentPerformanceStructCache.InitForType(FEquipmentPerformance::StaticStruct());
		check(EquipmentPerformanceStructCache.ScriptStructs.Num() != 0);
	}
	return EquipmentPerformanceStructCache;
}
//
// void FEquipmentPerformanceHandle::InitScriptStructs()
// {
// 	if (ScriptStructs.Num() != 0)
// 	{
// 		return;
// 	}
//
// 	// Find all script structs of this type and add them to the list
// 	// (not sure of a better way to do this but it should only happen once at startup)
// 	for (TObjectIterator<UScriptStruct> It; It; ++It)
// 	{
// 		if (It->IsChildOf(FEquipmentPerformance::StaticStruct()))
// 		{
// 			ScriptStructs.Add(*It);
// 		}
// 	}
//
// 	ScriptStructs.Sort([](const UScriptStruct& A, const UScriptStruct& B) { return A.GetName().ToLower() > B.GetName().ToLower(); });
// 	check(ScriptStructs.Num()!=0);
// }
