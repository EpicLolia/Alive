// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlivePlayerState.h"
#include "AlivePlayerController.h"
#include "AbilitySystem/AliveAbilitySystemComponent.h"
// Attribute Set
#include "AbilitySystem/Attributes/AmmoSet.h"
#include "AbilitySystem/Attributes/HealthSet.h"
#include "AbilitySystem/Attributes/CombatSet.h"
#include "Net/UnrealNetwork.h"

//DEFINE_LOG_CATEGORY_STATIC(LogPlayerState, Log, All);

AAlivePlayerState::AAlivePlayerState()
	: KillCount(0)
	  , DeathCount(0)
	  , TeamNumber(0)
	  , NumBulletsFired()
	  , NumRocketsFired()
{
	CharacterType = APlayerCharacter::StaticClass();
	
	AbilitySystemComponent = CreateDefaultSubobject<UAliveAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Mixed Mode will replicated GameplayEffect to owner (Autonomous).
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Creating AttributeSet in the constructor will be automatically registered to ASC.
	CreateDefaultSubobject<UHealthSet>(TEXT("HealthSet"));
	CreateDefaultSubobject<UCombatSet>(TEXT("CombatSet"));
	CreateDefaultSubobject<UAmmoSet>(TEXT("AmmoSet"));

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

void AAlivePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAlivePlayerState, TeamNumber, COND_None);
	DOREPLIFETIME_CONDITION(AAlivePlayerState, KillCount, COND_None);
	DOREPLIFETIME_CONDITION(AAlivePlayerState, DeathCount, COND_None);
}

void AAlivePlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
}
