// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveTypes.h"
#include "Pickup.h"
#include "Weapon/WeaponType.h"
#include "PickupWeapon.generated.h"

UCLASS()
class ALIVE_API APickupWeapon : public APickup
{
	GENERATED_BODY()

public:
	APickupWeapon();

	void InitWeaponPickup(class AWeapon* InitWeapon);
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	virtual bool CanPickUp(const AAliveCharacter* Character) const override;
	virtual void GivePickupTo(AAliveCharacter* Character) override;


	UPROPERTY(EditDefaultsOnly, Category = "Alive|Pickup")
	TSubclassOf<UWeaponType> WeaponType;

	// Only valid on the server. 
	UPROPERTY(BlueprintReadOnly, Category = "Alive|Pickup")
	class AWeapon* Weapon;

private:
	UPROPERTY(BlueprintReadOnly, Category = "WeaponMesh", meta=(AllowPrivateAccess = true))
	USkeletalMeshComponent* WeaponMesh;

	void UpdateWeaponTransformAndVelocity();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTransformWithVelocity)
	FTransformWithVelocity CurrentTransformWithVelocity;
	UFUNCTION()
	void OnRep_CurrentTransformWithVelocity();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartSimulatePhysics(const UWeaponType* PickupWeaponType);
	void MulticastStartSimulatePhysics_Implementation(const UWeaponType* PickupWeaponType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopSimulatePhysics(FTransformWithVelocity TransformWithVelocity);
	void MulticastStopSimulatePhysics_Implementation(FTransformWithVelocity TransformWithVelocity);

	bool bIsSimulatePhysics;

	FTimerHandle UpdateTransformTimerHandle;
};
