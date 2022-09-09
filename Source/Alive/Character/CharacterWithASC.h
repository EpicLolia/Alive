// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveCharacter.h"
#include "CharacterWithASC.generated.h"

UCLASS()
class ALIVE_API ACharacterWithASC : public AAliveCharacter
{
	GENERATED_BODY()

public:
	ACharacterWithASC();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
