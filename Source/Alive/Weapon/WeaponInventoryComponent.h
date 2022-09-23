// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "WeaponSpec.h"
#include "Components/ActorComponent.h"
#include "WeaponInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAmmoChangedDelegate, int32, CurrentAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponPerformanceChangedDelegate, FWeaponPerformance, CurrentWeaponPerformance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWeaponSpecInventoryChangedDelegate);

/** Used to show the weapon in blueprint. */
USTRUCT(BlueprintType)
struct FWeaponView
{
	GENERATED_BODY()
	FWeaponView() { return; }

	FWeaponView(const UWeaponType* InWeaponType, FWeaponSpecHandle InWeaponHandle)
		: WeaponType(InWeaponType), WeaponHandle(InWeaponHandle)
	{
	}

	/** Always the ClassDefaultObject*/
	UPROPERTY(BlueprintReadOnly)
	const UWeaponType* WeaponType;

	UPROPERTY(BlueprintReadOnly)
	FWeaponSpecHandle WeaponHandle;
};

UCLASS(ClassGroup=(Weapon), meta=(BlueprintSpawnableComponent))
class ALIVE_API UWeaponInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponInventoryComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	bool HasSameType(const UWeaponType* Weapon)const;
	/** Only work on the server. */
	UFUNCTION(BlueprintCallable)
	void AddWeaponToInventory(const FWeaponSpec& WeaponSpec);

	UFUNCTION(BlueprintCallable)
	void ChangeWeaponAndCallServer(const FWeaponSpecHandle& WeaponSpecHandle);
	UFUNCTION(BlueprintCallable)
	void RemoveWeaponFromInventoryAndCallServer(const FWeaponSpecHandle& WeaponSpecHandle);
	
	UFUNCTION(BlueprintCallable)
	bool CheckCurrentWeaponCost() const;
	UFUNCTION(BlueprintCallable)
	void ApplyCurrentWeaponCost();
	
	UPROPERTY(BlueprintAssignable)
	FAmmoChangedDelegate OnCurrentAmmoChanged;
	UPROPERTY(BlueprintAssignable)
	FWeaponPerformanceChangedDelegate OnCurrentWeaponPerformanceChanged;

	/** Only called on the owner. */
	UPROPERTY(BlueprintAssignable)
	FWeaponSpecInventoryChangedDelegate OnWeaponInventoryAdd;
	/** Only called on the owner. Replicated from server, there may be some latency. */
	UPROPERTY(BlueprintAssignable)
	FWeaponSpecInventoryChangedDelegate OnWeaponInventoryRemove;
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	TArray<FWeaponView> GetAllWeapons() const;

	/** Will be replicated on the simulated actor. Server and client should call UpdateWeaponPerformance by themselves. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = "UpdateWeaponPerformance")
	FWeaponPerformance CurrentWeaponPerformance;

private:
	UFUNCTION()
	void UpdateWeaponPerformance();

	const FWeaponPerformance GenerateWeaponPerformance(const UWeaponType* WeaponType)const;
	
	UFUNCTION(Server,Reliable)
	void ServerRemoveWeaponFromInventory(const FWeaponSpecHandle WeaponSpecHandle);
	UFUNCTION(Server,Reliable)
	void ServerChangeWeapon(const FWeaponSpecHandle WeaponSpecHandle);
	void ChangeWeapon(const FWeaponSpecHandle& WeaponSpecHandle);
	
	/** Caution! The return ptr may be invalid if the inventory is changed. */
	FWeaponSpec* GetWeaponSpecFromHandle(const FWeaponSpecHandle& WeaponHandle);
	const FWeaponSpec* GetWeaponSpecFromHandle(const FWeaponSpecHandle& WeaponHandle) const;
	
	/** Only replicated between server and weapon's owner. */
	UPROPERTY(Replicated)
	FWeaponSpecContainer WeaponInventory;

	/** Only replicated between server and weapon's owner. */
	UPROPERTY(Replicated)
	FWeaponSpecHandle CurrentWeapon;

	//FTimerHandle AmmoChangeTimerHandle;
};
