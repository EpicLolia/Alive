// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "AliveWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponAmmoChangedDelegate, int32, OldValue, int32, NewValue);

class AAliveCharacter;

UCLASS()
class ALIVE_API AAliveWeapon : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()


public:
	AAliveWeapon();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
protected:
	// Pickup on touch
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;


public:
	UFUNCTION(BlueprintCallable, Category = "Alive|Weapon")
	int32 GetPrimaryClipAmmo() const { return PrimaryClipAmmo; }

	UFUNCTION(BlueprintCallable, Category = "Alive|Weapon")
	void SetPrimaryClipAmmo(int32 Ammo);
	
	UFUNCTION(BlueprintCallable, Category = "Alive|Weapon")
	int32 GetMaxPrimaryClipAmmo() const { return MaxPrimaryClipAmmo; }
	
	UPROPERTY(BlueprintAssignable, Category = "Alive|Weapon")
	FWeaponAmmoChangedDelegate OnPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Alive|Weapon")
	FWeaponAmmoChangedDelegate OnMaxPrimaryClipAmmoChanged;
protected:
	// How much ammo in the clip the gun starts with
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_PrimaryClipAmmo, Category = "Alive|Weapon|Ammo")
	int32 PrimaryClipAmmo;
	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxPrimaryClipAmmo,
		Category = "Alive|Weapon|Ammo")
	int32 MaxPrimaryClipAmmo;
	UFUNCTION()
	virtual void OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo);

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "Alive|Weapon")
	FGameplayTag FireMode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Alive|Weapon")
	FGameplayTag DefaultFireMode;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Weapon")
	FGameplayTag PrimaryAmmoType;


public:
	UFUNCTION(BlueprintCallable, Category = "Alive|Weapon")
	UAnimMontage* GetEquipMontage() const { return EquipMontage; }

	UFUNCTION(BlueprintCallable, Category = "Alive|Weapon")
	FVector GetFirePointWorldLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Alive|Weapon")
	AAliveCharacter* GetOwningCharacter() const { return OwningCharacter; }

	// Owner Should have AbilitySystemComponent
	UFUNCTION(BlueprintCallable, Category = "Alive|Weapon") // TODO
	void SetOwningCharacter(AAliveCharacter* InOwningCharacter);

	// Called when the player equips this weapon
	UFUNCTION(BlueprintCallable)
	void Equip();

	// Called when the player unequips this weapon
	void UnEquip();
protected:
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alive|Weapon")
	AAliveCharacter* OwningCharacter;

	// Collision capsule for when weapon is in pickup mode
	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "Alive|Weapon")
	USkeletalMeshComponent* WeaponMesh;

	// Relative Transform of weapon Mesh when equipped
	UPROPERTY(EditDefaultsOnly, Category = "Alive|Weapon")
	FTransform WeaponMeshRelativeTransform;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Alive|Animation")
	UAnimMontage* EquipMontage;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Weapon")
	FName FirePointSocket;


public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
protected:
	UPROPERTY()
	class UAliveAbilitySystemComponent* AbilitySystemComponent;

	//Default abilities for this Character. These will be removed on Character death and regiven if Character respawns. 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Alive|Abilities")
	TArray<TSubclassOf<class UAliveGameplayAbility>> WeaponAbilities;

	UPROPERTY(BlueprintReadOnly, Category = "Alive|Abilities")
	TArray<FGameplayAbilitySpecHandle> WeaponAbilitySpecHandles;

	UPROPERTY()
	class ATA_LineTrace* LineTraceTargetActor;
private:
	// Only add or remove ability on server. It will auto sync to client.
	void AddAbilitiesOnServer();
	void RemoveAbilitiesOnServer();

	int32 GetWeaponAbilityLevel() const;

	// Cache tag
	FGameplayTag WeaponIsFiringTag;
};
