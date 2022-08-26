// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class AAliveCharacter;

UCLASS()
class ALIVE_API APickup : public AActor
{
	GENERATED_BODY()

public:
	APickup();

	// Should be called on server only 
	UFUNCTION(BlueprintCallable)
	void TryToPickItUp(AAliveCharacter* Character);

protected:
	virtual void BeginPlay() override;

	// Pickup Collision Trigger
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	class USphereComponent* CollisionComp;
	
	// Sound played when player picks it up
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pickup")
	class USoundCue* PickupSound;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pickup")
	TArray<TSubclassOf<class UGameplayEffect>> EffectClasses;

protected:
	virtual bool CanBePickedUp(const AAliveCharacter* Character) const;
	// Only work on server
	virtual void GivePickupTo(AAliveCharacter* Character);
};
