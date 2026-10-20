// Copyright: Elixir Studios

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TimerManager.h"
#include "Templates/Function.h"
#include "WallRunTypes.h"
#include "WallRunUtilityLibrary.generated.h"

// Forward Declarations
class ACharacter;
class UAbilitySystemComponent;
class UWorld;

UCLASS()
class WALLRUN_API UWallRunUtilityLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	static bool IsWallRunnable(const AActor* WallActor, const FWallRunSettings& Settings);
	static EWallRunSide DetermineWallSideFromCamera(const ACharacter* Character, const FVector& WallNormal);
	static bool DetectWall(ACharacter* Character, EWallRunSide Side, const FWallRunSettings& Settings, FWallRunState& InOutState);
	static bool CheckCornerAngle(const FVector& FirstWallNormal, const FVector& SecondWallNormal, EWallRunSide Side, const 
		FWallRunSettings& Settings, float& OutAngle);
	
	static bool CheckTopArea(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, float DeltaTime);
	static bool CheckBottomArea(const ACharacter* Character, const FWallRunSettings& Settings, const FWallRunState& State);
	
	static bool TraceRelativeToWall(const ACharacter* Character, FHitResult& OutHitResult, const FVector& CurrentWallNormal, const FVector&
		WallLocation, float InitTraceLength, float StartOffsetBackwards, float StartOffsetAwayFromWall, float EndOffsetBackwards, 
		EWallRunSide Side, ECollisionChannel TraceChannel);

	static void HugWall(ACharacter* Character, const FWallRunSettings& Settings, const FWallRunState& State, float DeltaTime);
	static void UpdateMeshWallOffset(const ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, float 
		DeltaTime);
	static void ResetMeshWallOffset(const ACharacter* Character, FWallRunState& InOutState, float DeltaTime = 0.0f);

	static void UpdateWallRunMovement(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, float DeltaTime);
	static void ApplyWallRunMovementSettings(const ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState);
	static void RestoreMovementSettings(const ACharacter* Character, const FWallRunState& State, EMovementMode 
		NewMovementMode = MOVE_Falling);

	static void CalculateWallJumpVelocity(const FWallRunSettings& Settings, const FWallRunState& State, FVector& OutLaunchVelocity, 
		FRotator& OutTargetRotation);
	static void ApplyWallJumpMovementSettings(const ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState);
	static void RestoreWallJumpMovementSettings(const ACharacter* Character, const FWallRunState& State);
	static void UpdateWallJumpRotation(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, 
		float DeltaTime);

	static void UpdateAnimationValues(const ACharacter* Character, FWallRunState& InOutState, float DeltaTime);

	//================================================================================================================
	// SHARED WALL RUN FLOW
	//================================================================================================================
	static bool CanStartWallRun(const ACharacter* Character, const FWallRunState& State, const UAbilitySystemComponent* ASC = nullptr, 
		const FGameplayTagContainer* BlockingTags = nullptr);
	static void BeginWallRun(const ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, 
		EWallRunSide Side);
	static void EndWallRun(const ACharacter* Character, FWallRunState& InOutState, EMovementMode NewMovementMode, 
		float DisableDuration);

	static bool BeginWallJump(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, 
		const FGameplayTag& JumpingTag, FVector& OutLaunchVelocity, const TFunction<void()>& OnWallRunStopped);
	static bool EndWallJump(const ACharacter* Character, FWallRunState& InOutState, const FGameplayTag& JumpingTag);

	static EWallRunStopReason TickActiveWallRun(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, 
		float DeltaTime);
	static float WallRunStopLockoutDuration(EWallRunStopReason Reason);
	static void TickWallJumpIdle(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, float DeltaTime, const TFunction<void(bool)>& OnSetTickStatus);
	static bool TryFindWallRunStart(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, float DeltaTime);

	static bool UpdateSpeedMonitor(const ACharacter* Character, FWallRunState& InOutState);
	static void SetWallJumpAirControl(const ACharacter* Character, const FWallRunSettings& Settings);
	static void SetWallRunTagCount(const ACharacter* Character, const FGameplayTag& Tag, bool bRemove);

	static void HandleMovementModeChanged(const ACharacter* Character, FWallRunState& InOutState, const TFunction<void(bool)>& 
		OnSetTickStatus, const TFunction<void()>& OnStopWallJump);
	static void HandleCharacterHit(const ACharacter* Character, const FWallRunState& InOutState, const FHitResult& Hit, 
		FTimerHandle& LockoutTimerHandle, const TFunction<void()>& OnStopWallRun, const TFunction<void()>& OnLockoutCleared);

	static void StartWallRunTimers(const UWorld* World, const FWallRunSettings& Settings, FTimerHandle& DurationHandle, 
		FTimerHandle& SpeedHandle, const FTimerDelegate& OnTimeout, const FTimerDelegate& OnSpeedCheck);
	static void ClearWallRunTimers(const UWorld* World, FTimerHandle& DurationHandle, FTimerHandle& SpeedHandle);
};
