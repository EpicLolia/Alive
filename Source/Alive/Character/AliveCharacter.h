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

	// Default abilities for this Character. These will be removed on Character death and given again when the character is respawned. 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Alive|AbilitySystem")
	TArray<TSubclassOf<class UAliveGameplayAbility>> CharacterAbilities;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Alive|AbilitySystem")
	TArray<TSubclassOf<class UGameplayEffect>> CharacterEffects;

public:
	FName GetWeaponSocket() const { return WeaponSocket; }

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Weapon")
	FName WeaponSocket;

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
	virtual void OnDeath(AActor* DamageInstigator)
	{
	}

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

	void ProcessOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec,
	                        float DamageMagnitude);
};
