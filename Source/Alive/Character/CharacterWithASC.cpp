// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWithASC.h"

#include "AbilitySystem/AliveAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AmmoSet.h"
#include "AbilitySystem/Attributes/CombatSet.h"
#include "AbilitySystem/Attributes/HealthSet.h"

ACharacterWithASC::ACharacterWithASC()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAliveAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// Creating AttributeSet in the constructor will be automatically registered to ASC.
	CreateDefaultSubobject<UHealthSet>(TEXT("HealthSet"));
	CreateDefaultSubobject<UCombatSet>(TEXT("CombatSet"));
	CreateDefaultSubobject<UAmmoSet>(TEXT("AmmoSet"));

	// The character will play a role as the monster or something like that.
	// Do not need needs to be updated at a high frequency.
	NetUpdateFrequency = 15.0f;
}

void ACharacterWithASC::BeginPlay()
{
	Super::BeginPlay();

	// Has generated ASC in constructor.
	InitializeWithAbilitySystem();
}

void ACharacterWithASC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


