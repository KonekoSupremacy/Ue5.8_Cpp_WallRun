// No Copyright!

// Game Includes
#include "WallRunComponent.h"
#include "WallRunGameplayTags.h"
#include "WallRunUtilityLibrary.h"

// Engine Includes
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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

		if (const UCharacterMovementComponent* CMC = CharacterOwner->GetCharacterMovement()) {
			if (CMC->MovementMode == MOVE_Falling) {
				SetTickStatus(true);
				State.PreviousVelocity = CharacterOwner->GetVelocity();
			}
		}
	}
}
void UWallRunComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	if (CharacterOwner) {
		CharacterOwner->MovementModeChangedDelegate.RemoveDynamic(this, &UWallRunComponent::OnCharacterMovementModeChanged);
		CharacterOwner->OnActorHit.RemoveDynamic(this, &UWallRunComponent::OnCharacterHit);
	}

	SetTickStatus(false);

	if (const UWorld* World = GetWorld()) {
		auto& TimerManager = World->GetTimerManager(); 
		TimerManager.ClearTimer(WallRunDurationTimerHandle);
		TimerManager.ClearTimer(SpeedMonitorTimerHandle);
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
	if (!State.bWallRunningAllowed || !CharacterOwner || State.bIsWallRunning) {
		return false;
	}

	if (!CharacterOwner->GetCharacterMovement() || CharacterOwner->GetCharacterMovement()->MovementMode != MOVE_Falling) {
		return false;
	}

	State.bIsWallRunning = true;
	State.WallSide = Side;
	State.CurrentWallRunDuration = 0.0f;

	UWallRunUtilityLibrary::ApplyWallRunMovementSettings(CharacterOwner, Settings, State);
	SetTickStatus(true);

	ToggleNativeTag((Side == EWallRunSide::Left) ? WallRunGameplayTags::Status_WallRunning_Left : 
		WallRunGameplayTags::Status_WallRunning_Right);

	if (Settings.WallRunDurationLimit > 0.0f) {
		GetWorld()->GetTimerManager().SetTimer(WallRunDurationTimerHandle, this, &UWallRunComponent::WallRunTimeout, 
			Settings.WallRunDurationLimit, false);
	}

	bSpeedTooLow = false;
	GetWorld()->GetTimerManager().SetTimer(SpeedMonitorTimerHandle, this, &UWallRunComponent::MonitorSpeed, 0.25f, 
		true);

	OnWallRunStarted.Broadcast(Side);
	OnWallSideChanged.Broadcast(Side);
	return true;
}
void UWallRunComponent::StopWallRun(const EMovementMode NewMovementMode, const float DisableDuration) {
	if (!State.bIsWallRunning) {
		return;
	}

	State.bIsWallRunning = false;

	ToggleNativeTag(WallRunGameplayTags::Status_WallRunning_Left, true);
	ToggleNativeTag(WallRunGameplayTags::Status_WallRunning_Right, true);

	if (const UWorld* World = GetWorld()) {
		World->GetTimerManager().ClearTimer(WallRunDurationTimerHandle);
		World->GetTimerManager().ClearTimer(SpeedMonitorTimerHandle);
	}

	UWallRunUtilityLibrary::RestoreMovementSettings(CharacterOwner, State, NewMovementMode);
	SetTickStatus(true);

	if (DisableDuration > 0.0f) {
		State.bWallRunningAllowed = false;
		GetWorld()->GetTimerManager().SetTimer(LockoutTimerHandle, this, &UWallRunComponent::ResetWallRunLockout, 
			DisableDuration, false);
	}

	OnWallRunEnded.Broadcast();
}

bool UWallRunComponent::StartWallJump() {
	if (!State.bIsWallRunning || !CharacterOwner) {
		return false;
	}

	FVector LaunchVel;
	FRotator TargetRot;
	UWallRunUtilityLibrary::CalculateWallJumpVelocity(Settings, State, LaunchVel, TargetRot);

	State.bIsWallJumping = true;
	State.TargetWallJumpRotation = TargetRot;
	State.bIsRotatingForWallJump = true;
	State.CurrentWallJumpTimeForRotation = 0.0f;

	StopWallRun(MOVE_Falling, 0.35f);

	UWallRunUtilityLibrary::ApplyWallJumpMovementSettings(CharacterOwner, Settings, State);
	CharacterOwner->LaunchCharacter(LaunchVel, true, true);

	ToggleNativeTag(WallRunGameplayTags::Status_WallJumping);

	if (const UWorld* World = GetWorld()) {
		World->GetTimerManager().SetTimer(AirControlTimerHandle, this, &UWallRunComponent::EnableDelayedAirControl, 
			Settings.DelayedAirControlTime, false);
	}

	OnWallJump.Broadcast(LaunchVel);
	return true;
}
void UWallRunComponent::StopWallJump() {
	if (!State.bIsWallJumping) {
		return;
	}

	State.bIsWallJumping = false;
	State.bIsRotatingForWallJump = false;
	State.CurrentWallJumpTimeForRotation = 0.0f;

	ToggleNativeTag(WallRunGameplayTags::Status_WallJumping, true);

	if (const UWorld* World = GetWorld()) {
		World->GetTimerManager().ClearTimer(AirControlTimerHandle);
	}

	UWallRunUtilityLibrary::RestoreWallJumpMovementSettings(CharacterOwner, State);
}

