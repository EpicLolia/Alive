// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlivePlayerState.h"
#include "AlivePlayerController.h"
#include "AbilitySystem/AliveAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/HealthSet.h"
#include "AbilitySystem/Attributes/CombatSet.h"

AAlivePlayerState::AAlivePlayerState(const FObjectInitializer& ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UAliveAbilitySystemComponent>
		(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 在构造函数中创建AttributeSet将会自动注册到ASC中
	CreateDefaultSubobject<UHealthSet>(TEXT("HealthSet"));
	CreateDefaultSubobject<UCombatSet>(TEXT("CombatSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	NetUpdateFrequency = 100.0f;
}

AAlivePlayerController* AAlivePlayerState::GetAlivePlayerController() const
{
	return Cast<AAlivePlayerController>(GetOwner());
}

UAbilitySystemComponent* AAlivePlayerState::GetAbilitySystemComponent() const
{
	return GetAliveAbilitySystemComponent();
}
