// Copyright: KonekoSupremacy

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "WallRunTypes.generated.h"

UENUM(BlueprintType)
enum class EWallRunSide : uint8 {
	Right UMETA(DisplayName = "Right"),
	Left  UMETA(DisplayName = "Left")
};

enum class EWallRunStopReason : uint8 {
	None,
	Timeout,
	LostWall
};

USTRUCT(BlueprintType)
struct WALLRUN_API FWallRunSettings {
	GENERATED_BODY()

	/** If true, player can only wall run on actors with the required tag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|General")
	bool bRequiresTag = false;
	/** Actor tag required to wall run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|General", meta = (EditCondition = "bRequiresTag"))
	FName RequiredTag = FName("WallRun");
	/** Tag on an actor that explicitly prevents wall running */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|General", meta = (EditCondition = "!bRequiresTag"))
	FName BlockingTag = FName("NoWallRun");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Collision")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Multiplier applied to capsule radius for traces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Collision", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float PrimaryTraceRadiusMultiplier = 0.85f;
	/** Minimum safe area between the capsule boundary and  wall surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Collision", meta = (ClampMin = "5.0", ClampMax = "50.0"))
	float CapsuleClearance = 5.0f;

	/** Max angle between two wall normals that can be navigated, inside or outside corner, in deg */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Corners", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float CornerAngleLimit = 85.0f;
	
	// Movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Movement")
	float WallRunSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Movement")
	float WallRunAcceleration = 50000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Movement")
	float WallRunBrakingDeceleration = 500.0f;

	/** Max degrees per second the character turns to align with the wall while wall running. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Movement", meta = (ClampMin = "30.0", ClampMax = "1080.0"))
	float MaxWallRunTurnRate = 360.0f;

	/** Max speed (cm/s) the capsule may be repositioned at to hug the walle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Movement", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
	float CapsuleAdjustmentSpeed = 400.0f;

	/** Vertical arc shape (0 = flat horizontal, 1 = maximum arc). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Movement", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ArcAmount = 0.65f;

	/** Max duration before wall run ends (0 = unlimited). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Movement", meta = (ClampMin = "0.0"))
	float WallRunDurationLimit = 3.0f;

	/** Min horizontal speed required to start a wall run when no movement input is there */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Movement", meta = (ClampMin = "0.0"))
	float MinStartSpeed = 200.0f;

	/** Angle at which character launches away from wall (90 = perpendicular, 45 = diagonal forward-outward). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Wall Jump", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float WallJumpAngle = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Wall Jump")
	float WallJumpHorizontalVelocity = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Wall Jump")
	float WallJumpVerticalVelocity = 500.0f;

	/** Time taken to rotate into the wall jump direction (0 = instant). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Wall Jump", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WallJumpRotationTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Wall Jump")
	float WallJumpGravity = 1.5f;
	/** Air control granted after wall jump. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Wall Jump", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WallJumpAirControl = 0.3f;
	/** Delay before wall jump grants air control (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Wall Jump")
	float DelayedAirControlTime = 0.35f;

	/** Whether to override braking deceleration falling during wall jump to maintain forward launch momentum. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Wall Jump")
	bool bOverrideBrakingDecelerationFalling = true;
	/** Braking deceleration falling applied during wall jump if override is enabled (0 = no mid-air braking). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Wall Jump", meta = (EditCondition = "bOverrideBrakingDecelerationFalling"))
	float WallJumpBrakingDecelerationFalling = 0.0f;

	/** Visual lateral offset applied to the skeletal mesh towards the wall to close visual gaps without clipping the capsule. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Visuals", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float MeshWallOffset = 5.0f;
	/** Speed of smoothly interpolating the visual mesh offset towards or away from the wall. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run|Visuals", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float MeshInterpSpeed = 10.0f;

};

USTRUCT(BlueprintType)
struct WALLRUN_API FWallRunState {
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	bool bIsWallRunning = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	bool bIsWallJumping = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	bool bWallRunningAllowed = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	bool bSpeedTooLow = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	EWallRunSide WallSide = EWallRunSide::Right;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	FVector AverageWallNormal = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	FVector PrimaryTraceNormal = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	FVector SecondaryTraceNormal = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	FVector CurrentWallLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	float DistanceFromWall = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	float CurrentWallRunDuration = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	float CurrentArcAngle = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	float CurrentTurnRate = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	float LastKnownSafeHeight = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	bool bWallTopReached = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	FVector PreviousVelocity = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	FRotator PreviousRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	FRotator TargetWallJumpRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	bool bIsRotatingForWallJump = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	float CurrentWallJumpTimeForRotation = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	FVector DefaultMeshRelativeLocation = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	bool bDefaultMeshLocationInitialized = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run|State")
	float CurrentMeshOffset = 0.0f;

	float SavedMaxFlySpeed = 0.0f;
	float SavedMaxAcceleration = 0.0f;
	float SavedBrakingDeceleration = 0.0f;
	float SavedAirControl = 0.0f;
	float SavedGravityScale = 1.0f;
	float SavedBrakingDecelerationFalling = 0.0f;
	bool bSavedOrientRotation = false;
	bool bSavedOverrideBrakingDecelerationFalling = false;
};
