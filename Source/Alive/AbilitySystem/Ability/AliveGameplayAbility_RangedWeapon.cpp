// Fill out your copyright notice in the Description page of Project Settings.

#include "AliveGameplayAbility_RangedWeapon.h"

#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "GameplayAbilityTargetData_GenerateProjectile.h"
#include "Character/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapon/AliveWeapon.h"
#include "Weapon/ProjectileComponent.h"

FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleDegrees, const float Exponent)
{
	if (ConeHalfAngleDegrees > 0.f)
	{
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
	bDrawDebug = false;
	HitDamageMultiplier = 1.0f;
	HitEffect = UGameplayEffect::StaticClass();
}

void UAliveGameplayAbility_RangedWeapon::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Bind target data callback
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	OnTargetDataReadyCallbackDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(
		CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(
		this, &UAliveGameplayAbility_RangedWeapon::OnTargetDataReadyCallback);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAliveGameplayAbility_RangedWeapon::EndAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
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
		MyAbilityComponent->AbilityTargetDataSetDelegate(
			CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
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

void UAliveGameplayAbility_RangedWeapon::StartTargetingLocally()
{
	if (!CurrentActorInfo->IsLocallyControlled())
	{
		return;
	}

	// Fill out the target data from the hit results
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	GenerateProjectileTargetDataHandle(TargetDataHandle);

	// Process the target data immediately
	OnTargetDataReadyCallback(TargetDataHandle, FGameplayTag());
}

void UAliveGameplayAbility_RangedWeapon::GenerateProjectileTargetDataHandle(FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	for (int32 bullet = 0; bullet < GetSourceWeapon()->GetPrimaryCartridgeAmmo(); ++bullet)
	{
		TargetDataHandle.Add(GenerateProjectileTargetData());
	}
	TargetDataHandle.UniqueId = FMath::Rand();
}

FGameplayAbilityTargetData* UAliveGameplayAbility_RangedWeapon::GenerateProjectileTargetData()
{
	APlayerCharacter* const MyCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	check(MyCharacter);

	const FVector TraceStart = MyCharacter->GetCameraBoom()->GetComponentLocation();
	const FVector TraceEnd = MyCharacter->GetBaseAimRotation().Vector() *
		GetSourceWeapon()->GetProjectileComponent()->GetRange() * 100.0f + TraceStart;

	TArray<FHitResult> FoundHits;
	TraceAndDrawDebug(/*out*/ FoundHits, TraceStart, TraceEnd);
	int32 ValidHitIndex = FindFirstValidTargettingResult(FoundHits);


	FVector AimDirAfterSpread = VRandConeNormalDistribution(
		MyCharacter->GetBaseAimRotation().Vector(), GetSourceWeapon()->GetCurrentSpreadAngle() / 2.0f, 1.0f);
	FVector TargetPoint;
	if (ValidHitIndex == INDEX_NONE)
	{
		TargetPoint = AimDirAfterSpread * GetSourceWeapon()->GetProjectileComponent()->GetRange() * 100.0f + TraceStart;
	}
	else
	{
		TargetPoint = AimDirAfterSpread * FoundHits[ValidHitIndex].Distance + TraceStart;
	}

	FGameplayAbilityTargetData_GenerateProjectile* TargetData = new FGameplayAbilityTargetData_GenerateProjectile();

	TargetData->SourceLocation = GetSourceWeapon()->GetFirePointWorldLocation();
	TargetData->Direction = (TargetPoint - GetSourceWeapon()->GetFirePointWorldLocation()).GetSafeNormal();

	return TargetData;
}

void UAliveGameplayAbility_RangedWeapon::OnTargetDataReadyCallback(
	const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, IsPredictingClient());

	// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
	FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

	// Call the server
	if (CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority())
	{
		MyAbilityComponent->CallServerSetReplicatedTargetData(
			CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(),
			LocalTargetDataHandle, ApplicationTag, MyAbilityComponent->ScopedPredictionKey);
	}

	FireProjectile(LocalTargetDataHandle);

	// We've processed the data
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UAliveGameplayAbility_RangedWeapon::FireProjectile(const FGameplayAbilityTargetDataHandle& TargetData)
{
	UProjectileComponent* ProjectileComp = GetSourceWeapon()->GetProjectileComponent();
	check(ProjectileComp);

	if (CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		GetSourceWeapon()->AddSpread();
		
		// Only Handle Effect on the server.
		if (GetCurrentActorInfo()->IsNetAuthority())
		{
			FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(HitEffect, GetAbilityLevel());
			EffectSpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), HitDamageMultiplier);
			ProjectileComp->GenerateProjectileHandle(TargetData.UniqueId,EffectSpecHandle,TargetData.Num());
		}
		// Only Generate ProjectileInstance locally.
		if(CurrentActorInfo->IsLocallyControlled())
		{
			for (const auto& Data : TargetData.Data)
			{
				const FGameplayAbilityTargetData_GenerateProjectile* MyTargetData =
					static_cast<const FGameplayAbilityTargetData_GenerateProjectile*>(Data.Get());

				ProjectileComp->GenerateProjectileInstance(TargetData.UniqueId, MyTargetData->SourceLocation, MyTargetData->Direction);
			}
		}
		// Let the blueprint do stuff
		OnProjectileFired(TargetData);
	}
	else
	{
		K2_EndAbility();
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
	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 3.0f, 0, 5);
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
		if (CurrentHitResult.Distance > 100.0f)
		{
			return Idx;
		}
	}
	return INDEX_NONE;
}
