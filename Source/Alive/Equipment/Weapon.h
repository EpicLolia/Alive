// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment.h"
#include "Weapon.generated.h"

class UWeapon;

/**
 * 
 */
UCLASS()
class ALIVE_API UWeapon : public UEquipment
{
	GENERATED_BODY()

public:
	UWeapon();
};
//
// USTRUCT()
// struct FWeaponSpec : public FEquipmentSpec
// {
// 	GENERATED_BODY()
//
// 	FWeaponSpec(TSubclassOf<UWeapon> WeaponType):Super(WeaponType){}
// };
