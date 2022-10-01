// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"

#include "AbilitySystemComponent.h"
#include "ProjectileComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Character/AliveCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Player/AlivePlayerState.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	// Caution! Only replicated to owner. 
	//bOnlyRelevantToOwner = true;

	ProjectileComponent = CreateDefaultSubobject<UProjectileComponent>(FName("ProjectileComponent"));

	bUseProjectileActor = false;
	SetActorTickInterval(1);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (bUseProjectileActor)
	{
		ProjectileComponent->SetActive(false);
	}
	else
	{
		ProjectileComponent->InitProjectileComponent();
	}
}

AWeapon* AWeapon::NewWeapon(const AActor* GenerateInstigator, TSubclassOf<UWeaponType> WeaponTypeClass, const FTransform& Transform)
{
	check(GenerateInstigator);
	check(GenerateInstigator->HasAuthority());
	AWeapon* NewWeapon = GenerateInstigator->GetWorld()->SpawnActor<AWeapon>(AWeapon::StaticClass(), Transform);
	NewWeapon->SetWeaponType(WeaponTypeClass);
	return NewWeapon;
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, WeaponType, COND_InitialOnly)
	DOREPLIFETIME_CONDITION(AWeapon, CurrentAmmo, COND_OwnerOnly)
}

void AWeapon::AddTo(AAliveCharacter* Character)
{
	// Will not add the actor to the weapon array here,
	// because the owner may not have the inventory component, like monster.
	check(Character);
	SetOwner(Character);
	GrantAbilitiesToOwner();
}

void AWeapon::DiscardFromOwner()
{
	check(GetOwner());
	RemoveAbilitiesFromOwner();
}

FVector AWeapon::GetFirePointWorldLocation() const
{
	// TODO: 这块的逻辑不对，还是在weapon里维护一个mesh吧，把visibility作为变量同步，这样就可以解决断线重连的问题
	return Cast<APlayerCharacter>(GetOwnerAsAliveCharacter())->GetWeaponMeshComponent()->
	                                                           GetSocketLocation(GetWeaponType()->FirePointSocket);
}

FWeaponPerformance AWeapon::GenerateWeaponPerformance() const
{
	if (GetOwnerAsAliveCharacter())
	{
		if (AAlivePlayerState* PS = Cast<AAlivePlayerState>(GetOwnerAsAliveCharacter()->GetPlayerState()))
		{
			// TODO: Custom weapon performance
			return FWeaponPerformance(WeaponType);
		}
	}
	return FWeaponPerformance(WeaponType);
}

void AWeapon::SetCurrentAmmo(int32 Ammo)
{
	CurrentAmmo = FMath::Clamp(Ammo, 0, WeaponType->MaxClipAmmo);
	OnCurrentAmmoChanged.ExecuteIfBound();
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::SetWeaponType(TSubclassOf<UWeaponType> WeaponTypeClass)
{
	WeaponType = WeaponTypeClass.GetDefaultObject();
	CurrentAmmo = WeaponType->MaxClipAmmo;
	OnRep_WeaponType();
}

void AWeapon::OnRep_WeaponType()
{
	ProjectileComponent->InitProjectileComponent();
}

void AWeapon::OnRep_CurrentAmmo(int32 OldAmmo)
{
	if (OldAmmo != CurrentAmmo)
	{
		OnCurrentAmmoChanged.ExecuteIfBound();
	}
}

AAliveCharacter* AWeapon::GetOwnerAsAliveCharacter() const
{
	return Cast<AAliveCharacter>(GetOwner());
}

void AWeapon::GrantAbilitiesToOwner()
{
	check(HasAuthority());
	check(GetOwnerAsAliveCharacter());
	check(AbilitySpecHandles.Num() == 0);

	for (const TSubclassOf<UGameplayAbility>& Ability : WeaponType->AbilitiesGrantedToOwner)
	{
		AbilitySpecHandles.Add(GetOwnerAsAliveCharacter()->GetAbilitySystemComponent()->GiveAbility(
			FGameplayAbilitySpec(Ability, 1, INDEX_NONE, this)));
	}
}

void AWeapon::RemoveAbilitiesFromOwner()
{
	check(HasAuthority());
	check(GetOwnerAsAliveCharacter());

	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
	{
		GetOwnerAsAliveCharacter()->GetAbilitySystemComponent()->ClearAbility(SpecHandle);
	}
	AbilitySpecHandles.Reset();
}