void UWallRunComponent::OnCharacterMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode) {
	if (!CharacterOwner || !CharacterOwner->GetCharacterMovement()) {
		return;
	}

	switch (CharacterOwner->GetCharacterMovement()->MovementMode) {
	case MOVE_Falling: {
		SetTickStatus(true);
		State.PreviousVelocity = CharacterOwner->GetVelocity();
		break;
	}
	case MOVE_Flying: {
		if (!State.bIsWallRunning) {
			SetTickStatus(false);
			if (State.bIsWallJumping) {
				StopWallJump();
			}
		}
		break;
	}
	default: {
		if (State.bIsWallJumping) {
			StopWallJump();
		}
		if (FMath::Abs(State.CurrentMeshOffset) <= 0.01f) {
			SetTickStatus(false);
		}
		break;
	}
	}
}

void UWallRunComponent::OnCharacterHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
	if (!CharacterOwner || !CharacterOwner->GetCharacterMovement()) {
		return;
	}

	if (CharacterOwner->GetCharacterMovement()->IsWalkable(Hit)) {
		if (State.bIsWallRunning) {
			const float BelowFeet = CharacterOwner->GetActorLocation().Z - CharacterOwner->GetCapsuleComponent()->
				GetScaledCapsuleHalfHeight_WithoutHemisphere();

			if (Hit.ImpactPoint.Z <= BelowFeet) {
				StopWallRun(MOVE_Walking, 0.0f);
			}
		}
		else if (LockoutTimerHandle.IsValid()) {
			GetWorld()->GetTimerManager().ClearTimer(LockoutTimerHandle);
			ResetWallRunLockout();
		}
	}
}

