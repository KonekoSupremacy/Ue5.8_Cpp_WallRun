// No Copyright!

// Game Includes
#include "UGA_WallRun.h"
#include "WallRunGameplayTags.h"
#include "WallRunUtilityLibrary.h"

// Engine Includes
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UGA_WallRun::UGA_WallRun() {
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	WallRunningTag = WallRunGameplayTags::Status_WallRunning;
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

	if (const UCharacterMovementComponent* CMC = CharacterAvatar->GetCharacterMovement()) {
		if (CMC->MovementMode == MOVE_Falling) {
			SetTickStatus(true);
			State.PreviousVelocity = CharacterAvatar->GetVelocity();
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

	if (const UWorld* World = GetWorld()) {
		auto& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(WallRunDurationTimerHandle);
		TimerManager.ClearTimer(SpeedMonitorTimerHandle);
		TimerManager.ClearTimer(AirControlTimerHandle);
		TimerManager.ClearTimer(LockoutTimerHandle);
	}

	if (State.bIsWallRunning) {
		StopWallRun(MOVE_Falling, 0.0f);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGA_WallRun::StartWallRun(const EWallRunSide Side) {
	if (!State.bWallRunningAllowed || !CharacterAvatar || State.bIsWallRunning) {
		return false;
	}

	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo()) {
		if (ASC->HasAnyMatchingGameplayTags(TagsBlockingWallRun)) {
			return false;
		}
	}

	if (!CharacterAvatar->GetCharacterMovement() || CharacterAvatar->GetCharacterMovement()->MovementMode != MOVE_Falling) {
		return false;
	}

	State.bIsWallRunning = true;
	State.WallSide = Side;
	State.CurrentWallRunDuration = 0.0f;

	UWallRunUtilityLibrary::ApplyWallRunMovementSettings(CharacterAvatar, Settings, State);
	SetTickStatus(true);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo()) {
		ASC->AddLooseGameplayTag(WallRunningTag);
		ASC->AddLooseGameplayTag((Side == EWallRunSide::Left) ? WallRunGameplayTags::Status_WallRunning_Left : 
			WallRunGameplayTags::Status_WallRunning_Right);
	}

	if (Settings.WallRunDurationLimit > 0.0f) {
		GetWorld()->GetTimerManager().SetTimer(WallRunDurationTimerHandle, this, &UGA_WallRun::WallRunTimeout, 
			Settings.WallRunDurationLimit, false);
	}

	bSpeedTooLow = false;
	GetWorld()->GetTimerManager().SetTimer(SpeedMonitorTimerHandle, this, &UGA_WallRun::MonitorSpeed, 0.25f, true);
	return true;
}
void UGA_WallRun::StopWallRun(const EMovementMode NewMovementMode, const float DisableDuration) {
	if (!State.bIsWallRunning) {
		return;
	}

	State.bIsWallRunning = false;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo()) {
		ASC->RemoveLooseGameplayTag(WallRunningTag);
		ASC->RemoveLooseGameplayTag(WallRunGameplayTags::Status_WallRunning_Left);
		ASC->RemoveLooseGameplayTag(WallRunGameplayTags::Status_WallRunning_Right);
	}

	if (const UWorld* World = GetWorld()) {
		World->GetTimerManager().ClearTimer(WallRunDurationTimerHandle);
		World->GetTimerManager().ClearTimer(SpeedMonitorTimerHandle);
	}

	UWallRunUtilityLibrary::RestoreMovementSettings(CharacterAvatar, State, NewMovementMode);
	SetTickStatus(true);

	if (DisableDuration > 0.0f) {
		State.bWallRunningAllowed = false;
		GetWorld()->GetTimerManager().SetTimer(LockoutTimerHandle, this, &UGA_WallRun::ResetWallRunLockout, 
			DisableDuration, false);
	}
}

bool UGA_WallRun::StartWallJump() {
	if (!State.bIsWallRunning || !CharacterAvatar) {
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

	UWallRunUtilityLibrary::ApplyWallJumpMovementSettings(CharacterAvatar, Settings, State);
	CharacterAvatar->LaunchCharacter(LaunchVel, true, true);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo()) {
		ASC->AddLooseGameplayTag(WallJumpingTag);
	}

	if (const UWorld* World = GetWorld()) {
		World->GetTimerManager().SetTimer(AirControlTimerHandle, this, &UGA_WallRun::EnableDelayedAirControl, 
			Settings.DelayedAirControlTime, false);
	}

	return true;
}
void UGA_WallRun::StopWallJump() {
	if (!State.bIsWallJumping) {
		return;
	}

	State.bIsWallJumping = false;
	State.bIsRotatingForWallJump = false;
	State.CurrentWallJumpTimeForRotation = 0.0f;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo()) {
		ASC->RemoveLooseGameplayTag(WallJumpingTag);
	}

	if (const UWorld* World = GetWorld()) {
		World->GetTimerManager().ClearTimer(AirControlTimerHandle);
	}

	UWallRunUtilityLibrary::RestoreWallJumpMovementSettings(CharacterAvatar, State);
}

