// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

#include "AliveCharacter.generated.h"

struct FGameplayEffectSpec;
class AAliveWeapon;
class UCameraComponent;
class UAliveAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponDelegate, AAliveWeapon*, Weapon);

/**
 * EDeathState
 *
 *	Defines current state of death.
 */
UENUM(BlueprintType)
enum class EDeathState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished
};


UCLASS(config=Game, Abstract)
class AAliveCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAliveCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Should be called by derived classes.
	void InitializeWithAbilitySystem();
	// Should be called when the actor is uninitialized from ability system.
	void UninitializeFromAbilitySystem();

private:
	UPROPERTY()
	const class UHealthSet* HealthSet;
	
protected:
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
	UFUNCTION(Server, Reliable)
	void ServerChangeWeapon(AAliveWeapon* Weapon);
	void ServerChangeWeapon_Implementation(AAliveWeapon* Weapon);

	UPROPERTY(ReplicatedUsing = OnRep_WeaponInventory)
	TArray<AAliveWeapon*> WeaponInventory;
	// This will eventually be executed on all clients. Including server.
	UFUNCTION()
	void OnRep_WeaponInventory();

	// Used for reconnection after disconnection
	void AdjustWeaponsVisibility();

public:
	EDeathState GetCurrentDeathState() const { return DeathState; }
	// Begins the death sequence for the owner.
	void StartDeath();
	// Ends the death sequence for the owner.
	void FinishDeath();
	void FinishDeathImmediately();
protected:
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnDeathStarted")
	void K2_OnDeathStarted();
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnDeathFinished")
	void K2_OnDeathFinished();
	
	// Process KDA and respawn character. 
	virtual void OnDeath(AActor* DamageInstigator){}
private:
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EDeathState DeathState;
	UFUNCTION()
	void OnRep_DeathState(EDeathState OldDeathState);

	// Called in StartDeath
	void DisableMovementAndCollision();
	// Called in FinishDeath
	void UninitializeAndDestroy();
	void UninitializeAbilitySystem();
	
	void ProcessOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);
};
