// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AliveGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ALIVE_API UAliveGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;
};
