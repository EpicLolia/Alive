// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "PickupWeapon.generated.h"

UCLASS()
class ALIVE_API APickupWeapon : public APickup
{
	GENERATED_BODY()

public:
	APickupWeapon();

protected:
	virtual void BeginPlay() override;

	// Only the server holds this object
	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn), Category = "Pickup")
	class AAliveWeapon* Weapon;
	
	virtual bool CanPickUp(const AAliveCharacter* Character) const override;
	virtual void GivePickupTo(AAliveCharacter* Character) override;
};
