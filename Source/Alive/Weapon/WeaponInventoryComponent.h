// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponType.h"
#include "Components/ActorComponent.h"
#include "WeaponInventoryComponent.generated.h"

class AWeapon;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAmmoChangedDelegate, int32, CurrentAmmo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponPerformanceChangedDelegate, FWeaponPerformance, CurrentWeaponPerformance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWeaponSpecInventoryChangedDelegate);

UCLASS(Blueprintable, ClassGroup=(Weapon), meta=(BlueprintSpawnableComponent))
class ALIVE_API UWeaponInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponInventoryComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	bool HasSameType(const UWeaponType* WeaponType) const;
	/** Only work on the server. */
	UFUNCTION(BlueprintCallable)
	void AddWeaponToInventory(AWeapon* Weapon);

	UFUNCTION(BlueprintCallable)
	void ChangeCurrentWeaponAndCallServer(AWeapon* Weapon = nullptr);
	UFUNCTION(BlueprintCallable)
	void RemoveWeaponFromInventoryAndCallServer(AWeapon* Weapon);

	// Called while death
	void RemoveAllWeapons();
	
	const TArray<AWeapon*>& GetWeaponInventory() const { return WeaponInventory; };
	UFUNCTION(BlueprintCallable)
	AWeapon* GetCurrentWeapon()const {return CurrentWeapon;}
	
	/** Only bound in the owner's actor. Used for UI. */
	UPROPERTY(BlueprintAssignable)
	FAmmoChangedDelegate OnCurrentAmmoChanged;
	UPROPERTY(BlueprintAssignable)
	FWeaponPerformanceChangedDelegate OnCurrentWeaponPerformanceChanged;

	/** Only called on the owner. */
	UPROPERTY(BlueprintAssignable)
	FWeaponSpecInventoryChangedDelegate OnWeaponInventoryAdd;
	/** Only called on the owner. */
	UPROPERTY(BlueprintAssignable)
	FWeaponSpecInventoryChangedDelegate OnWeaponInventoryRemove;

protected:
	/** Will be replicated on the simulated actor. Server and client should call UpdateWeaponPerformance by themselves. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = "UpdateWeaponPerformance")
	FWeaponPerformance CurrentWeaponPerformance;
	UFUNCTION()
	void UpdateWeaponPerformance();

	/** Only replicated between server and weapon's owner. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponInventory)
	TArray<AWeapon*> WeaponInventory;
	UFUNCTION()
	void OnRep_WeaponInventory(const TArray<AWeapon*>& OldInventory);

private:
	UFUNCTION(Server, Reliable)
	void ServerRemoveWeaponFromInventory(AWeapon* Weapon);
	UFUNCTION(Server, Reliable)
	void ServerChangeCurrentWeapon(AWeapon* Weapon);
	void ChangeWeapon(AWeapon* Weapon);

	/** Only replicated between server and weapon's owner. */
	UPROPERTY(Replicated)
	AWeapon* CurrentWeapon;
};
