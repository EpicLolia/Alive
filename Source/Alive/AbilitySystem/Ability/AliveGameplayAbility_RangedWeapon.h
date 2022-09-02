﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveGameplayAbility_Equipment.h"
#include "GameplayCueNotify_Static.h"
#include "AliveGameplayAbility_RangedWeapon.generated.h"

/**
 * The base GameplayAblity class of this project.
 */
UCLASS()
class ALIVE_API UAliveGameplayAbility_RangedWeapon : public UAliveGameplayAbility_Equipment
{
	GENERATED_BODY()

public:
	UAliveGameplayAbility_RangedWeapon();

	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

	class AAliveWeapon* GetSourceWeapon() const;

protected:
	UPROPERTY(EditDefaultsOnly,Category = "Alive|Projectile")
	TSubclassOf<UGameplayEffect> HitEffect;
	UPROPERTY(EditDefaultsOnly,Category = "Alive|Projectile")
	float HitDamageMultiplier;
	
	UFUNCTION(BlueprintCallable)
	void StartTargetingLocally();
	
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	// Called when target data is ready. Will check and commit cost
	UFUNCTION(BlueprintNativeEvent)
	void OnProjectileFire(const FGameplayAbilityTargetDataHandle& TargetData);
	void OnProjectileFire_Implementation(const FGameplayAbilityTargetDataHandle& TargetData);
	
private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;

	void GenerateProjectileTargetDataHandle(FGameplayAbilityTargetDataHandle& TargetDataHandle);
	/** Will return a target data pointer generated by new. */
	FGameplayAbilityTargetData* GenerateProjectileTargetData();
	
	void TraceAndDrawDebug(OUT TArray<FHitResult>& OutHits, const FVector& Start, const FVector& End);
	// Maybe we hit the teammate.
	int32 FindFirstValidTargettingResult(const TArray<FHitResult>& HitResults);
};
