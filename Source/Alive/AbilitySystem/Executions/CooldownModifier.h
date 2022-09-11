// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "CooldownModifier.generated.h"

/**
 * 
 */
UCLASS()
class ALIVE_API UCooldownModifier : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
		UCooldownModifier();
	
protected:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
