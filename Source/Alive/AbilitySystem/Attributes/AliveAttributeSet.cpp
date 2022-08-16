// Fill out your copyright notice in the Description page of Project Settings.

#include "AliveAttributeSet.h"
#include "AbilitySystem/AliveAbilitySystemComponent.h"

UAliveAttributeSet::UAliveAttributeSet()
{
}

UWorld* UAliveAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);
	
	return Outer->GetWorld();
}

UAliveAbilitySystemComponent* UAliveAttributeSet::GetAliveAbilitySystemComponent() const
{
	return Cast<UAliveAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
