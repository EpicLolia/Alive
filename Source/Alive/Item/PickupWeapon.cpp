// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupWeapon.h"

#include "Character/AliveCharacter.h"
#include "Components/SphereComponent.h"
#include "Weapon/AliveWeapon.h"


APickupWeapon::APickupWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APickupWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (Weapon)
	{
		Weapon->AttachToComponent(CollisionComp, FAttachmentTransformRules::KeepRelativeTransform);
		Weapon->SetActorRelativeTransform(FTransform());
	}
}

bool APickupWeapon::CanBePickedUp(const AAliveCharacter* Character) const
{
	return Super::CanBePickedUp(Character) && true;
}

void APickupWeapon::GivePickupTo(AAliveCharacter* Character)
{
	Super::GivePickupTo(Character);

	if (Weapon)
	{
		Weapon->SetOwningCharacter(Character);
		if (!Character->GetCurrentWeapon())
		{
			Character->SetCurrentWeapon(Weapon);
		}
	}
}
