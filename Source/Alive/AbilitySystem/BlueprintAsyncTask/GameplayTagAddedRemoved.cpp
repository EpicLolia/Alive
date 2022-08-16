// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/BlueprintAsyncTask/GameplayTagAddedRemoved.h"
#include "AbilitySystemComponent.h"

UGameplayTagAddedRemoved* UGameplayTagAddedRemoved::ListenForGameplayTagAddedOrRemoved(
	UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer Tags)
{
	UGameplayTagAddedRemoved* ListenForGameplayTagAddedRemoved = NewObject<UGameplayTagAddedRemoved>();
	ListenForGameplayTagAddedRemoved->ASC = AbilitySystemComponent;
	ListenForGameplayTagAddedRemoved->Tags = Tags;

	if (!IsValid(AbilitySystemComponent) || Tags.Num() < 1)
	{
		ListenForGameplayTagAddedRemoved->EndTask();
		return nullptr;
	}

	TArray<FGameplayTag> TagArray;
	Tags.GetGameplayTagArray(TagArray);

	for (FGameplayTag Tag : TagArray)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(
			ListenForGameplayTagAddedRemoved, &UGameplayTagAddedRemoved::TagChanged);
	}

	return ListenForGameplayTagAddedRemoved;
}

void UGameplayTagAddedRemoved::EndTask()
{
	if (IsValid(ASC))
	{
		TArray<FGameplayTag> TagArray;
		Tags.GetGameplayTagArray(TagArray);

		for (FGameplayTag Tag : TagArray)
		{
			ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}

	SetReadyToDestroy();
	MarkPendingKill();
}

void UGameplayTagAddedRemoved::TagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		OnTagAdded.Broadcast(Tag);
	}
	else
	{
		OnTagRemoved.Broadcast(Tag);
	}
}
