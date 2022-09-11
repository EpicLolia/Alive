// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveTypes.h"
#include "GameFramework/GameStateBase.h"
#include "AliveGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimeChangedDelegate, int32, CurrentTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchStateChangedDelegate, EMatchState, CurrentMatchState);

/**
 * AAliveGameState
 *
 * This GameState are designed to be used in conjunction with GameMode_Game.
 */
UCLASS()
class ALIVE_API AAliveGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AAliveGameState();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > &OutLifetimeProps) const override;
	
	FORCEINLINE int32 GetRemainingTime() const { return RemainingTime; }
	void SetRemainingTime(int32 NewTime);
	UPROPERTY(BlueprintAssignable)
	FTimeChangedDelegate OnRemainingTimeChanged;

	FORCEINLINE EMatchState GetMatchState() const { return MatchState; }
	void SetMatchState(EMatchState NewState);
	UPROPERTY(BlueprintAssignable)
	FMatchStateChangedDelegate OnMatchStateChanged;
protected:
	/** time left for warmup / match */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_RemainingTime)
	int32 RemainingTime;
	UFUNCTION()
	void OnRep_RemainingTime();

	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = OnRep_MatchState)
	EMatchState MatchState;
	UFUNCTION()
	void OnRep_MatchState();
};
