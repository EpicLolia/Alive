// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "WeaponType.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class UWeaponType;
class AAliveCharacter;

/**
 * AWeapon
 */
UCLASS(BlueprintType)
class ALIVE_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

	void SetWeaponType(TSubclassOf<UWeaponType> WeaponTypeClass);
	
	void AddTo(AAliveCharacter* Character);
	void DiscardFromOwner();

	FWeaponPerformance GenerateWeaponPerformance()const;
	
	/** Ability can change ammo by this. */
	UFUNCTION(BlueprintCallable)
	void SetCurrentAmmo(int32 Ammo);
	
	/** Can be bound in the inventory component. */
	FSimpleDelegate OnCurrentAmmoChanged;
	
protected:
	/** Always the ClassDefaultObject */
	UPROPERTY(Replicated, BlueprintReadOnly)
	const UWeaponType* WeaponType;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo, BlueprintReadOnly)
	int32 CurrentAmmo;
	UFUNCTION()
	void OnRep_CurrentAmmo(int32 OldAmmo);
	
private:
	// Record the skills given by this weapon. Only useful on the server because we should only add/remove with authority.
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	AAliveCharacter* GetOwnerAsAliveCharacter() const;

	void GrantAbilitiesToOwner();
	void RemoveAbilitiesFromOwner();
};

