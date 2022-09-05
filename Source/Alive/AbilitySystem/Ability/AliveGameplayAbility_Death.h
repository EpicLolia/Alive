// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveGameplayAbility.h"
#include "AliveGameplayAbility_Death.generated.h"

/**
 * UAliveGameplayAbility_Death
 *
 * Gameplay ability used for handling death.
 * Ability is activated automatically via the "GameplayEvent.Death" ability trigger tag.
 */
UCLASS()
class ALIVE_API UAliveGameplayAbility_Death : public UAliveGameplayAbility
{
	GENERATED_BODY()

public:
	UAliveGameplayAbility_Death();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
private:
	// Set Default Tag
	void DoneAddingNativeTags();

	// Starts the death sequence.
	void StartDeath();

	// Finishes the death sequence.
	void FinishDeath();
};
