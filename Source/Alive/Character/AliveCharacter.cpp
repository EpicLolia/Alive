// Copyright Epic Games, Inc. All Rights Reserved.

#include "AliveCharacter.h"

#include "AbilitySystem/Ability/AliveGameplayAbility.h"
#include "AbilitySystem/AliveAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/AliveWeapon.h"

//DEFINE_LOG_CATEGORY_STATIC(LogAliveChar, Warning, All);

AAliveCharacter::AAliveCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
}

UAbilitySystemComponent* AAliveCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAliveCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAliveCharacter, CurrentWeapon, COND_None);
	DOREPLIFETIME_CONDITION(AAliveCharacter, WeaponInventory, COND_OwnerOnly);
}

void AAliveCharacter::AddWeaponToInventory(AAliveWeapon* Weapon)
{
	WeaponInventory.Add(Weapon);
	OnRep_WeaponInventory();
}

void AAliveCharacter::OnRep_CurrentWeapon(const AAliveWeapon* PreviousWeapon)
{
	// Local player has changed his weapon
	if (!IsLocallyControlled())
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
}

void AAliveCharacter::ChangeWeaponAndRequestServer(AAliveWeapon* Weapon)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->SetWeaponVisibility(false);
	}
	CurrentWeapon = Weapon;
	if (CurrentWeapon)
	{
		CurrentWeapon->SetWeaponVisibility(true);
	}
	
	OnWeaponChanged.Broadcast(CurrentWeapon);

	if(!HasAuthority())
	{
		ServerChangeWeapon(Weapon);
	}
}

void AAliveCharacter::ServerChangeWeapon_Implementation(AAliveWeapon* Weapon)
{
	AAliveWeapon* PreviousWeapon = CurrentWeapon;
	CurrentWeapon = Weapon;
	OnRep_CurrentWeapon(PreviousWeapon);
}

void AAliveCharacter::AddCharacterAbilities()
{
	if (AbilitySystemComponent && !AbilitySystemComponent->bHasCharacterAbilities)
	{
		for (TSubclassOf<UAliveGameplayAbility>& Ability : CharacterAbilities)
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(Ability, 1, INDEX_NONE, this)); // 等级默认为1级
		}
		AbilitySystemComponent->bHasCharacterAbilities = true;
	}
}

void AAliveCharacter::RemoveCharacterAbilities()
{
	// Remove any abilities added from a previous call.
	// This checks to make sure the ability is in the startup 'CharacterAbilities' array.
	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if ((Spec.SourceObject == this) && CharacterAbilities.Contains(Spec.Ability->GetClass()))
		{
			AbilitiesToRemove.Add(Spec.Handle);
		}
	}
	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitiesToRemove)
	{
		AbilitySystemComponent->ClearAbility(SpecHandle);
	}

	AbilitySystemComponent->bHasCharacterAbilities = false;
}

void AAliveCharacter::AddCharacterEffects()
{
	if (AbilitySystemComponent)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		for (TSubclassOf<UGameplayEffect> EffectClass : CharacterEffects)
		{
			if (!EffectClass)
			{
				continue;
			}
			FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(
				EffectClass, 1, EffectContext);
			if (NewHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
			}
		}
	}
}

void AAliveCharacter::OnRep_WeaponInventory()
{
	if(IsLocallyControlled())
	{
		if (WeaponInventory.Num())
		{
			OnWeaponAdded.Broadcast(WeaponInventory.Last());
		}
		// The first weapon you get
		if (WeaponInventory.Num() == 1)
		{
			ChangeWeaponAndRequestServer(WeaponInventory.Last());
		}
	}
}
