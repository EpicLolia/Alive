// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Animation/AnimInstance.h"
#include "AliveAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ALIVE_API UAliveAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UAliveAnimInstance();
	
protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);
	
	// Gameplay tags that can be mapped to blueprint variables. The variables will automatically update as the tags are added or removed.
	// These should be used instead of manually querying for the gameplay tags.
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

private:
	bool bHasInitializeWithAbilitySystem = false;
};
