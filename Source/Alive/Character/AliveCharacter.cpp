// Copyright Epic Games, Inc. All Rights Reserved.

#include "AliveCharacter.h"

#include "AbilitySystem/Ability/AliveGameplayAbility.h"
#include "AbilitySystem/AliveAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/HealthSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/AliveWeapon.h"

//DEFINE_LOG_CATEGORY_STATIC(LogAliveChar, Warning, All);

AAliveCharacter::AAliveCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	DeathState = EDeathState::NotDead;
	HealthSet = nullptr;
}

UAbilitySystemComponent* AAliveCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAliveCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAliveCharacter, DeathState, COND_None);
}

void AAliveCharacter::UninitializeAbilitySystem()
{
	check(AbilitySystemComponent);
	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		AbilitySystemComponent->CancelAbilities(nullptr, nullptr);
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
			AbilitySystemComponent->ClearActorInfo();
		}

		UninitializeFromAbilitySystem();
	}

	AbilitySystemComponent = nullptr;
}

void AAliveCharacter::InitializeWithAbilitySystem()
{
	check(AbilitySystemComponent);
	HealthSet = AbilitySystemComponent->GetSet<UHealthSet>();
	check(HealthSet);
	// Register to listen for attribute changes.
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::ProcessOutOfHealth);
}

void AAliveCharacter::UninitializeFromAbilitySystem()
{
	if (HealthSet)
	{
		HealthSet->OnOutOfHealth.RemoveAll(this);
	}

	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void AAliveCharacter::AddCharacterAbilities()
{
	if (AbilitySystemComponent && !AbilitySystemComponent->bHasCharacterAbilities)
	{
		for (TSubclassOf<UAliveGameplayAbility>& Ability : CharacterAbilities)
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(Ability, 1, INDEX_NONE, this)); // ???????????????1???
		}
		AbilitySystemComponent->bHasCharacterAbilities = true;
	}
}

void AAliveCharacter::RemoveCharacterAbilities()
{
	// Remove any abilities added from a previous call.
	// This checks to make sure the ability is in the startup 'CharacterAbilities' array.
	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if ((Spec.SourceObject == this) && CharacterAbilities.Contains(Spec.Ability->GetClass()))
		{
			AbilitiesToRemove.Add(Spec.Handle);
		}
	}
	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitiesToRemove)
	{
		AbilitySystemComponent->ClearAbility(SpecHandle);
	}

	AbilitySystemComponent->bHasCharacterAbilities = false;
}

void AAliveCharacter::AddCharacterEffects()
{
	if (AbilitySystemComponent)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		for (TSubclassOf<UGameplayEffect> EffectClass : CharacterEffects)
		{
			if (!EffectClass)
			{
				continue;
			}
			FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(
				EffectClass, 1, EffectContext);
			if (NewHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
			}
		}
	}
}
void AAliveCharacter::StartDeath()
{
	DeathState = EDeathState::DeathStarted;

	DisableMovementAndCollision();

	K2_OnDeathStarted();
}

void AAliveCharacter::FinishDeath()
{
	DeathState = EDeathState::DeathFinished;
	
	K2_OnDeathFinished();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::UninitializeAndDestroy);
}

void AAliveCharacter::FinishDeathImmediately()
{
	UninitializeAndDestroy();
}

void AAliveCharacter::OnRep_DeathState(EDeathState OldDeathState)
{
	if (OldDeathState == EDeathState::NotDead)
	{
		if (DeathState == EDeathState::DeathStarted)
		{
			StartDeath();
		}
		else if (DeathState == EDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
	}
	else if (OldDeathState == EDeathState::DeathStarted && DeathState == EDeathState::DeathFinished)
	{
		FinishDeath();
	}
}

void AAliveCharacter::DisableMovementAndCollision()
{
	if (Controller)
	{
		Controller->SetIgnoreMoveInput(true);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	UCharacterMovementComponent* AliveMovementComp = Cast<UCharacterMovementComponent>(GetCharacterMovement());
	AliveMovementComp->StopMovementImmediately();
	AliveMovementComp->DisableMovement();
}

void AAliveCharacter::UninitializeAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (ASC->GetAvatarActor() == this)
		{
			UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}

void AAliveCharacter::ProcessOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec,
                                        float DamageMagnitude)
{
#if WITH_SERVER_CODE
	check(AbilitySystemComponent);
	// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
	{
		FGameplayEventData Payload;
		Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Death"));
		Payload.Instigator = DamageInstigator;
		Payload.Target = AbilitySystemComponent->GetAvatarActor();
		Payload.OptionalObject = DamageEffectSpec.Def;
		Payload.ContextHandle = DamageEffectSpec.GetEffectContext();
		Payload.InstigatorTags = *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
		Payload.TargetTags = *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();
		Payload.EventMagnitude = DamageMagnitude;

		FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
		AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
#endif
	OnDeath(DamageCauser);
}
