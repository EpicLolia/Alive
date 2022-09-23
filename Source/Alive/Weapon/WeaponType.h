// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "WeaponType.generated.h"

class UGameplayAbility;

/**
 * FWeaponPerformance
 * Replicated to every player.
 * Allow players to use different appearances and effects.
 */
USTRUCT(BlueprintType)
struct FWeaponPerformance
{
	GENERATED_BODY()

	FWeaponPerformance() { return; }
	
	FWeaponPerformance(const class UWeaponType* InWeaponType,
					   int32 InAppearanceType = 0,
					   int32 InOpenFireEmitterType = 0,
					   int32 InOpenFireSoundType = 0)
		: WeaponType(InWeaponType),
		  AppearanceType(InAppearanceType),
		  OpenFireEmitterType(InOpenFireEmitterType),
		  OpenFireSoundType(InOpenFireSoundType)
	{
	}

	/** Always the ClassDefaultObject. */
	UPROPERTY(BlueprintReadOnly)
	const class UWeaponType* WeaponType;

	UPROPERTY(BlueprintReadOnly)
	int32 AppearanceType;

	UPROPERTY(BlueprintReadOnly)
	int32 OpenFireEmitterType;

	UPROPERTY(BlueprintReadOnly)
	int32 OpenFireSoundType;
};


/**
 * UWeaponType
 */
UCLASS(Blueprintable)
class ALIVE_API UWeaponType : public UObject
{
	GENERATED_BODY()
	
protected:
	UWeaponType(){}

	UFUNCTION(BlueprintImplementableEvent,DisplayName = "OnProjectileHit")
	void K2_OnProjectileHit(const FHitResult& HitResult);
	
public:
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
	float MinSpreadAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Spread")
	float SpreadAngleIncreaseRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Spread")
	float MaxSpreadAngle;

	/**
	 * This determines whether the projectile will spawn projectile actors.
	 * If true, the parameters of the projectile should be set in the projectile actor.
	 * If false, use ray trace.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Projectile")
	bool bEntity = false;
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
};
