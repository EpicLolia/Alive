// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Executions/CooldownModifier.h"

#include "AbilitySystem/Ability/AliveGameplayAbility.h"
#include "AbilitySystem/Attributes/CombatSet.h"


struct FCooldownStatics
{
	FGameplayEffectAttributeCaptureDefinition AttackSpeedDef;

	FCooldownStatics()
	{
		// TODO: Attack Speed
		AttackSpeedDef = FGameplayEffectAttributeCaptureDefinition(UCombatSet::GetAttackAttribute(),
															  EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FCooldownStatics& CooldownStatics()
{
	static FCooldownStatics Statics;
	return Statics;
}

UCooldownModifier::UCooldownModifier()
{
	RelevantAttributesToCapture.Add(CooldownStatics().AttackSpeedDef);
}

float UCooldownModifier::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Gather the tags from the source as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;

	const UAliveGameplayAbility* Ability = Cast<UAliveGameplayAbility>(
		Spec.GetContext().GetAbilityInstance_NotReplicated());

	if (!Ability)
	{
		return 0.0f;
	}

	return Ability->CooldownDuration.GetValueAtLevel(Ability->GetAbilityLevel());
}
