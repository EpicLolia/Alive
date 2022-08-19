// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AliveAnimInstance.h"

#include "AbilitySystemGlobals.h"

UAliveAnimInstance::UAliveAnimInstance()
{
}

void UAliveAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(!bHasInitializeWithAbilitySystem)
	{
		if (AActor* OwningActor = GetOwningActor())
		{
			if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
			{
				InitializeWithAbilitySystem(ASC);
				bHasInitializeWithAbilitySystem = true;
			}
		}
	}
}

void UAliveAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	GameplayTagPropertyMap.Initialize(this, ASC);
}
