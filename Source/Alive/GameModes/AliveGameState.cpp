// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/AliveGameState.h"

#include "Net/UnrealNetwork.h"

AAliveGameState::AAliveGameState()
{
	RemainingTime = 0;
	MatchState = EMatchState::Warmup;
}

void AAliveGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAliveGameState, RemainingTime, COND_None);
	DOREPLIFETIME_CONDITION(AAliveGameState, MatchState, COND_None);
}

void AAliveGameState::SetRemainingTime(int32 NewTime)
{
	if (HasAuthority())
	{
		RemainingTime = NewTime;
		OnRemainingTimeChanged.Broadcast(NewTime);
	}
}

void AAliveGameState::SetMatchState(EMatchState NewState)
{
	if (HasAuthority())
	{
		MatchState = NewState;
		OnMatchStateChanged.Broadcast(NewState);
	}
}

void AAliveGameState::OnRep_RemainingTime()
{
	OnRemainingTimeChanged.Broadcast(RemainingTime);
}

void AAliveGameState::OnRep_MatchState()
{
	OnMatchStateChanged.Broadcast(MatchState);
}
