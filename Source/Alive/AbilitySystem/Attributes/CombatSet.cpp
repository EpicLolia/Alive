// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"


UCombatSet::UCombatSet()
	: Attack(100.f)
{
}

void UCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCombatSet, Attack, COND_None, REPNOTIFY_Always);
}

void UCombatSet::OnRep_Attack(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatSet, Attack, OldValue);
}
