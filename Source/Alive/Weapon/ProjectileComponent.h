// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectileComponent.generated.h"

UENUM(BlueprintType)
enum class EBulletType : uint8
{
	/** return the hit result instantly while Firing. */
	Hitscan,
	/** Consider the velocity of the bullet. */
	Projectile,
	/** Consider both the velocity and gravity of the bullet . */
	ProjectileAndGravity
};

/**
 * Provides functional bullets based on ray detection
 */
UCLASS(ClassGroup=(Alive), hidecategories=(Object,LOD,Lighting,Transform,Sockets,TextureStreaming))
class ALIVE_API UProjectileComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectileComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;


public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class AAliveWeapon* OwningWeapon;
};
