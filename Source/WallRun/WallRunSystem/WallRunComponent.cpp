// Copyright: Elixir Studios

// Game Includes
#include "WallRunComponent.h"
#include "WallRunGameplayTags.h"
#include "WallRunUtilityLibrary.h"

// Engine Includes
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UWallRunComponent::UWallRunComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UWallRunComponent::BeginPlay() {
	Super::BeginPlay();

	CharacterOwner = Cast<ACharacter>(GetOwner());
	if (CharacterOwner) {
		CharacterOwner->MovementModeChangedDelegate.AddDynamic(this, &UWallRunComponent::OnCharacterMovementModeChanged);
		CharacterOwner->OnActorHit.AddDynamic(this, &UWallRunComponent::OnCharacterHit);

		UWallRunUtilityLibrary::HandleMovementModeChanged(CharacterOwner, State,
			[this](const bool bShouldTick) {
				SetTickStatus(bShouldTick);
			},
			[this]() {
				StopWallJump();
			});
	}
}
void UWallRunComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	if (CharacterOwner) {
		CharacterOwner->MovementModeChangedDelegate.RemoveDynamic(this, &UWallRunComponent::OnCharacterMovementModeChanged);
		CharacterOwner->OnActorHit.RemoveDynamic(this, &UWallRunComponent::OnCharacterHit);
	}

	SetTickStatus(false);

	if (UWorld* World = GetWorld()) {
		UWallRunUtilityLibrary::ClearWallRunTimers(World, WallRunDurationTimerHandle, SpeedMonitorTimerHandle);

		auto& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(AirControlTimerHandle);
		TimerManager.ClearTimer(LockoutTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}
void UWallRunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	EventTick(DeltaTime);
}

bool UWallRunComponent::StartWallRun(const EWallRunSide Side) {
	if (!UWallRunUtilityLibrary::CanStartWallRun(CharacterOwner, State)) {
		return false;
	}

	UWallRunUtilityLibrary::BeginWallRun(CharacterOwner, Settings, State, Side);
	SetTickStatus(true);

	UWallRunUtilityLibrary::StartWallRunTimers(GetWorld(), Settings, WallRunDurationTimerHandle, SpeedMonitorTimerHandle,
		FTimerDelegate::CreateUObject(this, &UWallRunComponent::WallRunTimeout),
		FTimerDelegate::CreateUObject(this, &UWallRunComponent::MonitorSpeed));

	OnWallRunStarted.Broadcast(Side);
	OnWallSideChanged.Broadcast(Side);
	return true;
}
void UWallRunComponent::StopWallRun(const EMovementMode NewMovementMode, const float DisableDuration) {
	if (!State.bIsWallRunning) {
		return;
	}

	UWallRunUtilityLibrary::EndWallRun(CharacterOwner, State, NewMovementMode, DisableDuration);
	UWallRunUtilityLibrary::ClearWallRunTimers(GetWorld(), WallRunDurationTimerHandle, SpeedMonitorTimerHandle);

	if (DisableDuration > 0.0f) {
		GetWorld()->GetTimerManager().SetTimer(LockoutTimerHandle, this, &UWallRunComponent::ResetWallRunLockout, 
			DisableDuration, false);
	}

	SetTickStatus(true);
	OnWallRunEnded.Broadcast();
}

bool UWallRunComponent::StartWallJump() {
	FVector LaunchVelocity;
	if (!UWallRunUtilityLibrary::BeginWallJump(CharacterOwner, Settings, State, WallRunGameplayTags::Status_WallJumping,
			LaunchVelocity,
			[this]() {
				StopWallRun(MOVE_Falling, 0.35f);
			})) {
		return false;
	}

	if (const UWorld* World = GetWorld()) {
		World->GetTimerManager().SetTimer(AirControlTimerHandle, this, &UWallRunComponent::EnableDelayedAirControl, 
			Settings.DelayedAirControlTime, false);
	}

	OnWallJump.Broadcast(LaunchVelocity);
	return true;
}
void UWallRunComponent::StopWallJump() {
	if (!UWallRunUtilityLibrary::EndWallJump(CharacterOwner, Settings, State,WallRunGameplayTags::Status_WallJumping)) {
		return;
	}

	if (const UWorld* World = GetWorld()) {
		World->GetTimerManager().ClearTimer(AirControlTimerHandle);
	}
}

void UWallRunComponent::OnCharacterMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode) {
	UWallRunUtilityLibrary::HandleMovementModeChanged(CharacterOwner, State,
		[this](const bool bShouldTick) {
			SetTickStatus(bShouldTick);
		},
		[this]() {
			StopWallJump();
		});
}

void UWallRunComponent::OnCharacterHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
	UWallRunUtilityLibrary::HandleCharacterHit(CharacterOwner, State, Hit, LockoutTimerHandle,
		[this]() {
			StopWallRun(MOVE_Walking, 0.0f);
		},
		[this]() {
			ResetWallRunLockout();
		});
}

void UWallRunComponent::SetTickStatus(const bool bShouldTick) {
	SetComponentTickEnabled(bShouldTick);
}
void UWallRunComponent::EventTick(float DeltaTime) {
	if (!CharacterOwner || !GetWorld()) {
		return;
	}

	if (State.bIsWallRunning) {
		const EWallRunStopReason StopReason = UWallRunUtilityLibrary::TickActiveWallRun(CharacterOwner, Settings, State, DeltaTime);
		if (StopReason != EWallRunStopReason::None) {
			StopWallRun(MOVE_Falling, UWallRunUtilityLibrary::WallRunStopLockoutDuration(StopReason));
		}
		return;
	}

	UWallRunUtilityLibrary::TickWallJumpIdle(CharacterOwner, Settings, State, DeltaTime,
		[this](const bool bShouldTick) {
			SetTickStatus(bShouldTick);
		});

	if (UWallRunUtilityLibrary::TryFindWallRunStart(CharacterOwner, Settings, State, DeltaTime)) {
		StartWallRun(State.WallSide);
	}
}
void UWallRunComponent::WallRunTimeout() {
	StopWallRun(MOVE_Falling, 0.5f);
}

void UWallRunComponent::EnableDelayedAirControl() const {
	UWallRunUtilityLibrary::SetWallJumpAirControl(CharacterOwner, Settings);
}

void UWallRunComponent::MonitorSpeed() {
	if (UWallRunUtilityLibrary::UpdateSpeedMonitor(CharacterOwner, State)) {
		StopWallRun(MOVE_Falling, 0.5f);
	}
}

void UWallRunComponent::ResetWallRunLockout() {
	State.bWallRunningAllowed = true;
}