void UWallRunComponent::SetTickStatus(const bool bShouldTick) {
	SetComponentTickEnabled(bShouldTick);
}
void UWallRunComponent::EventTick(float DeltaTime) {
	if (!CharacterOwner || !GetWorld()) {
		return;
	}

	if (State.bIsWallRunning) {
		if (Settings.WallRunDurationLimit > 0.0f && State.CurrentWallRunDuration >= Settings.WallRunDurationLimit) {
			StopWallRun(MOVE_Falling, 0.5f);
			return;
		}

		if (!UWallRunUtilityLibrary::DetectWall(CharacterOwner, State.WallSide, Settings, State)) {
			StopWallRun(MOVE_Falling, 0.35f);
			return;
		}

		FHitResult WallCheckHit;
		const FVector WallTraceStart = CharacterOwner->GetActorLocation();
		const FVector WallTraceEnd = WallTraceStart + (State.AverageWallNormal * -200.0f);
		FCollisionQueryParams WallCheckParams;
		WallCheckParams.AddIgnoredActor(CharacterOwner);

		const bool bWallStillValid = GetWorld()->LineTraceSingleByChannel(WallCheckHit, WallTraceStart, WallTraceEnd, Settings
			.TraceChannel, WallCheckParams);

		if (!bWallStillValid) {
			StopWallRun(MOVE_Falling, 0.35f);
			return;
		}

		FVector WorldDir;
		FRotator TargetRot;
		UWallRunUtilityLibrary::UpdateWallRunMovement(CharacterOwner, Settings, State, DeltaTime, WorldDir, TargetRot);

		State.PreviousVelocity = CharacterOwner->GetVelocity();
		return;
	}

	auto* CMC = CharacterOwner->GetCharacterMovement();
	if (FMath::Abs(State.CurrentMeshOffset) > 0.01f) {
		UWallRunUtilityLibrary::ResetMeshWallOffset(CharacterOwner, State, DeltaTime);
		
		if (FMath::Abs(State.CurrentMeshOffset) <= 0.01f && !State.bIsWallJumping && CMC->MovementMode != MOVE_Falling) {
			SetTickStatus(false);
		}
	}

	WallJumpTick(DeltaTime);

	if (!State.bWallRunningAllowed || CMC->MovementMode != MOVE_Falling) {
		return;
	}

	const float HorizontalSpeed = CharacterOwner->GetVelocity().Size2D();
	const bool bHasMovementInput = !CharacterOwner->GetLastMovementInputVector().IsNearlyZero() ||
		!CMC->GetCurrentAcceleration().IsNearlyZero();
	if (HorizontalSpeed < Settings.MinStartSpeed && !bHasMovementInput) {
		return;
	}

	const float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector TraceStart = CharacterOwner->GetActorLocation() + (CharacterOwner->GetActorForwardVector() * CapsuleRadius * 0.5f);

	FRotator CameraRotation = CharacterOwner->GetActorRotation();
	if (const AController* Controller = CharacterOwner->GetController()) {
		CameraRotation = Controller->GetControlRotation();
	}

	const FVector CameraForward = CameraRotation.Vector();
	const FVector TraceEnd = TraceStart + (CameraForward * CapsuleRadius * 2.0f);

	FHitResult FrontHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);

	if (GetWorld()->SweepSingleByChannel(FrontHit, TraceStart, TraceEnd, FQuat::Identity, Settings.TraceChannel, 
		FCollisionShape::MakeSphere(CapsuleRadius * Settings.PrimaryTraceRadiusMultiplier), QueryParams)) {
		if (UWallRunUtilityLibrary::IsWallRunnable(FrontHit.GetActor(), Settings)) {
			const EWallRunSide Side = UWallRunUtilityLibrary::DetermineWallSideFromCamera(CharacterOwner, FrontHit.ImpactNormal);
			if (UWallRunUtilityLibrary::DetectWall(CharacterOwner, Side, Settings, State)) {
				float FrontCornerAngle = 0.0f;
				if (UWallRunUtilityLibrary::CheckCornerAngle(FrontHit.ImpactNormal, State.PrimaryTraceNormal, 
					Side, Settings, FrontCornerAngle)) {
					const EWallRunSide FinalSide = State.WallSide;
					if (UWallRunUtilityLibrary::CheckBottomArea(CharacterOwner, Settings, State) &&
						UWallRunUtilityLibrary::CheckTopArea(CharacterOwner, Settings, State)) {
						StartWallRun(FinalSide);
						return;
					}
				}
			}
		}
	}

	if (UWallRunUtilityLibrary::DetectWall(CharacterOwner, EWallRunSide::Right, Settings, State)) {
		const EWallRunSide FinalSide = State.WallSide;
		if (UWallRunUtilityLibrary::CheckBottomArea(CharacterOwner, Settings, State) &&
			UWallRunUtilityLibrary::CheckTopArea(CharacterOwner, Settings, State)) {
			StartWallRun(FinalSide);
			return;
		}
	}

	if (UWallRunUtilityLibrary::DetectWall(CharacterOwner, EWallRunSide::Left, Settings, State)) {
		const EWallRunSide FinalSide = State.WallSide;
		if (UWallRunUtilityLibrary::CheckBottomArea(CharacterOwner, Settings, State) &&
			UWallRunUtilityLibrary::CheckTopArea(CharacterOwner, Settings, State)) {
			StartWallRun(FinalSide);
			return;
		}
	}
}
void UWallRunComponent::WallJumpTick(const float DeltaTime) {
	if (!State.bIsWallJumping || !State.bIsRotatingForWallJump || !CharacterOwner) {
		return;
	}

	constexpr float StopRotationAfter = 0.04f;
	if (State.CurrentWallJumpTimeForRotation <= StopRotationAfter) {
		const FRotator InterpRot = FMath::RInterpTo(CharacterOwner->GetActorRotation(), State.TargetWallJumpRotation, 
			DeltaTime, 10.0f);
		CharacterOwner->SetActorRotation(InterpRot, ETeleportType::TeleportPhysics);

		State.CurrentWallJumpTimeForRotation += DeltaTime;
		if (State.CurrentWallJumpTimeForRotation >= StopRotationAfter) {
			CharacterOwner->SetActorRotation(State.TargetWallJumpRotation, ETeleportType::TeleportPhysics);
			State.bIsRotatingForWallJump = false;
			State.CurrentWallJumpTimeForRotation = 0.0f;
		}
	}
}

void UWallRunComponent::WallRunTimeout() {
	StopWallRun(MOVE_Falling, 0.5f);
}

void UWallRunComponent::EnableDelayedAirControl() const {
	if (CharacterOwner) {
		CharacterOwner->GetCharacterMovement()->AirControl = Settings.WallJumpAirControl;
	}
}

void UWallRunComponent::MonitorSpeed() {
	if (!CharacterOwner || !State.bIsWallRunning) {
		return;
	}

	const float Speed = CharacterOwner->GetVelocity().Size2D();
	if (Speed < 300.0f) {
		if (bSpeedTooLow) {
			bSpeedTooLow = false;
			StopWallRun(MOVE_Falling, 0.5f);
		}
		else {
			bSpeedTooLow = true;
		}
	}
	else {
		bSpeedTooLow = false;
	}
}

void UWallRunComponent::ResetWallRunLockout() {
	State.bWallRunningAllowed = true;
}

void UWallRunComponent::ToggleNativeTag(const FGameplayTag& Tag, const bool bRemove) const {
	if (!Tag.IsValid() || !CharacterOwner) {
		return;
	}

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CharacterOwner)) {
		ASC->SetLooseGameplayTagCount(Tag, bRemove ? 0 : 1);
	}
}

