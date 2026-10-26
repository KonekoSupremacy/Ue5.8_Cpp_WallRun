// Copyright KonekoSupremacy 2026 All Rights Reserved.

// Game Includes
#include "WallRunAnimInstance.h"
#include "WallRunComponent.h"
#include "UGA_WallRun.h"

// Engine Includes
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"

void UWallRunAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();

	if (const APawn* Pawn = TryGetPawnOwner()) {
		WallRunComponent = Pawn->FindComponentByClass<UWallRunComponent>();
		if (!WallRunComponent.IsValid()) {
			AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
		}
	}
}

void UWallRunAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds) {
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) {
		return;
	}

	FWallRunState CurrentState;
	bool bFoundState = false;

	if (auto* WallRunComp = WallRunComponent.Get()) {
		CurrentState = WallRunComp->GetWallRunState();
		bFoundState = true;
	}
	else if (auto* ASC = AbilitySystemComponent.Get()) {
		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()) {
			if (UGA_WallRun* GA = Cast<UGA_WallRun>(Spec.GetPrimaryInstance())) {
				CurrentState = GA->GetWallRunState();
				bFoundState = true;
				break;
			}
		}
	}

	if (bFoundState) {
		bIsWallRunning = CurrentState.bIsWallRunning;
		bIsWallJumping = CurrentState.bIsWallJumping;
		WallSide = CurrentState.WallSide;
		CurrentArcAngle = CurrentState.CurrentArcAngle;
		CurrentTurnRate = CurrentState.CurrentTurnRate;

		CurrentSpeed = Pawn->GetVelocity().Size();
		WallRunAnimPlayRate = FMath::GetMappedRangeValueClamped(PlayRateSpeedRange, PlayRateRange, CurrentSpeed);
	}
	else {
		bIsWallRunning = false;
		bIsWallJumping = false;
	}
}
