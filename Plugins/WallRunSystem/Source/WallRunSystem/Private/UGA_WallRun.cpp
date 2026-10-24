// Copyright: KonekoSupremacy

// Game Includes
#include "UGA_WallRun.h"
#include "WallRunInputProvider.h"
#include "WallRunGameplayTags.h"
#include "WallRunUtilityLibrary.h"

// Engine Includes
#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UGA_WallRun::UGA_WallRun() {
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	WallJumpingTag = WallRunGameplayTags::Status_WallJumping;
	TagsBlockingWallRun.AddTag(WallRunGameplayTags::Status_BlockingAction_WallRun);
}

void UGA_WallRun::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CharacterAvatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!CharacterAvatar) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CharacterAvatar->MovementModeChangedDelegate.AddDynamic(this, &UGA_WallRun::OnCharacterMovementModeChanged);
	CharacterAvatar->OnActorHit.AddDynamic(this, &UGA_WallRun::OnCharacterHit);

	UWallRunUtilityLibrary::HandleMovementModeChanged(CharacterAvatar, State,
		[this](const bool bShouldTick) {
			SetTickStatus(bShouldTick);
		},
		[this]() {
			StopWallJump();
		});

	if (!bJumpInputBound) {
		const UInputAction* JumpAction = JumpInputAction;
		if (!JumpAction) {
			if (const IWallRunInputProvider* InputProvider = Cast<IWallRunInputProvider>(CharacterAvatar)) {
				JumpAction = InputProvider->GetWallRunJumpAction();
			}
		}
		if (JumpAction) {
			if (UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(CharacterAvatar->InputComponent)) {
				InputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &UGA_WallRun::OnJumpInputPressed);
				bJumpInputBound = true;
			}
		}
	}
}
void UGA_WallRun::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) {
	if (CharacterAvatar) {
		CharacterAvatar->MovementModeChangedDelegate.RemoveDynamic(this, &UGA_WallRun::OnCharacterMovementModeChanged);
		CharacterAvatar->OnActorHit.RemoveDynamic(this, &UGA_WallRun::OnCharacterHit);
	}

	SetTickStatus(false);

	if (UWorld* World = GetWorld()) {
		UWallRunUtilityLibrary::ClearWallRunTimers(World, WallRunDurationTimerHandle, SpeedMonitorTimerHandle);

		auto& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(AirControlTimerHandle);
		TimerManager.ClearTimer(LockoutTimerHandle);
	}

	if (State.bIsWallRunning) {
		StopWallRun(MOVE_Falling, 0.0f);
	}

	if (State.bIsWallJumping) {
		StopWallJump();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGA_WallRun::StartWallRun(const EWallRunSide Side) {
	if (!UWallRunUtilityLibrary::CanStartWallRun(CharacterAvatar, State, GetAbilitySystemComponentFromActorInfo(),
			&TagsBlockingWallRun)) {
		return false;
	}

	UWallRunUtilityLibrary::BeginWallRun(CharacterAvatar, Settings, State, Side);
	SetTickStatus(true);

	UWallRunUtilityLibrary::StartWallRunTimers(GetWorld(), Settings, WallRunDurationTimerHandle, SpeedMonitorTimerHandle,
		FTimerDelegate::CreateUObject(this, &UGA_WallRun::WallRunTimeout),
		FTimerDelegate::CreateUObject(this, &UGA_WallRun::MonitorSpeed));
	return true;
}
void UGA_WallRun::StopWallRun(const EMovementMode NewMovementMode, const float DisableDuration) {
	if (!State.bIsWallRunning) {
		return;
	}

	UWallRunUtilityLibrary::EndWallRun(CharacterAvatar, State, NewMovementMode, DisableDuration);
	UWallRunUtilityLibrary::ClearWallRunTimers(GetWorld(), WallRunDurationTimerHandle, SpeedMonitorTimerHandle);

	if (DisableDuration > 0.0f) {
		GetWorld()->GetTimerManager().SetTimer(LockoutTimerHandle, this, &UGA_WallRun::ResetWallRunLockout, 
			DisableDuration, false);
	}

	SetTickStatus(true);
}

bool UGA_WallRun::StartWallJump() {
	FVector LaunchVelocity;
	if (!UWallRunUtilityLibrary::BeginWallJump(CharacterAvatar, Settings, State, WallJumpingTag, LaunchVelocity,
			[this]() {
				StopWallRun(MOVE_Falling, 0.35f);
			})) {
		return false;
	}

	if (const UWorld* World = GetWorld()) {
		World->GetTimerManager().SetTimer(AirControlTimerHandle, this, &UGA_WallRun::EnableDelayedAirControl, 
			Settings.DelayedAirControlTime, false);
	}

	return true;
}
void UGA_WallRun::StopWallJump() {
	if (!UWallRunUtilityLibrary::EndWallJump(CharacterAvatar, State, WallJumpingTag)) {
		return;
	}

	if (const UWorld* World = GetWorld()) {
		World->GetTimerManager().ClearTimer(AirControlTimerHandle);
	}
}

void UGA_WallRun::OnJumpInputPressed() {
	if (IsWallRunning()) {
		StartWallJump();
	}
}

void UGA_WallRun::OnCharacterMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode) {
	UWallRunUtilityLibrary::HandleMovementModeChanged(CharacterAvatar, State,
		[this](const bool bShouldTick) {
			SetTickStatus(bShouldTick);
		},
		[this]() {
			StopWallJump();
		});
}

void UGA_WallRun::OnCharacterHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
	UWallRunUtilityLibrary::HandleCharacterHit(CharacterAvatar, State, Hit, LockoutTimerHandle,
		[this]() {
			StopWallRun(MOVE_Walking, 0.0f);
		},
		[this]() {
			ResetWallRunLockout();
		});
}

void UGA_WallRun::SetTickStatus(const bool bShouldTick) {
	bTickEnabled = bShouldTick;
}
void UGA_WallRun::TickAbility(const float DeltaTime) {
	if (!bTickEnabled || !CharacterAvatar || !GetWorld()) {
		return;
	}

	if (State.bIsWallRunning) {
		const EWallRunStopReason StopReason = UWallRunUtilityLibrary::TickActiveWallRun(CharacterAvatar, Settings, State, DeltaTime);
		if (StopReason != EWallRunStopReason::None) {
			StopWallRun(MOVE_Falling, UWallRunUtilityLibrary::WallRunStopLockoutDuration(StopReason));
		}
		return;
	}

	UWallRunUtilityLibrary::TickWallJumpIdle(CharacterAvatar, Settings, State, DeltaTime,
		[this](const bool bShouldTick) {
			SetTickStatus(bShouldTick);
		});

	if (UWallRunUtilityLibrary::TryFindWallRunStart(CharacterAvatar, Settings, State, DeltaTime)) {
		StartWallRun(State.WallSide);
	}
}
void UGA_WallRun::WallRunTimeout() {
	StopWallRun(MOVE_Falling, 0.5f);
}
void UGA_WallRun::EnableDelayedAirControl() const {
	UWallRunUtilityLibrary::SetWallJumpAirControl(CharacterAvatar, Settings);
}
void UGA_WallRun::MonitorSpeed() {
	if (UWallRunUtilityLibrary::UpdateSpeedMonitor(CharacterAvatar, State)) {
		StopWallRun(MOVE_Falling, 0.5f);
	}
}
void UGA_WallRun::ResetWallRunLockout() {
	State.bWallRunningAllowed = true;
}
