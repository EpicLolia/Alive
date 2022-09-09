// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveGameMode.h"
#include "GameMode_Menu.generated.h"

UCLASS()
class ALIVE_API AGameMode_Menu : public AAliveGameMode
{
	GENERATED_BODY()

public:
	AGameMode_Menu();

	// Begin AGameModeBase interface
	/** skip it, menu doesn't require player start or pawn */
	virtual void RestartPlayer(class AController* NewPlayer) override;
	// End AGameModeBase interface

protected:
	virtual void BeginPlay() override;
};
