// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Character/AliveCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Player/AlivePlayerState.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	// Caution! Only replicated to owner. 
	//bOnlyRelevantToOwner = true;
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, WeaponType, COND_InitialOnly)
	DOREPLIFETIME_CONDITION(AWeapon, CurrentAmmo, COND_OwnerOnly)
}

void AWeapon::AddTo(AAliveCharacter* Character)
{
	// Will not add the actor to the weapon array here,
	// because the owner may not have the inventory component, like monster.
	check(Character);
	SetOwner(Character);
	GrantAbilitiesToOwner();
}

void AWeapon::DiscardFromOwner()
{
	check(GetOwner());
	RemoveAbilitiesFromOwner();
}

FWeaponPerformance AWeapon::GenerateWeaponPerformance() const
{
	if (GetOwnerAsAliveCharacter())
	{
		if (AAlivePlayerState* PS = Cast<AAlivePlayerState>(GetOwnerAsAliveCharacter()->GetPlayerState()))
		{
			// TODO: Custom weapon performance
			return FWeaponPerformance(WeaponType);
		}
	}
	return FWeaponPerformance(WeaponType);
}

void AWeapon::SetCurrentAmmo(int32 Ammo)
{
	CurrentAmmo = FMath::Clamp(Ammo, 0, WeaponType->MaxClipAmmo);
	OnCurrentAmmoChanged.ExecuteIfBound();
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::SetWeaponType(TSubclassOf<UWeaponType> WeaponTypeClass)
{
	WeaponType = WeaponTypeClass.GetDefaultObject();
	CurrentAmmo = WeaponType->MaxClipAmmo;
}

void AWeapon::OnRep_CurrentAmmo(int32 OldAmmo)
{
	if (OldAmmo != CurrentAmmo)
	{
		OnCurrentAmmoChanged.ExecuteIfBound();
	}
}

AAliveCharacter* AWeapon::GetOwnerAsAliveCharacter() const
{
	return Cast<AAliveCharacter>(GetOwner());
}

void AWeapon::GrantAbilitiesToOwner()
{
	check(HasAuthority());
	check(GetOwnerAsAliveCharacter());
	check(AbilitySpecHandles.Num() == 0);

	for (const TSubclassOf<UGameplayAbility>& Ability : WeaponType->AbilitiesGrantedToOwner)
	{
		AbilitySpecHandles.Add(GetOwnerAsAliveCharacter()->GetAbilitySystemComponent()->GiveAbility(
			FGameplayAbilitySpec(Ability)));
	}
}

void AWeapon::RemoveAbilitiesFromOwner()
{
	check(HasAuthority());
	check(GetOwnerAsAliveCharacter());

	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
	{
		GetOwnerAsAliveCharacter()->GetAbilitySystemComponent()->ClearAbility(SpecHandle);
	}
	AbilitySpecHandles.Reset();
}
