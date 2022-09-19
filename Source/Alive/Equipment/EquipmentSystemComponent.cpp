// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentSystemComponent.h"

#include "GameplayAbilitySpec.h"

UEquipmentSystemComponent::UEquipmentSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEquipmentSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FGameplayAbilitySpecContainer a;

}

void UEquipmentSystemComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UEquipmentSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
