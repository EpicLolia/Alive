// Fill out your copyright notice in the Description page of Project Settings.

#include "AliveGameplayAbility_RangedWeapon.h"

#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "GameplayAbilityTargetData_GenerateProjectile.h"
#include "Character/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapon/AliveWeapon.h"
#include "Weapon/ProjectileComponent.h"


/**
 * Copy from Lyra.
 */
FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad, const float Exponent)
{
	if (ConeHalfAngleRad > 0.f)
	{
		const float ConeHalfAngleDegrees = FMath::RadiansToDegrees(ConeHalfAngleRad);

		// consider the cone a concatenation of two rotations. one "away" from the center line, and another "around" the circle
		// apply the exponent to the away-from-center rotation. a larger exponent will cluster points more tightly around the center
		const float FromCenter = FMath::Pow(FMath::FRand(), Exponent);
		const float AngleFromCenter = FromCenter * ConeHalfAngleDegrees;
		const float AngleAround = FMath::FRand() * 360.0f;

		FRotator Rot = Dir.Rotation();
		FQuat DirQuat(Rot);
		FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));
		FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));
		FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
		FinalDirectionQuat.Normalize();

		return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
	}
	else
	{
		return Dir.GetSafeNormal();
	}
}

UAliveGameplayAbility_RangedWeapon::UAliveGameplayAbility_RangedWeapon()
{
}

void UAliveGameplayAbility_RangedWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                                                    bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo,
			                                                      bReplicateEndAbility, bWasCancelled));
			return;
		}

		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyAbilityComponent);

		// When ability ends, consume target data and remove delegate
		MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(
			OnTargetDataReadyCallbackDelegateHandle);
		MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

AAliveWeapon* UAliveGameplayAbility_RangedWeapon::GetSourceWeapon() const
{
	if (FGameplayAbilitySpec* Spec = UGameplayAbility::GetCurrentAbilitySpec())
	{
		return Cast<AAliveWeapon>(Spec->SourceObject);
	}
	return nullptr;
}

void UAliveGameplayAbility_RangedWeapon::StartRangedWeaponTargeting()
{
	TArray<FVector> ProjectileDirections;

	GenerateProjectileDirection(ProjectileDirections);

	// Fill out the target data from the hit results
	FGameplayAbilityTargetDataHandle TargetData;

	if (ProjectileDirections.Num() > 0)
	{
		const int16 ProjectileID = FMath::Rand();
		for (const FVector& Direction : ProjectileDirections)
		{
			FGameplayAbilityTargetData_GenerateProjectile* NewTargetData = new FGameplayAbilityTargetData_GenerateProjectile();
			// Directly use the unique predictionKey 
			NewTargetData->ProjectileID = ProjectileID;
			NewTargetData->Direction = Direction;

			TargetData.Add(NewTargetData);
		}
	}

	// Process the target data immediately
	OnTargetDataReadyCallback(TargetData, FGameplayTag());
}

void UAliveGameplayAbility_RangedWeapon::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData,
                                                                   FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, IsPredictingClient());

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		if (CurrentActorInfo->IsLocallyControlled())
		{
			AAliveWeapon* MyWeapon = GetSourceWeapon();
			check(MyWeapon);
			MyWeapon->AddSpread();

			// Let the blueprint do stuff like apply effects to the targets
			OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
			if (!CurrentActorInfo->IsNetAuthority())
			{
				MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(),
				                                                      InData, ApplicationTag,
				                                                      MyAbilityComponent->ScopedPredictionKey);
			}
		}
		else if (HasAuthority(&CurrentActivationInfo))
		{
			//Since multifire is supported, we still need to hook up the callbacks
			MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).
			                    AddUObject(
				                    this, &UAliveGameplayAbility_RangedWeapon::OnTargetDataReplicatedCallback);

			MyAbilityComponent->CallReplicatedTargetDataDelegatesIfSet(CurrentSpecHandle,
			                                                           CurrentActivationInfo.GetActivationPredictionKey());

			// We've processed the data
			MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
		}
	}
}

void UAliveGameplayAbility_RangedWeapon::TraceAndDrawDebug(TArray<FHitResult>& OutHits, const FVector& Start, const FVector& End)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());

	FCollisionQueryParams Params(SCENE_QUERY_STAT(UAliveGameplayAbility_RangedWeapon), false);
	Params.AddIgnoredActors(ActorsToIgnore);

	// Only the overlap hit results and the first block hit result will be generated.
	GetWorld()->LineTraceMultiByChannel(OutHits, Start, End, ECC_Projectile, Params);

#if ENABLE_DRAW_DEBUG
	//if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 3.0f, 0, 5);
	}
#endif
}

int32 UAliveGameplayAbility_RangedWeapon::FindFirstValidTargettingResult(const TArray<FHitResult>& HitResults)
{
	for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx)
	{
		const FHitResult& CurrentHitResult = HitResults[Idx];
		// TODO: Ignore Teammate
		// ...
		return Idx;
	}
	return INDEX_NONE;
}

void UAliveGameplayAbility_RangedWeapon::GenerateProjectileDirection(TArray<FVector>& ProjectileDirections)
{
	APlayerCharacter* const MyCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	check(MyCharacter);
	AAliveWeapon* const MyWeapon = GetSourceWeapon();
	check(MyWeapon);

	// Only do trace in local player
	if (!MyCharacter->IsLocallyControlled())
	{
		return;
	}

	const FVector TraceStart = MyCharacter->GetCameraBoom()->GetComponentLocation();
	const FVector TraceEnd = MyCharacter->GetBaseAimRotation().Vector() *
		MyWeapon->GetProjectileComponent()->GetRange() * 100.0f + TraceStart;

	TArray<FHitResult> FoundHits;

	TraceAndDrawDebug(/*out*/ FoundHits, TraceStart, TraceEnd);

	int32 ValidHitIndex = FindFirstValidTargettingResult(FoundHits);

	for (int32 bullet = 0; bullet < MyWeapon->GetPrimaryCartridgeAmmo(); ++bullet)
	{
		FVector AimDirAfterSpread = VRandConeNormalDistribution(
			MyCharacter->GetBaseAimRotation().Vector(), MyWeapon->GetCurrentSpreadAngle() / 2, 1.0f);
		FVector TargetPoint;
		if (ValidHitIndex == INDEX_NONE)
		{
			TargetPoint = AimDirAfterSpread * MyWeapon->GetProjectileComponent()->GetRange() * 100.0f;
		}
		else
		{
			TargetPoint = AimDirAfterSpread * FoundHits[ValidHitIndex].Distance;
		}
		ProjectileDirections.Emplace((TargetPoint - MyWeapon->GetFirePointWorldLocation()).GetSafeNormal());
	}
}

void UAliveGameplayAbility_RangedWeapon::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data,
                                                                        FGameplayTag ActivationTag)
{
	OnRangedWeaponTargetDataReady(Data);
}
