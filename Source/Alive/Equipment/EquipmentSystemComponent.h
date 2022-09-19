// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentSystemComponent.generated.h"

UCLASS(ClassGroup=(Alive), meta=(BlueprintSpawnableComponent))
class ALIVE_API UEquipmentSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentSystemComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// UPROPERTY(ReplicatedUsing = OnRep_Equipments)
	// TArray<FEquipmentSpec> Equipments;
	// UFUNCTION()
	// void OnRep_Equipments();
	
};
