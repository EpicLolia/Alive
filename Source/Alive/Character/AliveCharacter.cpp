// Copyright Epic Games, Inc. All Rights Reserved.

#include "AliveCharacter.h"

#include "AbilitySystem/Ability/AliveGameplayAbility.h"
#include "AbilitySystem/AliveAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/AlivePlayerState.h"

//DEFINE_LOG_CATEGORY_STATIC(LogAliveChar, Warning, All);

AAliveCharacter::AAliveCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.0f, 40.0f, 60.0f));
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	FollowCamera->bAutoActivate = true;

	// Create a FirstPersonCamera
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(20.0f, 0.0f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bAutoActivate = false;

	bHasBoundAbilityInput = false;
}

void AAliveCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AAlivePlayerState* PS = NewController->GetPlayerState<AAlivePlayerState>();
	if (PS)
	{
		AbilitySystemComponent = PS->GetAliveAbilitySystemComponent();
		
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
		
		AddDefaultAbilities();
	}
}

void AAliveCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	const AAlivePlayerState* PS = Cast<AAlivePlayerState>(GetPlayerState());
	if (PS)
	{
		AbilitySystemComponent = PS->GetAliveAbilitySystemComponent();
		
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// Bind player input to the AbilitySystemComponent.
		// Also called in SetupPlayerInputComponent because of a potential race condition.
		BindAbilityInput();
	}
}

void AAliveCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AAliveCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward/Backwards", this, &AAliveCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight/Left", this, &AAliveCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookLeft/Right", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp/Down", this, &APawn::AddControllerPitchInput);

	BindAbilityInput();
}

UAbilitySystemComponent* AAliveCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAliveCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AAliveCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AAliveCharacter::AddDefaultAbilities()
{
	if (AbilitySystemComponent && !AbilitySystemComponent->bHasDefaultAbilities)
	{
		for (TSubclassOf<UAliveGameplayAbility>& Ability : DefaultAbilities)
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(Ability, 1,
									 static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID),
									 this));
		}

		AbilitySystemComponent->bHasDefaultAbilities = true;
	}
}

void AAliveCharacter::BindAbilityInput()
{
	if (!bHasBoundAbilityInput && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent,FGameplayAbilityInputBinds(
			FString("ConfirmTarget"),
			FString("CancelTarget"),
			FString("EAbilityInputID"),
			static_cast<int32>(EAbilityInputID::Confirm),
			static_cast<int32>(EAbilityInputID::Cancel)));
		bHasBoundAbilityInput = true;
	}
}
