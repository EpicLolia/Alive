﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickup.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Character/AliveCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/AliveWeapon.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Collision
	CollisionComp = CreateDefaultSubobject<USphereComponent>(FName("CollisionComponent"));
	CollisionComp->InitSphereRadius(40.0f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic); // TODO
	RootComponent = CollisionComp;
}

void APickup::BeginPlay()
{
	Super::BeginPlay();
}

void APickup::TryToPickItUp(AAliveCharacter* Character)
{
	check(HasAuthority());

	if (!CanPickUp(Character))
	{
		return;
	}

	bHasBeenTriggered = true;
	
	GivePickupTo(Character);

	NetMulticast_PickUpEvent();

	Destroy();
}


void APickup::GivePickupTo(AAliveCharacter* Character)
{
	check(HasAuthority());
	
	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		for (TSubclassOf<UGameplayEffect> EffectClass : EffectClasses)
		{
			if (!EffectClass)
			{
				continue;
			}
			FGameplayEffectSpecHandle NewHandle = ASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
			if (NewHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
			}
		}
	}
}

void APickup::NetMulticast_PickUpEvent_Implementation()
{
	OnPickUpEvent();
}

bool APickup::CanPickUp(const AAliveCharacter* Character) const
{
	return IsValid(Character) && !bHasBeenTriggered;
}
