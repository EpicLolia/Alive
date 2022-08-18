// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

#include "AliveCharacter.generated.h"

class AAliveWeapon;
class UCameraComponent;
class UAliveAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponChangedDelegate, AAliveWeapon*, CurrentWeapon);

UCLASS(config=Game, Abstract)
class AAliveCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAliveCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
protected:
	// Only add or remove ability on server
	virtual void AddCharacterAbilities();
	void RemoveCharacterAbilities();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alive|Character", meta = (AllowPrivateAccess = "true"))
	UAliveAbilitySystemComponent* AbilitySystemComponent;
	
	// Default abilities for this Character. These will be removed on Character death and regiven if Character respawns. 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Alive|Character")
	TArray<TSubclassOf<class UAliveGameplayAbility>> CharacterAbilities;

public:
	AAliveWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Alive|Character") // TODO
	void SetCurrentWeapon(AAliveWeapon* Weapon);

	FName GetWeaponSocket() const { return WeaponSocket; }

	UPROPERTY(BlueprintAssignable, Category = "Alive|Character")
	FWeaponChangedDelegate OnWeaponChanged;
protected:
	UPROPERTY()
	AAliveWeapon* CurrentWeapon;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Weapon")
	FName WeaponSocket;
};
