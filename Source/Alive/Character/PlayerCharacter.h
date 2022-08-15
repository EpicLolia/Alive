// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Alive.h"
#include "AliveCharacter.h"
#include "GameFramework/Character.h"

#include "PlayerCharacter.generated.h"

UCLASS()
class ALIVE_API APlayerCharacter : public AAliveCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

protected:
	virtual void BeginPlay() override;
	// Server Only
	virtual void PossessedBy(AController* NewController) override;
	// Client Only
	virtual void OnRep_PlayerState() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

private:
	void MoveForward(float Val);
	void MoveRight(float Val);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	// 需要在添加技能时绑定按键,TODO:并接入技能等级系统
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
