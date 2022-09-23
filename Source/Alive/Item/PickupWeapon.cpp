// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupWeapon.h"
#include "AliveLogChannels.h"
#include "Character/AliveCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/AliveWeapon.h"
#include "Weapon/Weapon.h"
#include "Weapon/WeaponInventoryComponent.h"

APickupWeapon::APickupWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh->CastShadow = true;
	WeaponMesh->SetVisibility(true, true);
	WeaponMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	RootComponent->SetupAttachment(WeaponMesh);
	RootComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	RootComponent = WeaponMesh;
}

void APickupWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponType && HasAuthority())
	{
		InitWeapon(AWeapon::NewWeapon(this, WeaponType, GetTransform()));
	}
}

void APickupWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APickupWeapon, CurrentTransformWithVelocity, COND_None);
}

void APickupWeapon::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(APickupWeapon, CurrentTransformWithVelocity, bIsSimulatePhysics);
}

bool APickupWeapon::CanPickUp(const AAliveCharacter* Character) const
{
	return Super::CanPickUp(Character)
		&& Cast<APlayerCharacter>(Character)
		&& GetGameTimeSinceCreation() > 0.5f; // Should wait weapon pointer to be replicated.
}

void APickupWeapon::GivePickupTo(AAliveCharacter* Character)
{
	Super::GivePickupTo(Character);

	MulticastStopSimulatePhysics(CurrentTransformWithVelocity);

	this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Cast<APlayerCharacter>(Character)->GetWeaponInventoryComponent()->AddWeaponToInventory(Weapon);
	
	GetWorld()->GetTimerManager().ClearTimer(UpdateTransformTimerHandle);
}

void APickupWeapon::InitWeapon(AWeapon* InitWeapon)
{
	check(!Weapon);
	check(HasAuthority());
	check(InitWeapon);
	Weapon = InitWeapon;

	MulticastStartSimulatePhysics(Weapon->GetWeaponType());
	GetWorld()->GetTimerManager().SetTimer(UpdateTransformTimerHandle, this, &APickupWeapon::UpdateWeaponTransformAndVelocity,
	                                       GetWorldSettings()->GetEffectiveTimeDilation()/* 1s */, true, 0.1f);
}

void APickupWeapon::UpdateWeaponTransformAndVelocity()
{
	check(HasAuthority());

	CurrentTransformWithVelocity = FTransformWithVelocity(GetActorTransform(), GetVelocity());

	// When should we stop simulate. Can be non-stop if you want.
	if (GetVelocity().Size() < 10.0f && GetGameTimeSinceCreation() > 1.0f)
	{
		MulticastStopSimulatePhysics(CurrentTransformWithVelocity);
		GetWorld()->GetTimerManager().ClearTimer(UpdateTransformTimerHandle);
	}
}

void APickupWeapon::OnRep_CurrentTransformWithVelocity()
{
	if (bIsSimulatePhysics)
	{
		SetActorTransform(CurrentTransformWithVelocity.ToTransform(), false, nullptr, ETeleportType::TeleportPhysics);
		WeaponMesh->SetPhysicsLinearVelocity(CurrentTransformWithVelocity.Velocity);
	}
}

void APickupWeapon::MulticastStartSimulatePhysics_Implementation(const UWeaponType* PickupWeaponType)
{
	WeaponMesh->SetSkeletalMesh(PickupWeaponType->WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	WeaponMesh->SetSimulatePhysics(true);
	bIsSimulatePhysics = true;

	if (HasAuthority())
	{
		// Give mesh an impulse
		const FVector InitImpulse(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(200.0f, 1000.0f));
		WeaponMesh->AddImpulse(InitImpulse);
	}
}

void APickupWeapon::MulticastStopSimulatePhysics_Implementation(FTransformWithVelocity TransformWithVelocity)
{
	DisableComponentsSimulatePhysics();
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTransform(TransformWithVelocity.ToTransform());
	bIsSimulatePhysics = false;
}
