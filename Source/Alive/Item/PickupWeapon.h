// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveTypes.h"
#include "Pickup.h"
#include "PickupWeapon.generated.h"

class AAliveWeapon;

UCLASS()
class ALIVE_API APickupWeapon : public APickup
{
	GENERATED_BODY()

public:
	APickupWeapon();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	
	virtual bool CanPickUp(const AAliveCharacter* Character) const override;
	virtual void GivePickupTo(AAliveCharacter* Character) override;

	UPROPERTY(EditDefaultsOnly, Category = "Alive|Pickup")
	TSubclassOf<AAliveWeapon> WeaponToSpawn;
	
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alive|Pickup")
	AAliveWeapon* Weapon;

private:
	void UpdateWeaponTransformAndVelocity();

	UPROPERTY(ReplicatedUsing=OnRep_CurrentTransformWithVelocity)
	FTransformWithVelocity CurrentTransformWithVelocity;
	UFUNCTION()
	void OnRep_CurrentTransformWithVelocity();
	
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartSimulatePhysics();
	void MulticastStartSimulatePhysics_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopSimulatePhysics(FTransformWithVelocity TransformWithVelocity);
	void MulticastStopSimulatePhysics_Implementation(FTransformWithVelocity TransformWithVelocity);

	bool bIsSimulatePhysics;
	
	FTimerHandle UpdateTransformTimerHandle;
};

