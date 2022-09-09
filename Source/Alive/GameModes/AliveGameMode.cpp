// Copyright Epic Games, Inc. All Rights Reserved.

#include "AliveGameMode.h"

#include "Online/AliveGameSession.h"

AAliveGameMode::AAliveGameMode()
	: Super()
{
}

TSubclassOf<AGameSession> AAliveGameMode::GetGameSessionClass() const
{
	return AAliveGameSession::StaticClass();
}
