// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupWeapon.h"
#include "AliveLogChannels.h"
#include "Character/AliveCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Character/PlayerInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/AliveWeapon.h"

APickupWeapon::APickupWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	WeaponToSpawn = AAliveWeapon::StaticClass();
}

void APickupWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		Weapon = GetWorld()->SpawnActor<AAliveWeapon>(WeaponToSpawn, GetTransform());
		check(Weapon);

		this->AttachToActor(Weapon, FAttachmentTransformRules::KeepRelativeTransform);
		this->SetActorRelativeTransform(FTransform());

		// Wait Weapon pointer to be replicated.
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APickupWeapon::MulticastStartSimulatePhysics);

		GetWorld()->GetTimerManager().SetTimer(UpdateTransformTimerHandle, this, &APickupWeapon::UpdateWeaponTransformAndVelocity,
		                                       GetWorldSettings()->GetEffectiveTimeDilation()/* 1s */, true, 0.1f);
	}
}

void APickupWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APickupWeapon, Weapon, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(APickupWeapon, CurrentTransformWithVelocity, COND_None);
}

void APickupWeapon::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(APickupWeapon, CurrentTransformWithVelocity, bIsSimulatePhysics);
}

bool APickupWeapon::CanPickUp(const AAliveCharacter* Character) const
{
	// TODO: Limit on the number of weapons
	return Super::CanPickUp(Character)
		&& Character->CanAddWeapon(Weapon)
		&& GetGameTimeSinceCreation() > 0.5f; // Should wait weapon pointer to be replicated.
}

void APickupWeapon::GivePickupTo(AAliveCharacter* Character)
{
	Super::GivePickupTo(Character);

	MulticastStopSimulatePhysics(CurrentTransformWithVelocity);

	this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Character->AddWeapon(Weapon);

	UpdateTransformTimerHandle.Invalidate();
	//Weapon = nullptr; // Should I do this?
}

void APickupWeapon::OnPickUpEvent()
{
	Weapon->SetWeaponVisibility(false);
}

void APickupWeapon::UpdateWeaponTransformAndVelocity()
{
	check(HasAuthority());

	CurrentTransformWithVelocity = FTransformWithVelocity(Weapon->GetActorTransform(), Weapon->GetVelocity());

	// When should we stop simulate. Can be non-stop if you want.
	if (Weapon->GetVelocity().Size() < 10.0f && GetGameTimeSinceCreation() > 1.0f)
	{
		MulticastStopSimulatePhysics(CurrentTransformWithVelocity);
		UpdateTransformTimerHandle.Invalidate();
	}
}

void APickupWeapon::OnRep_CurrentTransformWithVelocity()
{
	if (bIsSimulatePhysics)
	{
		Weapon->SetActorTransform(CurrentTransformWithVelocity.ToTransform(), false, nullptr, ETeleportType::TeleportPhysics);
		Weapon->GetWeaponMesh()->SetPhysicsLinearVelocity(CurrentTransformWithVelocity.Velocity);
	}
}

void APickupWeapon::MulticastStartSimulatePhysics_Implementation()
{
	check(Weapon);

	Weapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Weapon->GetWeaponMesh()->SetSimulatePhysics(true);
	bIsSimulatePhysics = true;

	if (HasAuthority())
	{
		// Give mesh an impulse
		const FVector InitImpulse(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(200.0f, 1000.0f));
		Weapon->GetWeaponMesh()->AddImpulse(InitImpulse);
	}
}

void APickupWeapon::MulticastStopSimulatePhysics_Implementation(FTransformWithVelocity TransformWithVelocity)
{
	check(Weapon);
	Weapon->DisableComponentsSimulatePhysics();
	Weapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Weapon->SetActorTransform(TransformWithVelocity.ToTransform());
	bIsSimulatePhysics = false;
}
