// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliveTypes.h"
#include "GameFramework/PlayerController.h"
#include "AlivePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ALIVE_API AAlivePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAlivePlayerController();

	UFUNCTION(Client, Reliable)
	void ClientRepDamageResultAsSource(FDamageResult DamageResult);
	void ClientRepDamageResultAsSource_Implementation(FDamageResult DamageResult);
	UFUNCTION(Client, Reliable)
	void ClientRepDamageResultAsTarget(FDamageResult DamageResult);
	void ClientRepDamageResultAsTarget_Implementation(FDamageResult DamageResult);

protected:
	/** Called when the damage applied on the server, and you are the sufferer. Used to display the damage number. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, DisplayName = "OnSufferDamage")
	void K2_OnSufferDamage(const FDamageResult& DamageResult);
	/** Called when the damage applied on the server, and you are the causer. Used to display the damage number. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, DisplayName = "OnCauseDamage")
	void K2_OnCauseDamage(const FDamageResult& DamageResult);
};
