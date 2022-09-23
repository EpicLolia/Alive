// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponInventoryComponent.h"

#include "AliveLogChannels.h"
#include "Weapon.h"
#include "Character/AliveCharacter.h"
#include "Net/UnrealNetwork.h"

UWeaponInventoryComponent::UWeaponInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
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

void UWeaponInventoryComponent::UpdateWeaponPerformance()
{
	OnCurrentWeaponPerformanceChanged.Broadcast(CurrentWeaponPerformance);
}

void UWeaponInventoryComponent::ChangeCurrentWeaponAndCallServer(AWeapon* Weapon)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->OnCurrentAmmoChanged.Unbind();
	}
	
	Weapon->OnCurrentAmmoChanged.BindLambda([this]()
	{
		OnCurrentAmmoChanged.Broadcast(this->CurrentWeapon->GetCurrentAmmo());
	});

	ChangeWeapon(Weapon);
	if (!GetOwner()->HasAuthority())
	{
		ServerChangeCurrentWeapon(Weapon);
	}
}

void UWeaponInventoryComponent::ChangeWeapon(AWeapon* Weapon)
{
	CurrentWeapon = Weapon;
	if (Weapon)
	{
		check(GetOwner() == Weapon->GetOwner())
		CurrentWeaponPerformance = Weapon->GenerateWeaponPerformance();
		UpdateWeaponPerformance();
	}
	else
	{
		CurrentWeaponPerformance = FWeaponPerformance();
		UpdateWeaponPerformance();
	}
}

void UWeaponInventoryComponent::OnRep_WeaponInventory(const TArray<AWeapon*>& OldInventory)
{
	if (OldInventory.Num() < WeaponInventory.Num())
	{
		OnWeaponInventoryAdd.Broadcast();
	}
}

void UWeaponInventoryComponent::ServerChangeCurrentWeapon_Implementation(AWeapon* WeaponSpecHandle)
{
	ChangeWeapon(WeaponSpecHandle);
}

bool UWeaponInventoryComponent::HasSameType(const UWeaponType* WeaponType) const
{
	for (const auto& Weapon : WeaponInventory)
	{
		if (Weapon->GetWeaponType() == WeaponType)
		{
			return true;
		}
	}
	return false;
}

void UWeaponInventoryComponent::AddWeaponToInventory(AWeapon* Weapon)
{
	check(GetOwner()->HasAuthority());
	check(!HasSameType(Weapon->GetWeaponType()));

	WeaponInventory.Emplace(Weapon);
	Weapon->AddTo(Cast<AAliveCharacter>(GetOwner()));
	if (Cast<APawn>(GetOwner()) && Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		// If server is owner.
		OnWeaponInventoryAdd.Broadcast();
	}
}

void UWeaponInventoryComponent::RemoveWeaponFromInventoryAndCallServer(AWeapon* Weapon)
{
	check(Weapon->GetOwner() == GetOwner());
	
	if (CurrentWeapon == Weapon)
	{
		if (WeaponInventory.Num())
		{
			ChangeCurrentWeaponAndCallServer(WeaponInventory.Last());
		}
		else
		{
			ChangeCurrentWeaponAndCallServer();
		}
	}
	// Just in case.
	Weapon->OnCurrentAmmoChanged.Unbind();

	OnWeaponInventoryRemove.Broadcast();

	if(!GetOwner()->HasAuthority())
	{
		// Predicted Remove
		WeaponInventory.RemoveSingleSwap(Weapon);
	}
	
	ServerRemoveWeaponFromInventory(Weapon);
}

void UWeaponInventoryComponent::RemoveAllWeapons()
{
	for(const auto& Weapon: WeaponInventory)
	{
		Weapon->DiscardFromOwner();
		// TODO: Spawn Pickup
		Weapon->SetLifeSpan(0.1f);
	}
	WeaponInventory.Empty();
}

void UWeaponInventoryComponent::ServerRemoveWeaponFromInventory_Implementation(AWeapon* Weapon)
{
	Weapon->DiscardFromOwner();
	WeaponInventory.RemoveSingleSwap(Weapon);
	
	// TODO: Spawn Pickup
	Weapon->SetLifeSpan(0.1f);
}
