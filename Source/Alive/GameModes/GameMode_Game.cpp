// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMode_Game.h"

#include "AliveGameState.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AGameMode_Game::AGameMode_Game()
{
	PrimaryActorTick.bCanEverTick = false;

	WarmupTime = 15;
	RoundTime = 300;
	TimeBetweenMatches = 20;
	TimeWaitEvaluate = 5;
	MaxBots = 6;
	PlayerRespawnCooldown = 5;
}

const FWeaponSpec AGameMode_Game::GenerateNewWeapon(TSubclassOf<UWeaponType> Weapon)
{
	return FWeaponSpec(Weapon.GetDefaultObject());
}

void AGameMode_Game::BeginPlay()
{
	Super::BeginPlay();
	
	AAliveGameState* GS = GetGameState<AAliveGameState>();
	GS->SetMatchState(EMatchState::Warmup);
	GS->SetRemainingTime(WarmupTime);
	
	GetWorld()->GetTimerManager().SetTimer(
		DefaultGameTimerHandle, this, &AGameMode_Game::DefaultTimerUpdate,
		GetWorldSettings()->GetEffectiveTimeDilation()/* 1 sec */, true);
}

void AGameMode_Game::DefaultTimerUpdate()
{
	AAliveGameState* GS = GetGameState<AAliveGameState>();

	if(GS->GetMatchState()==EMatchState::Evaluate)
	{
		// Game was already over. We will switch to Evaluate Map
		return;
	}
	
	if (GS->GetRemainingTime() <= 0)
	{
		switch (GS->GetMatchState())
		{
		case EMatchState::Warmup:
			GS->SetMatchState(EMatchState::Playing);
			GS->SetRemainingTime(RoundTime);
			break;
		case EMatchState::Playing:
			GS->SetMatchState(EMatchState::Finish);
			GS->SetRemainingTime(TimeWaitEvaluate);
			break;
		case EMatchState::Finish:
			GS->SetMatchState(EMatchState::Evaluate);
			break;
		default: check(false);
		}
	}
	else
	{
		GS->SetRemainingTime(GS->GetRemainingTime() - 1);
	}
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
