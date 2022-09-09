// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveGameMode.h"
#include "GameMode_Game.generated.h"

UCLASS()
class ALIVE_API AGameMode_Game : public AAliveGameMode
{
	GENERATED_BODY()

public:
	AGameMode_Game();

protected:
	/** delay between first player login and starting match */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	int32 WarmupTime;

	/** match duration */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	int32 RoundTime;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	int32 TimeBetweenMatches;

	/** score for kill */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	int32 KillScore;

	/** score for death */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	int32 DeathScore;

	/** scale for self instigated damage */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	float DamageSelfScale;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	int32 MaxBots;


	virtual void BeginPlay() override;
};
