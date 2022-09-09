// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode_Game.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AGameMode_Game::AGameMode_Game()
{
	PrimaryActorTick.bCanEverTick = false;

	WarmupTime = 15.0f;
	RoundTime = 300.0f;
	TimeBetweenMatches = 20.0f;
	MaxBots = 6;
	PlayerRespawnCooldown = 5.0f;
}

FTransform AGameMode_Game::GetRandomSpawnTransform(FName Tag) const
{
	TArray<APlayerStart*> StarterPoints;
	for (TActorIterator<APlayerStart> StartIt(GetWorld()); StartIt; ++StartIt)
	{
		if (StartIt->PlayerStartTag == Tag)
		{
			StarterPoints.Add(*StartIt);
		}
	}
	if (StarterPoints.Num())
	{
		return StarterPoints[FMath::RandRange(0, StarterPoints.Num() - 1)]->GetTransform();
	}

	return FTransform();
}

void AGameMode_Game::BeginPlay()
{
	Super::BeginPlay();
}
