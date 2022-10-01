// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "WeaponType.h"
#include "GameFramework/Actor.h"
#include "Item/PickupInterface.h"
#include "Weapon.generated.h"

class UWeaponType;
class AAliveCharacter;

/**
 * AWeapon
 */
UCLASS(BlueprintType)
class ALIVE_API AWeapon : public AActor, public IPickupInterface
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void BeginPlay() override;
	
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

protected:
	/*
	 * Static Settings
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesGrantedToOwner;

	/** The type of ammo used in this weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	FGameplayTag AmmoType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	int32 MaxClipAmmo = 30;

	/** How many bullets will be fired in a single firing. e.g. Rifle or pistol = 1, shotgun > 1 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	int32 CartridgeAmmo = 1;

	/* All fire modes this weapon has. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FGameplayTagContainer FireModes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Spread")
	float MinSpreadAngle = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Spread")
	float SpreadAngleIncreaseRate = 0.4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Spread")
	float MaxSpreadAngle = 15.0f;

	/**
	 * This determines whether the projectile will spawn projectile actors.
	 * If true, we will not activate the ProjectileComponent.
	 * And the configuration of the projectile should be set in the projectile actor.
	 * If false, activate the ProjectileComponent and use ray trace to simulate projectile.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile")
	bool bUseProjectileActor;
	/**
	 * Use a logical tick rate. Ensure that the client and server have the same projectile simulation results.
	 * MaxFrequency is decided by tick rate.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile",
		meta = (EditCondition = "!bEntity", EditConditionHides, ClampMin = 1, ClampMax = 60))
	int32 UpdateFrequency = 10;
	/** Do not work on projectile actor. Units: m */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile",
		meta = (EditCondition = "!bEntity", EditConditionHides, ClampMin = 0.5f, ClampMax = 3000.0f))
	float Range = 600.0f;

	/**
	 * Determines whether the projectile is affected by gravity.
	 * Do not work on projectile actor.
	 * Units: m/s^2
	 **/
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile",
		meta = (EditCondition = "bGravity", ClampMin = 0.1f, ClampMax = 100.0f))
	float Gravity = 10.0f;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile",
		meta = (InlineEditConditionToggle = "Gravity"))
	bool bGravity = false;

	/**
	 * Determines whether the projectile is affected by gravity.
	 * If false, it will perform like a hit scan, and get hit result immediately.
	 * Do not work on projectile actor.
	 * Units: m/s 
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile",
		meta = (EditCondition = "bVelocity", ClampMin = 1.0f, ClampMax = 1200.0f))
	float Velocity = 300.0f;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile",
		meta = (InlineEditConditionToggle = "Velocity"))
	bool bVelocity = false;


	/*
	 * Performance
	 */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
	USkeletalMesh* WeaponMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
	TArray<UMaterial*> WeaponMaterials;

	// Relative Transform of weapon Mesh when equipped
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
	FTransform WeaponMeshRelativeTransform;
	// Used to find the openfire location
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance")
	FName FirePointSocket;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance|Animation")
	UAnimMontage* EquipMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance|Animation")
	UAnimMontage* MeleeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance|Effect")
	TArray<UParticleSystem*> OpenFireEmitters;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Performance|Effect")
	TArray<USoundBase*> OpenFireSounds;

	/*
	 * Display
	 */

	/** UI */
	
	/*
	 * Debug
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	bool bDrawWeaponTrace = false;
};
