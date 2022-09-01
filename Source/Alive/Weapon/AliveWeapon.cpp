// Fill out your copyright notice in the Description page of Project Settings.

#include "AliveWeapon.h"

#include "ProjectileComponent.h"
#include "AbilitySystem/AliveAbilitySystemComponent.h"
#include "AbilitySystem/Ability/AliveGameplayAbility.h"
#include "Character/AliveCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

AAliveWeapon::AAliveWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	// Network Setting
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	NetUpdateFrequency = 100.0f; // Set this to a value that's appropriate for your game

	// Ammo
	PrimaryClipAmmo = 30;
	MaxPrimaryClipAmmo = 30;
	PrimaryCartridgeAmmo = 1;
	PrimaryAmmoType = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));
	MaxSpreadAngle = 15.0f;

	ProjectileComponent = CreateDefaultSubobject<UProjectileComponent>(FName("ProjectileComponent"));

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetRelativeTransform(WeaponMeshRelativeTransform);
	WeaponMesh->CastShadow = true;
	WeaponMesh->SetVisibility(true, true);
	WeaponMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	RootComponent = WeaponMesh;

	DefaultFireMode = FGameplayTag::RequestGameplayTag(FName("Weapon.Rifle.FireMode.FullAuto"));
	FireMode = FGameplayTag::RequestGameplayTag("Weapon.Rifle.FireMode.FullAuto");
	WeaponIsFiringTag = FGameplayTag::RequestGameplayTag("Weapon.IsFiring");
}

void AAliveWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateSpread(DeltaSeconds);
}


void AAliveWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAliveWeapon, PrimaryClipAmmo, COND_OwnerOnly);
}

void AAliveWeapon::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	// Prevent the gun from replicating back the ammo account and clobbering the local ammo amount during automatic fire.
	// Essentially doing local prediction here

	DOREPLIFETIME_ACTIVE_OVERRIDE(AAliveWeapon, PrimaryClipAmmo,
	                              (IsValid(AbilitySystemComponent) && !AbilitySystemComponent->HasMatchingGameplayTag(
		                              WeaponIsFiringTag)));
}

void AAliveWeapon::SetOwningCharacter(AAliveCharacter* InOwningCharacter)
{
	check(HasAuthority());

	OwningCharacter = InOwningCharacter;
	if (OwningCharacter)
	{
		AbilitySystemComponent = Cast<UAliveAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());
		SetOwner(InOwningCharacter);
		AttachToComponent(OwningCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform,
		                  OwningCharacter->GetWeaponSocket());
		SetActorRelativeTransform(WeaponMeshRelativeTransform);

		AddAbilities();
	}
	else
	{
		AbilitySystemComponent = nullptr;
		SetOwner(nullptr);
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void AAliveWeapon::RemoveFormOwningCharacter()
{
	check(HasAuthority());

	RemoveAbilities();

	AbilitySystemComponent = nullptr;
	SetOwner(nullptr);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	OwningCharacter = nullptr;
}

void AAliveWeapon::SetWeaponVisibility(bool bWeaponVisibility) const
{
	WeaponMesh->CastShadow = bWeaponVisibility;
	WeaponMesh->SetVisibility(bWeaponVisibility, true);
}

FVector AAliveWeapon::GetFirePointWorldLocation() const
{
	return WeaponMesh->GetSocketLocation(FirePointSocket);
}

void AAliveWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AAliveWeapon::SetPrimaryClipAmmo(int32 Ammo)
{
	int32 OldPrimaryClipAmmo = PrimaryClipAmmo;
	PrimaryClipAmmo = FMath::Clamp(Ammo, 0, MaxPrimaryClipAmmo);
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
}

void AAliveWeapon::AddSpread()
{
	CurrentSpreadAngle += FMath::Max((MaxSpreadAngle - CurrentSpreadAngle) * 0.4f, 0.0f);
}

UAbilitySystemComponent* AAliveWeapon::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAliveWeapon::AddAbilities()
{
	check(HasAuthority());

	if (!IsValid(OwningCharacter))
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

void AAliveWeapon::RemoveAbilities()
{
	check(HasAuthority());

	if (!IsValid(OwningCharacter))
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

int32 AAliveWeapon::GetWeaponAbilityLevel() const
{
	// TODO
	return 1;
}

void AAliveWeapon::OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo)
{
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
}

void AAliveWeapon::UpdateSpread(float DeltaSeconds)
{
	CurrentSpreadAngle = FMath::Max(CurrentSpreadAngle - DeltaSeconds * MaxSpreadAngle, 0.0f);
}