void UGA_WallRun::OnCharacterMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode) {
	if (!CharacterAvatar || !CharacterAvatar->GetCharacterMovement()) {
		return;
	}

	switch (CharacterAvatar->GetCharacterMovement()->MovementMode) {
	case MOVE_Falling: {
		SetTickStatus(true);
		State.PreviousVelocity = CharacterAvatar->GetVelocity();
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

void UGA_WallRun::OnCharacterHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
	if (!CharacterAvatar || !CharacterAvatar->GetCharacterMovement()) {
		return;
	}

	if (CharacterAvatar->GetCharacterMovement()->IsWalkable(Hit)) {
		if (State.bIsWallRunning) {
			const float BelowFeet = CharacterAvatar->GetActorLocation().Z - CharacterAvatar->GetCapsuleComponent()->
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

void UGA_WallRun::SetTickStatus(const bool bShouldTick) {
	if (!GetWorld()) {
		return;
	}

	if (bShouldTick) {
		if (!TickTimerHandle.IsValid()) {
			FTimerManagerTimerParameters Params;
			Params.bMaxOncePerFrame = true;
			Params.bLoop = true;
			GetWorld()->GetTimerManager().SetTimer(TickTimerHandle, this, &UGA_WallRun::EventTick, 0.001f, Params);
		}
	}
	else {
		GetWorld()->GetTimerManager().ClearTimer(TickTimerHandle);
	}
}
void UGA_WallRun::EventTick() {
	if (!CharacterAvatar || !GetWorld()) {
		return;
	}

	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	if (State.bIsWallRunning) {
		if (Settings.WallRunDurationLimit > 0.0f && State.CurrentWallRunDuration >= Settings.WallRunDurationLimit) {
			StopWallRun(MOVE_Falling, 0.5f);
			return;
		}

		if (!UWallRunUtilityLibrary::DetectWall(CharacterAvatar, State.WallSide, Settings, State)) {
			StopWallRun(MOVE_Falling, 0.35f);
			return;
		}

		FHitResult WallCheckHit;
		const FVector WallTraceStart = CharacterAvatar->GetActorLocation();
		const FVector WallTraceEnd = WallTraceStart + (State.AverageWallNormal * -200.0f);
		FCollisionQueryParams WallCheckParams;
		WallCheckParams.AddIgnoredActor(CharacterAvatar);

		const bool bWallStillValid = GetWorld()->LineTraceSingleByChannel(WallCheckHit, WallTraceStart, WallTraceEnd, Settings
			.TraceChannel, WallCheckParams);

		if (!bWallStillValid) {
			StopWallRun(MOVE_Falling, 0.35f);
			return;
		}

		FVector WorldDir;
		FRotator TargetRot;
		UWallRunUtilityLibrary::UpdateWallRunMovement(CharacterAvatar, Settings, State, DeltaTime, WorldDir, TargetRot);

		State.PreviousVelocity = CharacterAvatar->GetVelocity();
		return;
	}

	auto* CMC = CharacterAvatar->GetCharacterMovement();
	if (FMath::Abs(State.CurrentMeshOffset) > 0.01f) {
		UWallRunUtilityLibrary::ResetMeshWallOffset(CharacterAvatar, State, DeltaTime);

		if (FMath::Abs(State.CurrentMeshOffset) <= 0.01f && !State.bIsWallJumping && CMC->MovementMode != MOVE_Falling) {
			SetTickStatus(false);
		}
	}

	WallJumpTick(DeltaTime);

	if (!State.bWallRunningAllowed || CMC->MovementMode != MOVE_Falling) {
		return;
	}

	const float HorizontalSpeed = CharacterAvatar->GetVelocity().Size2D();
	const bool bHasMovementInput = !CharacterAvatar->GetLastMovementInputVector().IsNearlyZero() ||
		!CMC->GetCurrentAcceleration().IsNearlyZero();
	if (HorizontalSpeed < Settings.MinStartSpeed && !bHasMovementInput) {
		return;
	}

	const float CapsuleRadius = CharacterAvatar->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector TraceStart = CharacterAvatar->GetActorLocation() + (CharacterAvatar->GetActorForwardVector() * CapsuleRadius * 0.5f);

	FRotator CameraRotation = CharacterAvatar->GetActorRotation();
	if (const AController* Controller = CharacterAvatar->GetController()) {
		CameraRotation = Controller->GetControlRotation();
	}

	const FVector CameraForward = CameraRotation.Vector();
	const FVector TraceEnd = TraceStart + (CameraForward * CapsuleRadius * 2.0f);

	FHitResult FrontHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterAvatar);

	if (GetWorld()->SweepSingleByChannel(FrontHit, TraceStart, TraceEnd, FQuat::Identity, Settings.TraceChannel, 
		FCollisionShape::MakeSphere(CapsuleRadius * Settings.PrimaryTraceRadiusMultiplier), QueryParams)) {
		if (UWallRunUtilityLibrary::IsWallRunnable(FrontHit.GetActor(), Settings)) {
			const EWallRunSide Side = UWallRunUtilityLibrary::DetermineWallSideFromCamera(CharacterAvatar, FrontHit.ImpactNormal);
			if (UWallRunUtilityLibrary::DetectWall(CharacterAvatar, Side, Settings, State)) {
				float FrontCornerAngle = 0.0f;
				if (UWallRunUtilityLibrary::CheckCornerAngle(FrontHit.ImpactNormal, State.PrimaryTraceNormal, 
					Side, Settings, FrontCornerAngle)) {
					const EWallRunSide FinalSide = State.WallSide;
					if (UWallRunUtilityLibrary::CheckBottomArea(CharacterAvatar, Settings, State) &&
						UWallRunUtilityLibrary::CheckTopArea(CharacterAvatar, Settings, State)) {
						StartWallRun(FinalSide);
						return;
					}
				}
			}
		}
	}

	if (UWallRunUtilityLibrary::DetectWall(CharacterAvatar, EWallRunSide::Right, Settings, State)) {
		const EWallRunSide FinalSide = State.WallSide;
		if (UWallRunUtilityLibrary::CheckBottomArea(CharacterAvatar, Settings, State) &&
			UWallRunUtilityLibrary::CheckTopArea(CharacterAvatar, Settings, State)) {
			StartWallRun(FinalSide);
			return;
		}
	}

	if (UWallRunUtilityLibrary::DetectWall(CharacterAvatar, EWallRunSide::Left, Settings, State)) {
		const EWallRunSide FinalSide = State.WallSide;
		if (UWallRunUtilityLibrary::CheckBottomArea(CharacterAvatar, Settings, State) &&
			UWallRunUtilityLibrary::CheckTopArea(CharacterAvatar, Settings, State)) {
			StartWallRun(FinalSide);
			return;
		}
	}
}
void UGA_WallRun::WallJumpTick(const float DeltaTime) {
	if (!State.bIsWallJumping || !State.bIsRotatingForWallJump || !CharacterAvatar) {
		return;
	}

	constexpr float StopRotationAfter = 0.04f;
	if (State.CurrentWallJumpTimeForRotation <= StopRotationAfter) {
		const FRotator InterpRot = FMath::RInterpTo(CharacterAvatar->GetActorRotation(), State.TargetWallJumpRotation, 
			DeltaTime, 10.0f);
		CharacterAvatar->SetActorRotation(InterpRot, ETeleportType::TeleportPhysics);

		State.CurrentWallJumpTimeForRotation += DeltaTime;
		if (State.CurrentWallJumpTimeForRotation >= StopRotationAfter) {
			CharacterAvatar->SetActorRotation(State.TargetWallJumpRotation, ETeleportType::TeleportPhysics);
			State.bIsRotatingForWallJump = false;
			State.CurrentWallJumpTimeForRotation = 0.0f;
		}
	}
}
void UGA_WallRun::WallRunTimeout() {
	StopWallRun(MOVE_Falling, 0.5f);
}
void UGA_WallRun::EnableDelayedAirControl() const {
	if (CharacterAvatar && CharacterAvatar->GetCharacterMovement()) {
		CharacterAvatar->GetCharacterMovement()->AirControl = Settings.WallJumpAirControl;
	}
}
void UGA_WallRun::MonitorSpeed() {
	if (!CharacterAvatar || !State.bIsWallRunning) {
		return;
	}

	const float Speed = CharacterAvatar->GetVelocity().Size2D();
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
void UGA_WallRun::ResetWallRunLockout() {
	State.bWallRunningAllowed = true;
}
