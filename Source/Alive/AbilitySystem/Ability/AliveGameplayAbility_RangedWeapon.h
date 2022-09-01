// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveGameplayAbility_Equipment.h"
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
	UFUNCTION(BlueprintCallable)
	void StartTargetingLocally();
	
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	// Called when target data is ready
	UFUNCTION(BlueprintNativeEvent)
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);
	void OnRangedWeaponTargetDataReady_Implementation(const FGameplayAbilityTargetDataHandle& TargetData);
	
private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;

	void TraceAndDrawDebug(OUT TArray<FHitResult>& OutHits, const FVector& Start, const FVector& End);
	// Maybe we hit the teammate.
	int32 FindFirstValidTargettingResult(const TArray<FHitResult>& HitResults);
	void GenerateProjectileDirection(TArray<FVector>& ProjectileDirections);
};
