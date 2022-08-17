// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

#include "AliveCharacter.generated.h"

class UCameraComponent;
class UAliveAbilitySystemComponent;
class AAliveWeapon;

UCLASS(config=Game,Abstract)
class AAliveCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAliveCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	AAliveWeapon* GetCurrentWeapon()const{ return CurrentWeapon; }

	UFUNCTION(BlueprintCallable)
	void SetCurrentWeapon(AAliveWeapon* Weapon);
	
protected:
	// Only add or remove ability on server
	virtual void AddCharacterAbilities();
	void RemoveCharacterAbilities();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alive|Character", meta = (AllowPrivateAccess = "true"))
	UAliveAbilitySystemComponent* AbilitySystemComponent;
	
	// Default abilities for this Character. These will be removed on Character death and regiven if Character respawns. 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Alive|Character")
	TArray<TSubclassOf<class UAliveGameplayAbility>> CharacterAbilities;
	
	UPROPERTY()
	AAliveWeapon* CurrentWeapon;
};
