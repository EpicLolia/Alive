// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "AlivePhysicalMaterial.generated.h"

/**
 * 
 */
UCLASS()
class ALIVE_API UAlivePhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly,Category="Alive")
	int32 FootstepType;
};
