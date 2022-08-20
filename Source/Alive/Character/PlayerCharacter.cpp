// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"

#include "AbilitySystem/AliveAbilitySystemComponent.h"
#include "AbilitySystem/Ability/AliveGameplayAbility.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/AlivePlayerState.h"

APlayerCharacter::APlayerCharacter()
	:TouchRotateRate(50.0f)
{
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // 不能朝向速度方向
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
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

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AAlivePlayerState* PS = NewController->GetPlayerState<AAlivePlayerState>();
	if (PS)
	{
		AbilitySystemComponent = PS->GetAliveAbilitySystemComponent();

		AbilitySystemComponent->InitAbilityActorInfo(PS, this);

		AddCharacterAbilities();
	}
}

void APlayerCharacter::OnRep_PlayerState()
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

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward/Backwards", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight/Left", this, &APlayerCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookLeft/Right", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp/Down", this, &APawn::AddControllerPitchInput);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &APlayerCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Repeat, this, &APlayerCharacter::TouchMoved);
	PlayerInputComponent->BindTouch(IE_Repeat, this, &APlayerCharacter::TouchStopped);

	BindAbilityInput();
}

void APlayerCharacter::MoveForward(float Value)
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

void APlayerCharacter::MoveRight(float Value)
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

void APlayerCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	if(!bIsTouching)
	{
		bIsTouching = true;
		CurrentFingerIndex = FingerIndex;
		PreviousTouchLocation = Location;
	}
}

void APlayerCharacter::TouchMoved(ETouchIndex::Type FingerIndex, FVector Location)
{
	if(FingerIndex == CurrentFingerIndex)
	{
		FVector2D ViewportSize;
		GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);
		if (Location.X > ViewportSize.X / 4) // 能在右3/4屏幕控制视角旋转
			{
			AddControllerYawInput((Location.X - PreviousTouchLocation.X) * 0.002 * TouchRotateRate);
			AddControllerPitchInput((Location.Y - PreviousTouchLocation.Y) * 0.002 * TouchRotateRate);
			}
		PreviousTouchLocation = Location;
	}
}

void APlayerCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	bIsTouching = false;
}

void APlayerCharacter::AddCharacterAbilities()
{
	if (AbilitySystemComponent && !AbilitySystemComponent->bHasCharacterAbilities)
	{
		for (TSubclassOf<UAliveGameplayAbility>& Ability : CharacterAbilities)
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(Ability, GetAbilityLevel(Ability.GetDefaultObject()->AbilityID),
				                     static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID),
				                     this));
		}
		AbilitySystemComponent->bHasCharacterAbilities = true;
	}
}

void APlayerCharacter::BindAbilityInput()
{
	if (!bHasBoundAbilityInput && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(
			                                                              FString("ConfirmTarget"),
			                                                              FString("CancelTarget"),
			                                                              FString("EAbilityInputID"),
			                                                              static_cast<int32>(EAbilityInputID::Confirm),
			                                                              static_cast<int32>(EAbilityInputID::Cancel)));
		bHasBoundAbilityInput = true;
	}
}

int32 APlayerCharacter::GetAbilityLevel(EAbilityInputID AbilityID) const
{
	// TODO: 与技能插槽(按键)绑定的技能等级系统
	return 1;
}
