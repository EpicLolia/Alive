// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/BlueprintAsyncTask/AttributeChanged.h"

UAttributeChanged* UAttributeChanged::ListenForAttributeChange(UAbilitySystemComponent* AbilitySystemComponent,
                                                               FGameplayAttribute Attribute)
{
	UAttributeChanged* WaitForAttributeChangedTask = NewObject<UAttributeChanged>();
	WaitForAttributeChangedTask->ASC = AbilitySystemComponent;
	WaitForAttributeChangedTask->AttributeToListenFor = Attribute;

	if (!IsValid(AbilitySystemComponent) || !Attribute.IsValid())
	{
		WaitForAttributeChangedTask->RemoveFromRoot();
		return nullptr;
	}

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(
		WaitForAttributeChangedTask, &UAttributeChanged::AttributeChanged);

	return WaitForAttributeChangedTask;
}

UAttributeChanged* UAttributeChanged::ListenForAttributesChange(UAbilitySystemComponent* AbilitySystemComponent,
                                                                TArray<FGameplayAttribute> Attributes)
{
	UAttributeChanged* WaitForAttributeChangedTask = NewObject<UAttributeChanged>();
	WaitForAttributeChangedTask->ASC = AbilitySystemComponent;
	WaitForAttributeChangedTask->AttributesToListenFor = Attributes;

	if (!IsValid(AbilitySystemComponent) || Attributes.Num() < 1)
	{
		WaitForAttributeChangedTask->RemoveFromRoot();
		return nullptr;
	}

	for (FGameplayAttribute Attribute : Attributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(
			WaitForAttributeChangedTask, &UAttributeChanged::AttributeChanged);
	}

	return WaitForAttributeChangedTask;
}

void UAttributeChanged::EndTask()
{
	if (IsValid(ASC))
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeToListenFor).RemoveAll(this);

		for (FGameplayAttribute Attribute : AttributesToListenFor)
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
		}
	}

	SetReadyToDestroy();
	MarkPendingKill();
}

void UAttributeChanged::AttributeChanged(const FOnAttributeChangeData& Data)
{
	OnAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}
