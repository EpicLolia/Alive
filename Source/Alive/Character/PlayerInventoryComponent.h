// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInventoryComponent.generated.h"

class AAliveWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponChangedDelegate, AAliveWeapon*, Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWeaponInventoryChangedDelegate);

UCLASS(ClassGroup=(Alive), hidecategories=(Object,LOD,Lighting,Transform,Sockets,TextureStreaming))
class ALIVE_API UPlayerInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerInventoryComponent();
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FORCEINLINE AAliveWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	void AddWeapon(AAliveWeapon* Weapon);
	UFUNCTION(BlueprintCallable)
	void ChangeWeaponAndRequestServer(AAliveWeapon* Weapon);

	/*
	void DiscardWeapon(AAliveWeapon* Weapon);
	// Called when character dies.
	void DiscardAllWeapons();
	*/

	// Check before pick up
	bool HasSameWeapon(const AAliveWeapon* Weapon) const;
	bool IsTheOwner(const AAliveWeapon* Weapon)const;

protected:
	// Can be used to Change AnimLayer or UI 
	UPROPERTY(BlueprintAssignable, Category = "Alive|Character")
	FWeaponChangedDelegate OnWeaponChanged;

	// A locally delegate. Only called on actor's owner. Used to show some UI Tips.
	UPROPERTY(BlueprintAssignable, Category = "Alive|Character")
	FWeaponInventoryChangedDelegate OnMyWeaponInventoryChanged;
private:
	// Replicate to owner only. Just a list tells you which weapon actor is yours.
	UPROPERTY(ReplicatedUsing = OnRep_WeaponInventory)
	TArray<AAliveWeapon*> WeaponInventory;
	// Will be Called on the owner only. Tells you that your inventory has some change.
	UFUNCTION()
	void OnRep_WeaponInventory(const TArray<AAliveWeapon*>& PreviousWeaponInventory);
	
	// Replicated Simulated Only
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	AAliveWeapon* CurrentWeapon;
	// This will eventually be executed on all clients. Including server.
	UFUNCTION()
	void OnRep_CurrentWeapon(const AAliveWeapon* PreviousWeapon);

	UFUNCTION(Server, Reliable)
	void ServerChangeWeapon(AAliveWeapon* Weapon);
	void ServerChangeWeapon_Implementation(AAliveWeapon* Weapon);
};

