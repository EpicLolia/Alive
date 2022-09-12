// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerInventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/AliveWeapon.h"
#include "AliveCharacter.h"
#include "Player/AlivePlayerController.h"

UPlayerInventoryComponent::UPlayerInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// ...
}

void UPlayerInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UPlayerInventoryComponent, CurrentWeapon, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(UPlayerInventoryComponent, WeaponInventory, COND_OwnerOnly);
}

void UPlayerInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerInventoryComponent::AddWeapon(AAliveWeapon* Weapon)
{
	check(GetOwner()->HasAuthority());

	const TArray<AAliveWeapon*> PreInventory = WeaponInventory;
	Weapon->SetOwningCharacter(Cast<AAliveCharacter>(GetOwner()));
	WeaponInventory.Add(Weapon);

	if (Cast<AController>(GetOwner()->GetOwner())->IsLocalController())
	{
		OnRep_WeaponInventory(PreInventory);
	}
}

void UPlayerInventoryComponent::ChangeWeaponAndRequestServer(AAliveWeapon* Weapon)
{
	const AAliveWeapon* PreviousWeapon = CurrentWeapon;
	CurrentWeapon = Weapon;
	OnRep_CurrentWeapon(PreviousWeapon);

	// Cancel the Abilities of previous weapon. Will not remove the projectile that is already fired. 
	if (PreviousWeapon)
	{
		for (const auto& ability : PreviousWeapon->GetWeaponAbilitySpecHandles())
		{
			Cast<AAliveCharacter>(GetOwner())->GetAbilitySystemComponent()->CancelAbilityHandle(ability);
		}
	}

	if (!GetOwner()->HasAuthority())
	{
		ServerChangeWeapon(Weapon);
	}
}

bool UPlayerInventoryComponent::HasSameWeapon(const AAliveWeapon* Weapon) const
{
	for (const auto& MyWeapon : WeaponInventory)
	{
		if (Weapon->GetClass() == MyWeapon->GetClass())
		{
			return true;
		}
	}
	return false;
}

bool UPlayerInventoryComponent::IsTheOwner(const AAliveWeapon* Weapon) const
{
	return Weapon->GetOwningCharacter() && Cast<AActor>(Weapon->GetOwningCharacter()) == GetOwner();
}

void UPlayerInventoryComponent::OnRep_WeaponInventory(const TArray<AAliveWeapon*>& PreviousWeaponInventory)
{
	if (Cast<AAlivePlayerController>(GetOwner()->GetOwner()))
	{
		// Will not broadcast for AI
		OnMyWeaponInventoryChanged.Broadcast();
	}
	
	if (PreviousWeaponInventory.Num() == 0 && WeaponInventory.Last())
	{
		// Equip the first weapon you picked up automatically. 
		ChangeWeaponAndRequestServer(WeaponInventory.Last());
	}
}

void UPlayerInventoryComponent::OnRep_CurrentWeapon(const AAliveWeapon* PreviousWeapon)
{
	if (PreviousWeapon)
	{
		PreviousWeapon->SetWeaponVisibility(false);
	}
	if (CurrentWeapon)
	{
		CurrentWeapon->SetWeaponVisibility(true);
	}
	OnWeaponChanged.Broadcast(CurrentWeapon);
}

void UPlayerInventoryComponent::ServerChangeWeapon_Implementation(AAliveWeapon* Weapon)
{
	const AAliveWeapon* PreviousWeapon = CurrentWeapon;
	CurrentWeapon = Weapon;
	OnRep_CurrentWeapon(PreviousWeapon);
}
