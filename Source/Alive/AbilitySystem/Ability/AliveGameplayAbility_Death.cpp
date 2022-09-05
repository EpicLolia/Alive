// Fill out your copyright notice in the Description page of Project Settings.

#include "AliveGameplayAbility_Death.h"

#include "GameplayTagsManager.h"
#include "AbilitySystem/AliveAbilitySystemComponent.h"
#include "Character/AliveCharacter.h"

UAliveGameplayAbility_Death::UAliveGameplayAbility_Death()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	UGameplayTagsManager::Get().CallOrRegister_OnDoneAddingNativeTagsDelegate(
		FSimpleDelegate::CreateUObject(this, &ThisClass::DoneAddingNativeTags));
}

void UAliveGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                  const FGameplayAbilityActivationInfo ActivationInfo,
                                                  const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);
	UAliveAbilitySystemComponent* ASC = Cast<UAliveAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	// Cancel all abilities and block others from starting.
	ASC->CancelAbilities(nullptr,nullptr, this);
	SetCanBeCanceled(false);
	StartDeath();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAliveGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                             const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                                             bool bWasCancelled)
{
	check(ActorInfo);
	// Always try to finish the death when the ability ends in case the ability doesn't.
	// This won't do anything if the death hasn't been started.
	FinishDeath();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAliveGameplayAbility_Death::DoneAddingNativeTags()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Death"));
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UAliveGameplayAbility_Death::StartDeath()
{
	AAliveCharacter* Character = Cast<AAliveCharacter>(GetAvatarActorFromActorInfo());
	if (Character && Character->GetCurrentDeathState() == EDeathState::NotDead)
	{
		// This func may be called by the death ability or the OnRep_DeathState. Remember to check.
		Character->StartDeath();
	}
}

void UAliveGameplayAbility_Death::FinishDeath()
{
	AAliveCharacter* Character = Cast<AAliveCharacter>(GetAvatarActorFromActorInfo());
	if (Character && Character->GetCurrentDeathState() != EDeathState::DeathFinished)
	{
		// This func may be called by the death ability or the OnRep_DeathState. Remember to check.
		Character->FinishDeath();
	}
}
