// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AliveAttributeSet.h"
#include "NativeGameplayTags.h"
#include "CombatSet.generated.h"

/**
 * UCombatSet
 * 
*	Class that defines attributes that are necessary for applying damage or healing.
 *	Attribute examples include: damage, healing, attack power, and shield penetrations.
 */
UCLASS(BlueprintType)
class UCombatSet:public UAliveAttributeSet
{
	GENERATED_BODY()

public:
	UCombatSet();
	
	ATTRIBUTE_ACCESSORS(UCombatSet, Attack);

protected:
	UFUNCTION()
	void OnRep_Attack(const FGameplayAttributeData& OldValue);

private:
	// The Attack to apply in the damage execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Attack, Category = "Aries|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Attack;

};
