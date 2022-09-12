// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Alive.h"
#include "AliveCharacter.h"
#include "GameFramework/Character.h"

#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UPlayerInventoryComponent;
UCLASS()
class ALIVE_API APlayerCharacter : public AAliveCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter(const class FObjectInitializer& ObjectInitializer);

	virtual bool CanAddWeapon(AAliveWeapon* Weapon) const override;
	virtual void AddWeapon(AAliveWeapon* Weapon) override;
	
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UPlayerInventoryComponent* GetPlayerInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	class UAliveCharacterMovementComponent* GetAliveCharacterMovementComponent();
protected:
	virtual void BeginPlay() override;
	// Server Only
	virtual void PossessedBy(AController* NewController) override;
	// Client Only
	virtual void OnRep_PlayerState() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void OnDeath(AActor* DamageInstigator) override;

protected:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Alive|Input")
	float TouchRotateRate;

	// Used to add Widget to local screen
	UFUNCTION(BlueprintImplementableEvent)
	void OnPossessedLocally(APlayerController* LocalController);

private:
	void MoveForward(float Val);
	void MoveRight(float Val);
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchMoved(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	FVector PreviousTouchLocation;
	ETouchIndex::Type CurrentFingerIndex;
	bool bIsTouching = false;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UPlayerInventoryComponent* InventoryComponent;

	// Should bind input while giving ability, TODO: Ability Level
	virtual void AddCharacterAbilities() override final;

	// Called from both SetupPlayerInputComponent and OnRep_PlayerState because of a potential race condition where the PlayerController might
	// call ClientRestart which calls SetupPlayerInputComponent before the PlayerState is repped to the client so the PlayerState would be null in SetupPlayerInputComponent.
	// Conversely, the PlayerState might be repped before the PlayerController calls ClientRestart so the Actor's InputComponent would be null in OnRep_PlayerState.
	void BindAbilityInput();

	// Only Bind Once
	bool bHasBoundAbilityInput;

protected:
	// Switch on AbilityID to return individual ability levels.
	UFUNCTION(BlueprintCallable, Category = "Alive|PlayerCharacter")
	int32 GetAbilityLevel(EAbilityInputID AbilityID) const;
};
