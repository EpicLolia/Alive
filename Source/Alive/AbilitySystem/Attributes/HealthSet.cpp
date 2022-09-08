// Copyright Epic Games, Inc. All Rights Reserved.

#include "HealthSet.h"

#include "AliveTypes.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Player/AlivePlayerController.h"
#include "Player/AlivePlayerState.h"


UHealthSet::UHealthSet()
	: Health(100.f)
	  , MaxHealth(100.f)
	  , bOutOfHealth(false)
{
}

void UHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthSet, Health, OldValue);
}

void UHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthSet, MaxHealth, OldValue);
}

void UHealthSet::SendDamageInfoToRelevantPlayers(float DamageNum, const FGameplayEffectModCallbackData& Data) const
{
	FDamageResult DamageResult;
	DamageResult.Damage = DamageNum;
	if(Data.EffectSpec.DynamicAssetTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Effect.Damage.HeadShot"))))
	{
		DamageResult.bWeakness = true;
	}
		
	if(Data.EffectSpec.DynamicAssetTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Effect.Damage.HeadShot"))))
	{
		DamageResult.bCriticalHit = true;
	}
	else if(Data.EffectSpec.DynamicAssetTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Effect.Damage.HeadShot"))))
	{
		DamageResult.bBlock = true;
	}

	AAlivePlayerState* SourcePS = Cast<AAlivePlayerState>(
		Data.EffectSpec.GetContext().GetOriginalInstigatorAbilitySystemComponent()->GetOwner());
	if(SourcePS)
	{
		AAlivePlayerController* SourcePC = SourcePS->GetAlivePlayerController();
		// Can be AI Controller. So we should check it.
		if(SourcePC)
		{
			DamageResult.OppositeLocation = Data.EffectSpec.GetContext().GetHitResult()->Location;
			SourcePC->ClientRepDamageResultAsSource(DamageResult);
		}
	}

	AAlivePlayerState* TargetPS = Cast<AAlivePlayerState>(Data.Target.GetOwner());
	if(TargetPS)
	{
		AAlivePlayerController* TargetPC = TargetPS->GetAlivePlayerController();
		// Can be AI Controller. So we should check it.
		if(TargetPC)
		{
			DamageResult.OppositeLocation = Data.EffectSpec.GetContext().GetInstigator()->GetActorLocation();
			TargetPC->ClientRepDamageResultAsTarget(DamageResult);
		}
	}
}

bool UHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	return true;
}

void UHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		SetHealth(GetHealth() - GetDamage());
		
		SendDamageInfoToRelevantPlayers(GetDamage(),Data);
		
		SetDamage(0.0f);
		
		if ((GetHealth() <= 0.0f) && !bOutOfHealth)
		{
			if (OnOutOfHealth.IsBound())
			{
				const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
				AActor* Instigator = EffectContext.GetOriginalInstigator();
				AActor* Causer = EffectContext.GetEffectCauser();

				OnOutOfHealth.Broadcast(Instigator, Causer, Data.EffectSpec, Data.EvaluatedData.Magnitude);
			}
		}
		// Check health again in case an event above changed it.
		bOutOfHealth = (GetHealth() <= 0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		SetHealth(GetHealth() + GetHealing());
		SetHealing(0.0f);
	}
}

void UHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}
