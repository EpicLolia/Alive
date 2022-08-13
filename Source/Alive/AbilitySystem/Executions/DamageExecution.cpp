// Copyright Epic Games, Inc. All Rights Reserved.

#include "DamageExecution.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/Attributes/HealthSet.h"
#include "AbilitySystem/Attributes/CombatSet.h"

struct FDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition AttackDef;

	FDamageStatics()
	{
		AttackDef = FGameplayEffectAttributeCaptureDefinition(UCombatSet::GetAttackAttribute(),
		                                                      EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}


UDamageExecution::UDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackDef);
}

void UDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float Attack = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackDef, EvaluateParameters, Attack);
	Attack = FMath::Max<float>(Attack, 0.0f);

	const float DamageDone = Attack;
	if (DamageDone > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(UHealthSet::GetDamageAttribute(), EGameplayModOp::Additive,
			                               DamageDone));
	}
#endif // #if WITH_SERVER_CODE
}
