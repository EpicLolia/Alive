// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

#include "AliveCharacter.generated.h"

class AAliveWeapon;
class UCameraComponent;
class UAliveAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponDelegate, AAliveWeapon*, Weapon);

UCLASS(config=Game, Abstract)
class AAliveCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAliveCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Only add or remove ability on server, this function will be called in derived class
	virtual void AddCharacterAbilities();
	void RemoveCharacterAbilities();
	void AddCharacterEffects();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alive|AbilitySystem")
	UAliveAbilitySystemComponent* AbilitySystemComponent;
	
	// Default abilities for this Character. These will be removed on Character death and regiven if Character respawns. 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Alive|AbilitySystem")
	TArray<TSubclassOf<class UAliveGameplayAbility>> CharacterAbilities;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Alive|AbilitySystem")
	TArray<TSubclassOf<class UGameplayEffect>> CharacterEffects;

public:
	AAliveWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	void AddWeaponToInventory(AAliveWeapon* Weapon);
	
	FName GetWeaponSocket() const { return WeaponSocket; }

	// Can be used to Change AnimLayer or UI 
	UPROPERTY(BlueprintAssignable, Category = "Alive|Character")
	FWeaponDelegate OnWeaponChanged;

	// A locally delegate. Only called on actor's owner. Used to show some UI Tips.
	UPROPERTY(BlueprintAssignable, Category = "Alive|Character")
	FWeaponDelegate OnWeaponAddedToMyInventory;
	
protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Weapon")
	FName WeaponSocket;

private:
	// Replicated Simulated Only
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	AAliveWeapon* CurrentWeapon;
	// This will eventually be executed on all clients. Including server.
	UFUNCTION()
	void OnRep_CurrentWeapon(const AAliveWeapon* PreviousWeapon);

	void ChangeWeaponAndRequestServer(AAliveWeapon* Weapon);
	UFUNCTION(Server,Reliable)
	void ServerChangeWeapon(AAliveWeapon* Weapon);
	void ServerChangeWeapon_Implementation(AAliveWeapon* Weapon);

	UPROPERTY(ReplicatedUsing = OnRep_WeaponInventory)
	TArray<AAliveWeapon*> WeaponInventory;
	// This will eventually be executed on all clients. Including server.
	UFUNCTION()
	void OnRep_WeaponInventory();

	// Used for reconnection after disconnection
	void AdjustWeaponsVisibility();
};

