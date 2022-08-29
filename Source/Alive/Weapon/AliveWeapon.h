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

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	
	virtual void BeginPlay() override;
	
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:
	int32 GetPrimaryClipAmmo() const { return PrimaryClipAmmo; }
	int32 GetMaxPrimaryClipAmmo() const { return MaxPrimaryClipAmmo; }

	UFUNCTION(BlueprintCallable, Category = "Alive|Ammo")
	void SetPrimaryClipAmmo(int32 Ammo);
	
	UPROPERTY(BlueprintAssignable, Category = "Alive|Ammo")
	FWeaponAmmoChangedDelegate OnPrimaryClipAmmoChanged;

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, ReplicatedUsing = OnRep_PrimaryClipAmmo, Category = "Alive|Ammo")
	int32 PrimaryClipAmmo;
	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Ammo")
	int32 MaxPrimaryClipAmmo;

	// What kind of Ammo is used in this weapon.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Ammo")
	FGameplayTag PrimaryAmmoType;

	// Init the FireMode while character pick the weapon up
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive")
	FGameplayTag DefaultFireMode;

	// Only the client's fire action will be affected. Server do not care about it.
	UPROPERTY(BlueprintReadWrite, Category = "Alive")
	FGameplayTag FireMode;

public:
	FORCEINLINE AAliveCharacter* GetOwningCharacter()const{return OwningCharacter;}
	
	// Called when added to inventory. Owner Should have AbilitySystemComponent
	UFUNCTION(BlueprintCallable, Category = "Alive")
	void SetOwningCharacter(AAliveCharacter* InOwningCharacter);
	
	// Called when the character dies or the weapon is discarded
	UFUNCTION(BlueprintCallable, Category = "Alive")
	void RemoveFormOwningCharacter();
	
	void SetWeaponVisibility(bool bWeaponVisibility) const;

protected:
	// This object is valid only in the server. Temporarily. 
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alive")
	AAliveCharacter* OwningCharacter;
	
	// Relative Transform of weapon Mesh when equipped
	UPROPERTY(EditDefaultsOnly, Category = "Alive|Transform")
	FTransform WeaponMeshRelativeTransform;
	
private:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "AliveWeapon",meta=(AllowPrivateAccess = true))
	USkeletalMeshComponent* WeaponMesh;

public:
	// Must set up FirePointSocket in Alive|Transform. 
	UFUNCTION(BlueprintCallable, Category = "Alive")
	FVector GetFirePointWorldLocation() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Alive|Transform")
	FName FirePointSocket;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|Animation")
	UAnimMontage* EquipMontage;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	UPROPERTY()
	class UAliveAbilitySystemComponent* AbilitySystemComponent;

	//Default abilities for this Character. These will be removed on Character death and regiven if Character respawns. 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Alive|Abilities")
	TArray<TSubclassOf<class UAliveGameplayAbility>> WeaponAbilities;

	UPROPERTY()
	class ATA_LineTrace* LineTraceTargetActor;
	
private:
	// Only add or remove ability on server. It will auto sync to client.
	void AddAbilities();
	void RemoveAbilities();

	int32 GetWeaponAbilityLevel() const;
	// Cache tag
	FGameplayTag WeaponIsFiringTag;
	
public:
	// Cache the activatable abilities added by this weapon
	TArray<FGameplayAbilitySpecHandle> WeaponAbilitySpecHandles;
};
