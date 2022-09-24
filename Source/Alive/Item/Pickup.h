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

	class USphereComponent* GetCollisionComponent() const { return CollisionComp; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void TryToPickItUp(AAliveCharacter* Character);

	// Only called on the server. Used to notify the pickup generator.
	FSimpleDelegate OnPickUpOrTimeOut;

protected:
	virtual void BeginPlay() override;

	virtual bool CanPickUp(const AAliveCharacter* Character) const;
	virtual void GivePickupTo(AAliveCharacter* Character);

	UFUNCTION(BlueprintImplementableEvent, DisplayName="OnPickUpEvent", Category="Alive|Pickup")
	void K2_OnPickUpEvent();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pickup")
	TArray<TSubclassOf<class UGameplayEffect>> EffectClasses;

private:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pickup", meta=(AllowPrivateAccess = true))
	class USphereComponent* CollisionComp;


	UFUNCTION(NetMulticast, Reliable)
	void MulticastPickUpEvent();
	void MulticastPickUpEvent_Implementation();

	// Only trigger once
	bool bHasBeenTriggered = false;
};
