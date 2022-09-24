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

	AWeapon();
public:
	UFUNCTION(BlueprintCallable, meta= (DefaultToSelf = GenerateInstigator))
	static AWeapon* NewWeapon(const AActor* GenerateInstigator, TSubclassOf<UWeaponType> WeaponTypeClass, const FTransform& Transform);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

	const UWeaponType* GetWeaponType() const { return WeaponType; }

	// Only called on the server.
	void AddTo(AAliveCharacter* Character);
	void DiscardFromOwner();

	UFUNCTION(BlueprintCallable)
	FVector GetFirePointWorldLocation() const;

	class UProjectileComponent* GetProjectileComponent() const { return ProjectileComponent; }

	FWeaponPerformance GenerateWeaponPerformance() const;

	int32 GetCurrentAmmo() const { return CurrentAmmo; }
	/** Ability can change ammo by this. */
	UFUNCTION(BlueprintCallable)
	void SetCurrentAmmo(int32 Ammo);

	/** Can be bound in the inventory component. Useful for the owner. */
	FSimpleDelegate OnCurrentAmmoChanged;

protected:
	/** Always the ClassDefaultObject */
	UPROPERTY(ReplicatedUsing = OnRep_WeaponType, BlueprintReadOnly)
	const UWeaponType* WeaponType;
	// On weapon type ready.
	UFUNCTION()
	void OnRep_WeaponType();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo, BlueprintReadOnly)
	int32 CurrentAmmo;
	UFUNCTION()
	void OnRep_CurrentAmmo(int32 OldAmmo);

private:
	UPROPERTY()
	class UProjectileComponent* ProjectileComponent;

	// Record the skills given by this weapon. Only useful on the server because we should only add/remove with authority.
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	AAliveCharacter* GetOwnerAsAliveCharacter() const;

	void GrantAbilitiesToOwner();
	void RemoveAbilitiesFromOwner();

	void SetWeaponType(TSubclassOf<UWeaponType> WeaponTypeClass);
};
