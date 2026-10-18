// No Copyright!

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WallRunTypes.h"
#include "WallRunUtilityLibrary.generated.h"

class ACharacter;

UCLASS()
class WALLRUN_API UWallRunUtilityLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	static bool IsWallRunnable(const AActor* WallActor, const FWallRunSettings& Settings);
	static EWallRunSide DetermineWallSideFromCamera(const ACharacter* Character, const FVector& WallNormal);
	static bool DetectWall(ACharacter* Character, EWallRunSide Side, const FWallRunSettings& Settings, FWallRunState& InOutState);
	static bool CheckCornerAngle(const FVector& FirstWallNormal, const FVector& SecondWallNormal, EWallRunSide Side, const 
		FWallRunSettings& Settings, float& OutAngle);
	
	static bool CheckTopArea(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState);
	static bool CheckBottomArea(const ACharacter* Character, const FWallRunSettings& Settings, const FWallRunState& State);
	
	static bool TraceRelativeToWall(const ACharacter* Character, FHitResult& OutHitResult, const FVector& CurrentWallNormal, const FVector&
		WallLocation, float InitTraceLength, float StartOffsetBackwards, float StartOffsetAwayFromWall, float EndOffsetBackwards, 
		EWallRunSide Side, ECollisionChannel TraceChannel);

	static void HugWall(ACharacter* Character, const FWallRunState& State);
	static void UpdateMeshWallOffset(const ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, float 
		DeltaTime);
	static void ResetMeshWallOffset(const ACharacter* Character, FWallRunState& InOutState, float DeltaTime = 0.0f);

	static void UpdateWallRunMovement(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, float DeltaTime, 
		FVector& OutWorldDirection, FRotator& OutTargetRotation);
	static void ApplyWallRunMovementSettings(const ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState);
	static void RestoreMovementSettings(const ACharacter* Character, const FWallRunState& State, EMovementMode 
		NewMovementMode = MOVE_Falling);

	static void CalculateWallJumpVelocity(const FWallRunSettings& Settings, const FWallRunState& State, FVector& OutLaunchVelocity, 
		FRotator& OutTargetRotation);
	static void ApplyWallJumpMovementSettings(const ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState);
	static void RestoreWallJumpMovementSettings(const ACharacter* Character, const FWallRunState& State);

	static void UpdateAnimationValues(const ACharacter* Character, FWallRunState& InOutState, float DeltaTime);
};
