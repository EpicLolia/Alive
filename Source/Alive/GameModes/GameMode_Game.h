// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveGameMode.h"
#include "Weapon/WeaponSpec.h"
#include "GameMode_Game.generated.h"

UCLASS()
class ALIVE_API AGameMode_Game : public AAliveGameMode
{
	GENERATED_BODY()

public:
	AGameMode_Game();

	FTransform GetRandomSpawnTransform(FName Tag = FName("None")) const;
	FORCEINLINE int32 GetPlayerRespawnCooldown() const { return PlayerRespawnCooldown; }

	UFUNCTION(BlueprintCallable)
	const FWeaponSpec GenerateNewWeapon(TSubclassOf<UWeaponType> Weapon);
	
protected:
	virtual void BeginPlay() override;

protected:
	/** delay between first player login and starting match */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSettings")
	int32 WarmupTime;

	/** match duration */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSettings")
	int32 RoundTime;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSettings")
	int32 TimeBetweenMatches;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSettings")
	int32 TimeWaitEvaluate;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSettings|Player")
	int32 PlayerRespawnCooldown;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive|GameSettings")
	int32 MaxBots;

private:
	void DefaultTimerUpdate();
	FTimerHandle DefaultGameTimerHandle;
};
