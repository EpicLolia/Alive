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

	FTransform GetRandomSpawnTransform(FName Tag = FName("None")) const;
	FORCEINLINE float GetPlayerRespawnCooldown() const { return PlayerRespawnCooldown; }
protected:
	virtual void BeginPlay() override;

protected:
	/** delay between first player login and starting match */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	float WarmupTime;

	/** match duration */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	float RoundTime;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	float TimeBetweenMatches;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	int32 MaxBots;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSetting")
	float PlayerRespawnCooldown;
};
