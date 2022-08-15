// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AliveWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponAmmoChangedDelegate, int32, OldValue, int32, NewValue);

UCLASS()
class ALIVE_API AAliveWeapon : public AActor
{
	GENERATED_BODY()

public:
	AAliveWeapon();

	UPROPERTY(BlueprintAssignable, Category = "Alive|Weapon")
	FWeaponAmmoChangedDelegate OnPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Alive|Weapon")
	FWeaponAmmoChangedDelegate OnMaxPrimaryClipAmmoChanged;

	UFUNCTION(BlueprintCallable)
	int32 GetPrimaryClipAmmo() const { return PrimaryClipAmmo; }
	
	UFUNCTION(BlueprintCallable)
	int32 GetMaxPrimaryClipAmmo() const { return MaxPrimaryClipAmmo; }

	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Pickup on touch
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;
	
protected:
	virtual void BeginPlay() override;

	// Add the abilities to owner's ASC
	UPROPERTY()
	class UAliveAbilitySystemComponent* AbilitySystemComponent;

	// How much ammo in the clip the gun starts with
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_PrimaryClipAmmo, Category = "Alive|Weapon|Ammo")
	int32 PrimaryClipAmmo;
	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxPrimaryClipAmmo, Category = "Alive|Weapon|Ammo")
	int32 MaxPrimaryClipAmmo;
	UFUNCTION()
	virtual void OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo);
};
