// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Character/PlayerCharacter.h"
#include "GameFramework/PlayerState.h"
#include "AlivePlayerState.generated.h"

class AAlivePlayerController;
class UAliveAbilitySystemComponent;

UCLASS()
class ALIVE_API AAlivePlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAlivePlayerState();

	UFUNCTION(BlueprintCallable, Category = "Alive|PlayerState")
	AAlivePlayerController* GetAlivePlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "Alive|PlayerState")
	UAliveAbilitySystemComponent* GetAliveAbilitySystemComponent() const { return AbilitySystemComponent; }

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Alive|PlayerState")
	UAliveAbilitySystemComponent* AbilitySystemComponent;

public:
	/**
	 * Set new team and update pawn. Also updates player character team colors.
	 *
	 * @param	NewTeamNumber	Team we want to be on.
	 */
	void SetTeamNum(int32 NewTeamNumber);
	FORCEINLINE int32 GetTeamNum() const { return TeamNumber; }

	TSubclassOf<APlayerCharacter> GetCharacterType() const { return CharacterType; }

	void AddKillCount() { ++KillCount; }
	void AddDeathCount() { ++DeathCount; }

protected:
	// Used to spawn player's character on the server.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Alive")
	TSubclassOf<APlayerCharacter> CharacterType;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alive")
	int32 KillCount;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alive")
	int32 DeathCount;

	/** Team number. */
	UPROPERTY(Transient, Replicated)
	int32 TeamNumber;

	// Local Data. Won't be replicated.
	/** number of bullets fired this match */
	UPROPERTY()
	int32 NumBulletsFired;
	/** number of rockets fired this match */
	UPROPERTY()
	int32 NumRocketsFired;
	/** number of rockets fired this match */
	UPROPERTY()
	int32 NumGrenadesThrown;
};
