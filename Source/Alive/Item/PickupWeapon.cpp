// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupWeapon.h"

#include "Character/AliveCharacter.h"
#include "Weapon/AliveWeapon.h"



APickupWeapon::APickupWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APickupWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (Weapon)
		{
			if (Weapon->GetOwningCharacter())
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
											 TEXT("APickupWeapon: Weapon already has owner!"));
				Weapon->RemoveFormOwningCharacter();
			}
			Weapon->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
			Weapon->SetActorRelativeTransform(FTransform());
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			                                 TEXT("APickupWeapon: Do not have weapon!"));
			Destroy();
		}
	}
}

bool APickupWeapon::CanPickUp(const AAliveCharacter* Character) const
{
	// TODO: Limit on the number of weapons
	return Super::CanPickUp(Character) && Weapon;
}

void APickupWeapon::GivePickupTo(AAliveCharacter* Character)
{
	Super::GivePickupTo(Character);

	if (Weapon)
	{
		Weapon->SetOwningCharacter(Character);
	}
	Weapon = nullptr;
}
