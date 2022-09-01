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
	const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle);
	check(AbilitySpec);

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
	AAliveWeapon* MyWeapon = GetSourceWeapon();
	check(MyWeapon);
	MyWeapon->AddSpread();

	// Let the blueprint do stuff like apply effects to the targets
	OnRangedWeaponTargetDataReady(LocalTargetDataHandle);

	// We've processed the data
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UAliveGameplayAbility_RangedWeapon::OnRangedWeaponTargetDataReady_Implementation(const FGameplayAbilityTargetDataHandle& TargetData)
{
	UProjectileComponent* ProjectileComp = GetSourceWeapon()->GetProjectileComponent();
	check(ProjectileComp);

	for (const auto& Data : TargetData.Data)
	{
		ProjectileComp->FireOneProjectile(Data.Get());
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
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange,
		                                 FString::Printf(TEXT("Distance is: %f"), CurrentHitResult.Distance));
		if (CurrentHitResult.Distance > 100.0f)
		{
			return Idx;
		}
	}
	return INDEX_NONE;
}

void UAliveGameplayAbility_RangedWeapon::GenerateProjectileDirection(TArray<FVector>& ProjectileDirections)
{
	APlayerCharacter* const MyCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	check(MyCharacter);
	AAliveWeapon* const MyWeapon = GetSourceWeapon();
	check(MyWeapon);

	const FVector TraceStart = MyCharacter->GetCameraBoom()->GetComponentLocation();
	const FVector TraceEnd = MyCharacter->GetBaseAimRotation().Vector() *
		MyWeapon->GetProjectileComponent()->GetRange() * 100.0f + TraceStart;

	TArray<FHitResult> FoundHits;

	TraceAndDrawDebug(/*out*/ FoundHits, TraceStart, TraceEnd);

	int32 ValidHitIndex = FindFirstValidTargettingResult(FoundHits);

	for (int32 bullet = 0; bullet < MyWeapon->GetPrimaryCartridgeAmmo(); ++bullet)
	{
		FVector AimDirAfterSpread = VRandConeNormalDistribution(
			MyCharacter->GetBaseAimRotation().Vector(), MyWeapon->GetCurrentSpreadAngle() / 2.0f, 1.0f);
		FVector TargetPoint;
		if (ValidHitIndex == INDEX_NONE)
		{
			TargetPoint = AimDirAfterSpread * MyWeapon->GetProjectileComponent()->GetRange() * 100.0f + TraceStart;
		}
		else
		{
			TargetPoint = AimDirAfterSpread * FoundHits[ValidHitIndex].Distance + TraceStart;
		}
		ProjectileDirections.Emplace((TargetPoint - MyWeapon->GetFirePointWorldLocation()).GetSafeNormal());
	}
}
