// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AliveAbilitySystemComponent.generated.h"

/**
 *
 */
UCLASS()
class ALIVE_API UAliveAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UAliveAbilitySystemComponent();

	bool bHasCharacterAbilities;
};
