#include "AliveAbilitySystemComponent.h"

UAliveAbilitySystemComponent::UAliveAbilitySystemComponent()
	:bHasCharacterAbilities(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	// AI's RepMode uses Minimal. Player's RepMode uses Mixed.
	ReplicationMode = EGameplayEffectReplicationMode::Minimal;
}
