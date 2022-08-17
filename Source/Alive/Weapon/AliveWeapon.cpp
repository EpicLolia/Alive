// Fill out your copyright notice in the Description page of Project Settings.

#include "AliveWeapon.h"

#include "AbilitySystem/AliveAbilitySystemComponent.h"
#include "AbilitySystem/Ability/AliveGameplayAbility.h"
#include "AbilitySystem/Ability/TargetActor/TA_LineTrace.h"
#include "Character/AliveCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

AAliveWeapon::AAliveWeapon()
{
	// Network Setting
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	NetUpdateFrequency = 100.0f; // Set this to a value that's appropriate for your game

	// Ammo
	PrimaryClipAmmo = 30;
	MaxPrimaryClipAmmo = 30;
	PrimaryAmmoType = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));

	// Collision
	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionComponent"));
	CollisionComp->InitCapsuleSize(40.0f, 50.0f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = CollisionComp;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetupAttachment(CollisionComp);
	WeaponMesh->SetRelativeTransform(WeaponMeshRelativeTransform);
	WeaponMesh->CastShadow = true;
	WeaponMesh->SetVisibility(true, true);
	WeaponMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;

	FireMode = FGameplayTag::RequestGameplayTag("Weapon.FireMode.None");
	WeaponIsFiringTag = FGameplayTag::RequestGameplayTag("Weapon.IsFiring");
}

void AAliveWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAliveWeapon, PrimaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AAliveWeapon, MaxPrimaryClipAmmo, COND_OwnerOnly);
}

void AAliveWeapon::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(AAliveWeapon, PrimaryClipAmmo,
	                              (IsValid(AbilitySystemComponent) && !AbilitySystemComponent->HasMatchingGameplayTag(
		                              WeaponIsFiringTag)));
}

void AAliveWeapon::NotifyActorBeginOverlap(AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	// TODO: Pickup on touch
}

void AAliveWeapon::SetOwningCharacter(AAliveCharacter* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
	if (OwningCharacter)
	{
		// Called when added to inventory
		AbilitySystemComponent = Cast<UAliveAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());
		SetOwner(InOwningCharacter);
		AttachToComponent(OwningCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Hide if not equip
		if (OwningCharacter->GetCurrentWeapon() != this)
		{
			WeaponMesh->CastShadow = false;
			WeaponMesh->SetVisibility(true, true);
			WeaponMesh->SetVisibility(false, true);
		}
	}
	else
	{
		AbilitySystemComponent = nullptr;
		SetOwner(nullptr);
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void AAliveWeapon::Equip()
{
	WeaponMesh->CastShadow = true;
	WeaponMesh->SetVisibility(true, true);
	AddAbilitiesOnServer();
}

void AAliveWeapon::UnEquip()
{
	WeaponMesh->CastShadow = false;
	WeaponMesh->SetVisibility(false, true);
	RemoveAbilitiesOnServer();
}

void AAliveWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AAliveWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (LineTraceTargetActor)
	{
		LineTraceTargetActor->Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* AAliveWeapon::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAliveWeapon::AddAbilitiesOnServer()
{
	if (!IsValid(OwningCharacter) ||
		!OwningCharacter->GetAbilitySystemComponent() ||
		GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	UAliveAbilitySystemComponent* ASC =
		Cast<UAliveAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());
	if (ASC)
	{
		for (TSubclassOf<UAliveGameplayAbility>& Ability : WeaponAbilities)
		{
			WeaponAbilitySpecHandles.Add(ASC->GiveAbility(
				FGameplayAbilitySpec(Ability, GetWeaponAbilityLevel(),
				                     static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID),
				                     this)));
		}
	}
}

void AAliveWeapon::RemoveAbilitiesOnServer()
{
	if (!IsValid(OwningCharacter) ||
		!OwningCharacter->GetAbilitySystemComponent() ||
		GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	
	UAliveAbilitySystemComponent* ASC =
		Cast<UAliveAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());
	if (ASC)
	{
		for (FGameplayAbilitySpecHandle& SpecHandle : WeaponAbilitySpecHandles)
		{
			ASC->ClearAbility(SpecHandle);
		}
		WeaponAbilitySpecHandles.Reset();
	}
}

int32 AAliveWeapon::GetWeaponAbilityLevel()
{
	// TODO
	return 1;
}

void AAliveWeapon::OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo)
{
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
}

void AAliveWeapon::OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo)
{
	OnMaxPrimaryClipAmmoChanged.Broadcast(OldMaxPrimaryClipAmmo, MaxPrimaryClipAmmo);
}
