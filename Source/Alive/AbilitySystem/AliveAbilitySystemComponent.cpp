#include "AliveAbilitySystemComponent.h"

UAliveAbilitySystemComponent::UAliveAbilitySystemComponent()
	:bHasDefaultAbilities(false)
{
	PrimaryComponentTick.bCanEverTick = true;
}
