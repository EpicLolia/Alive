// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "AliveGameSession.generated.h"

UCLASS(Config = Game)
class ALIVE_API AAliveGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	AAliveGameSession(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
