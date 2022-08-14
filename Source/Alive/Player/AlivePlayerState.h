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
	GENERATED_UCLASS_BODY()
	
public:
	AAlivePlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Alive|PlayerState")
	AAlivePlayerController* GetAlivePlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "Alive|PlayerState")
	UAliveAbilitySystemComponent* GetAliveAbilitySystemComponent() const { return AbilitySystemComponent; }

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Alive|PlayerState")
	UAliveAbilitySystemComponent* AbilitySystemComponent;
};
