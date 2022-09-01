// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/AliveGameInstance.h"
#include "AbilitySystemGlobals.h"

void UAliveGameInstance::Init()
{
	Super::Init();
	
	UAbilitySystemGlobals::Get().InitGlobalData();
}
