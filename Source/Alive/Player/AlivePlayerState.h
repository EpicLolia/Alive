// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
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

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Alive")
	int32 KillCount = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Alive")
	int32 DeathCount = 0;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Alive|PlayerState")
	UAliveAbilitySystemComponent* AbilitySystemComponent;
};
