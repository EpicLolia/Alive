// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class AAliveCharacter;

/**
 * The Pickup Actor should be spawned by server
 */
UCLASS()
class ALIVE_API APickup : public AActor
{
	GENERATED_BODY()

public:
	APickup();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void TryToPickItUp(AAliveCharacter* Character);

protected:
	virtual bool CanPickUp(const AAliveCharacter* Character) const;
	virtual void GivePickupTo(AAliveCharacter* Character);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnPickUpEvent();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pickup")
	TArray<TSubclassOf<class UGameplayEffect>> EffectClasses;

private:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pickup", meta=(AllowPrivateAccess = true))
	class USphereComponent* CollisionComp;
	
	UFUNCTION(NetMulticast,Unreliable)
	void NetMulticast_PickUpEvent();
	void NetMulticast_PickUpEvent_Implementation();

	// Only trigger once
	bool bHasBeenTriggered = false;
};
