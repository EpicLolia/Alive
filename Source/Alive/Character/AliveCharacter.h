// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

#include "AliveCharacter.generated.h"

class UCameraComponent;
class UAliveAbilitySystemComponent;

UCLASS(config=Game,Abstract)
class AAliveCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAliveCharacter();

protected:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Alive|Character", meta = (AllowPrivateAccess = "true"))
	UAliveAbilitySystemComponent* AbilitySystemComponent;

protected:
	//Default abilities for this Character. These will be removed on Character death and regiven if Character respawns. 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Alive|Character")
	TArray<TSubclassOf<class UAliveGameplayAbility>> CharacterAbilities;

	// 只能在服务器添加和移除ability
	virtual void AddCharacterAbilities();
	void RemoveCharacterAbilities();
};
