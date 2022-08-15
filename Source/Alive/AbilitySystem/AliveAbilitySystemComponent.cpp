#include "AliveAbilitySystemComponent.h"

UAliveAbilitySystemComponent::UAliveAbilitySystemComponent()
	:bHasCharacterAbilities(false)
{
	PrimaryComponentTick.bCanEverTick = true;
}
