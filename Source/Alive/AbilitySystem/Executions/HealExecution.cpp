// Copyright Epic Games, Inc. All Rights Reserved.

#include "HealExecution.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/Attributes/HealthSet.h"
#include "AbilitySystem/Attributes/CombatSet.h"


struct FHealStatics
{
	FGameplayEffectAttributeCaptureDefinition AttackDef;

	FHealStatics()
	{
		AttackDef = FGameplayEffectAttributeCaptureDefinition(UCombatSet::GetAttackAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FHealStatics& HealStatics()
{
	static FHealStatics Statics;
	return Statics;
} 


UHealExecution::UHealExecution()
{
	RelevantAttributesToCapture.Add(HealStatics().AttackDef);
}

void UHealExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Attack = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealStatics().AttackDef, EvaluateParameters, Attack);
	Attack = FMath::Max<float>(Attack, 0.0f);
	
	const float HealingDone = Attack;
	if (HealingDone > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthSet::GetHealingAttribute(), EGameplayModOp::Additive, HealingDone));
	}
#endif // #if WITH_SERVER_CODE
}
