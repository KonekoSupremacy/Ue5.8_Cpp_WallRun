// No Copyright!

// Game Includes
#include "WallRunCharacter.h"
#include "WallRun.h"
#include "WallRunSystem/WallRunComponent.h"

// Engine Includes
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AWallRunCharacter::AWallRunCharacter() {
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeSize = 6.0f;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 20.0f;
	CameraBoom->CameraLagMaxDistance = 0.0f;
	CameraBoom->bUseCameraLagSubstepping = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	WallRunComponent = CreateDefaultSubobject<UWallRunComponent>("WallRunComponent");
}

void AWallRunCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AWallRunCharacter::DoJump);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWallRunCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWallRunCharacter::Look);
	}
	else {
		UE_LOG(LogWallRun, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

bool AWallRunCharacter::CanMove() const {
	if (WallRunComponent && WallRunComponent->IsWallRunning()) {
		return false;
	}
	return true;
}

void AWallRunCharacter::DoLook(const float Yaw, const float Pitch) {
	if (GetController() != nullptr) {
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AWallRunCharacter::DoJump() {
	if (WallRunComponent && WallRunComponent->IsWallRunning()) {
		WallRunComponent->StartWallJump();
		return;
	}

	Jump();
}

void AWallRunCharacter::Move(const FInputActionValue& Value) {
	if (!CanMove()) {
		return;
	}

	if (GetController() != nullptr) {
		const FVector2D MovementVector = Value.Get<FVector2D>();
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}
void AWallRunCharacter::Look(const FInputActionValue& Value) {
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}
