// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AliveGameMode.generated.h"

/**
 * AAliveGameMode
 *
 * The base GameMode class of this project
 */
UCLASS(minimalapi)
class AAliveGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAliveGameMode();
	
	// Begin AGameModeBase interface
	/** Returns game session class to use */
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;
	// End AGameModeBase interface
};



