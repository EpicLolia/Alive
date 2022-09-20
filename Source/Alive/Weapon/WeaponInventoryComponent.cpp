// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponInventoryComponent.h"

#include "Net/UnrealNetwork.h"

UWeaponInventoryComponent::UWeaponInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UWeaponInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

TArray<FWeaponView> UWeaponInventoryComponent::GetAllWeapons() const
{
	TArray<FWeaponView> WeaponViews;
	for (const auto& Weapon : WeaponInventory.Items)
	{
		WeaponViews.Emplace(Weapon.WeaponType, Weapon.GetWeaponSpecHandle());
	}
	return WeaponViews;
}

void UWeaponInventoryComponent::UpdateWeaponPerformance()
{
	OnCurrentWeaponPerformanceChanged.Broadcast(CurrentWeaponPerformance);
}

const FWeaponPerformance UWeaponInventoryComponent::GenerateWeaponPerformance(const UWeapon* WeaponType) const
{
	// TODO: Get more information from player's custom appearance settings.
	return FWeaponPerformance(WeaponType);
}

void UWeaponInventoryComponent::ServerChangeWeapon_Implementation(const FWeaponSpecHandle& WeaponSpecHandle)
{
	ChangeWeapon(WeaponSpecHandle);
}

void UWeaponInventoryComponent::ChangeWeapon(const FWeaponSpecHandle& WeaponSpecHandle)
{
	const FWeaponSpec* WeaponSpec = GetWeaponSpecFromHandle(WeaponSpecHandle);
	check(WeaponSpec);
	CurrentWeaponPerformance = GenerateWeaponPerformance(WeaponSpec->WeaponType);
	CurrentWeapon = WeaponSpecHandle;
}

FWeaponSpec* UWeaponInventoryComponent::GetWeaponSpecFromHandle(const FWeaponSpecHandle& WeaponHandle)
{
	return const_cast<FWeaponSpec*>(static_cast<const UWeaponInventoryComponent*>(this)->GetWeaponSpecFromHandle(WeaponHandle));
}

const FWeaponSpec* UWeaponInventoryComponent::GetWeaponSpecFromHandle(const FWeaponSpecHandle& WeaponHandle) const
{
	for (const FWeaponSpec& WeaponSpec : WeaponInventory.Items)
	{
		if (WeaponSpec.GetWeaponSpecHandle() == WeaponHandle)
		{
			return &WeaponSpec;
		}
	}
	return nullptr;
}

void UWeaponInventoryComponent::WeaponInventoryChanged()
{
	OnWeaponInventoryChanged.Broadcast();
}

void UWeaponInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWeaponInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UWeaponInventoryComponent, CurrentWeaponPerformance, COND_SimulatedOnly)
	DOREPLIFETIME_CONDITION(UWeaponInventoryComponent, WeaponInventory, COND_OwnerOnly)
}

bool UWeaponInventoryComponent::HasSameType(const UWeapon* Weapon) const
{
	for (const auto& WeaponSpec : WeaponInventory.Items)
	{
		if (WeaponSpec.WeaponType == Weapon)
		{
			return true;
		}
	}
	return false;
}

void UWeaponInventoryComponent::AddWeaponToInventory(const FWeaponSpec& WeaponSpec)
{
	check(GetOwner()->HasAuthority());
	check(!HasSameType(WeaponSpec.WeaponType));

	WeaponInventory.Items.Emplace(WeaponSpec);
	WeaponInventory.MarkItemDirty(WeaponInventory.Items.Last());
}

void UWeaponInventoryComponent::ChangeWeaponAndCallServer(const FWeaponSpecHandle& WeaponSpecHandle)
{
	ChangeWeapon(WeaponSpecHandle);
	if (!GetOwner()->HasAuthority())
	{
		ServerChangeWeapon(WeaponSpecHandle);
	}
}

void UWeaponInventoryComponent::RemoveWeaponFromInventoryAndCallServer(const FWeaponSpecHandle& WeaponSpecHandle)
{
	if (CurrentWeapon == WeaponSpecHandle)
	{
		if (WeaponInventory.Items.Num() != 0)
		{
			ChangeWeaponAndCallServer(WeaponInventory.Items[0].GetWeaponSpecHandle());
		}
	}
	
	if (const FWeaponSpec* WeaponSpec = GetWeaponSpecFromHandle(WeaponSpecHandle))
	{
		if (!GetOwner()->HasAuthority())
		{
			// Prediction
			WeaponInventory.Items.Remove(*WeaponSpec);
			WeaponInventory.MarkArrayDirty();
		}
		ServerRemoveWeaponFromInventory(WeaponSpecHandle);
	}
}

void UWeaponInventoryComponent::ServerRemoveWeaponFromInventory_Implementation(const FWeaponSpecHandle& WeaponSpecHandle)
{
	if (const FWeaponSpec* WeaponSpec = GetWeaponSpecFromHandle(WeaponSpecHandle))
	{
		WeaponInventory.Items.Remove(*WeaponSpec);
		WeaponInventory.MarkArrayDirty();
	}
	// TODO: Spawn pickup
}

bool UWeaponInventoryComponent::CheckCurrentWeaponCost() const
{
	if (!CurrentWeapon.IsValid())
	{
		// No cost if no weapon in hand.
		return true;
	}

	if (const FWeaponSpec* WeaponSpec = GetWeaponSpecFromHandle(CurrentWeapon))
	{
		return WeaponSpec->CurrentClipAmmo > 0;
	}

	return false;
}

void UWeaponInventoryComponent::ApplyCurrentWeaponCost()
{
	if (CurrentWeapon.IsValid())
	{
		if (FWeaponSpec* WeaponSpec = GetWeaponSpecFromHandle(CurrentWeapon))
		{
			--WeaponSpec->CurrentClipAmmo;
			//GetWorld()->GetTimerManager().SetTimer()
			WeaponInventory.MarkItemDirty(*WeaponSpec);
			OnCurrentAmmoChanged.Broadcast(WeaponSpec->CurrentClipAmmo);
		}
	}
}
