// Copyright: KonekoSupremacy

// Game Includes
#include "WallRunUtilityLibrary.h"
#include "WallRunGameplayTags.h"

// Engine Includes
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"

static void AdjustActorHeight(ACharacter* Character, const float TargetHeight, const FWallRunSettings& Settings, const float DeltaTime) {
	const FVector Location = Character->GetActorLocation();
	Character->SetActorLocation(FVector(Location.X, Location.Y, 
		FMath::FInterpConstantTo(Location.Z, TargetHeight, DeltaTime, Settings.CapsuleAdjustmentSpeed)), true);
}
static void BlockWallRunAscent(ACharacter* Character, FWallRunState& InOutState, const FWallRunSettings& Settings, 
	const float TargetHeight, const float DeltaTime) {
	InOutState.bWallTopReached = true;
	AdjustActorHeight(Character, TargetHeight, Settings, DeltaTime);

	if (UCharacterMovementComponent* CMC = Character->GetCharacterMovement()) {
		CMC->Velocity.Z = FMath::Min(CMC->Velocity.Z, 0.0f);
	}
}

bool UWallRunUtilityLibrary::IsWallRunnable(const AActor* WallActor, const FWallRunSettings& Settings) {
	if (!WallActor) {
		return false;
	}
	if (Settings.bRequiresTag) {
		return WallActor->ActorHasTag(Settings.RequiredTag);
	}
	return !WallActor->ActorHasTag(Settings.BlockingTag);
}
EWallRunSide UWallRunUtilityLibrary::DetermineWallSideFromCamera(const ACharacter* Character, const FVector& WallNormal) {
	if (!Character) {
		return EWallRunSide::Right;
	}

	FRotator CameraRotation = Character->GetActorRotation();
	if (const AController* Controller = Character->GetController()) {
		CameraRotation = Controller->GetControlRotation();
	}

	const FVector CamRight = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Y);
	const float CameraWallRightDot = FVector::DotProduct(CamRight, WallNormal);

	return (CameraWallRightDot > 0.0f) ? EWallRunSide::Left : EWallRunSide::Right;
}
bool UWallRunUtilityLibrary::DetectWall(ACharacter* Character, EWallRunSide Side, const FWallRunSettings& Settings, 
	FWallRunState& InOutState) {
	if (!Character) {
		return false;
	}
	const float CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector ActorLoc = Character->GetActorLocation();

	FRotator CameraRotation = Character->GetActorRotation();
	if (const AController* Controller = Character->GetController()) {
		CameraRotation = Controller->GetControlRotation();
	}

	const FVector CamForward = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::X);
	const FVector CamRight = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Y);

	FVector Start;
	FVector End;
	if (InOutState.bIsWallRunning) {
		Start = ActorLoc;
		End = ActorLoc - (InOutState.AverageWallNormal * CapsuleRadius * 2.0f);
	}
	else {
		Start = ActorLoc + (CamForward * CapsuleRadius * 0.5f);
		const float PrimarySideMultiplier = (Side == EWallRunSide::Right) ? 1.0f : -1.0f;
		const FVector PrimaryDir = CamRight * PrimarySideMultiplier;
		End = Start + (PrimaryDir * CapsuleRadius * 2.0f);
	}

	FHitResult PrimaryHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	const float SphereRadius = CapsuleRadius * Settings.PrimaryTraceRadiusMultiplier;
	const bool bPrimaryHit = Character->GetWorld()->SweepSingleByChannel(PrimaryHit, Start, End, FQuat::Identity, Settings
		.TraceChannel, FCollisionShape::MakeSphere(SphereRadius), Params);
	
	if (bPrimaryHit && IsWallRunnable(PrimaryHit.GetActor(), Settings)) {
		InOutState.PrimaryTraceNormal = PrimaryHit.ImpactNormal;
		InOutState.AverageWallNormal = InOutState.PrimaryTraceNormal;

		const FVector WallToChar = ActorLoc - PrimaryHit.ImpactPoint;
		const float ProjectedDistance = FVector::DotProduct(WallToChar, PrimaryHit.ImpactNormal);
		InOutState.DistanceFromWall = ProjectedDistance - (CapsuleRadius + Settings.CapsuleClearance);
		InOutState.CurrentWallLocation = PrimaryHit.ImpactPoint;

		const float CameraWallRightDot = FVector::DotProduct(CamRight, InOutState.AverageWallNormal);
		const EWallRunSide CameraBasedDirection = (CameraWallRightDot > 0.0f) ? EWallRunSide::Left : EWallRunSide::Right;

		if (!InOutState.bIsWallRunning) {
			InOutState.WallSide = CameraBasedDirection;
		}

		const EWallRunSide EffectiveSide = (InOutState.bIsWallRunning) ? Side : CameraBasedDirection;

		FHitResult SecondaryHitResult;
		const bool bSecondaryHit = TraceRelativeToWall(Character, SecondaryHitResult, InOutState.PrimaryTraceNormal, 
			InOutState.CurrentWallLocation, 2.5f, -1.5f, 0.75f, -2.0f,
			EffectiveSide, Settings.TraceChannel);

		if (bSecondaryHit && IsWallRunnable(SecondaryHitResult.GetActor(), Settings)) {
			float CornerAngle = 0.0f;
			if (CheckCornerAngle(InOutState.PrimaryTraceNormal, SecondaryHitResult
				.ImpactNormal, Settings, CornerAngle)) {
				InOutState.SecondaryTraceNormal = SecondaryHitResult.ImpactNormal;
				InOutState.AverageWallNormal = (InOutState.AverageWallNormal + InOutState.SecondaryTraceNormal).GetSafeNormal();
				return true;
			}

			FHitResult BackupHitResult;
			if (TraceRelativeToWall(Character, BackupHitResult, InOutState.PrimaryTraceNormal, InOutState
				.CurrentWallLocation, 3.0f, -2.0f, 0.75f, 
				-2.5f, EffectiveSide, Settings.TraceChannel)) {
				if (IsWallRunnable(BackupHitResult.GetActor(), Settings)) {
					if (CheckCornerAngle(InOutState.PrimaryTraceNormal, BackupHitResult.ImpactNormal,
						Settings, CornerAngle)) {
						InOutState.SecondaryTraceNormal = BackupHitResult.ImpactNormal;
						InOutState.AverageWallNormal = (InOutState.AverageWallNormal + InOutState.SecondaryTraceNormal).GetSafeNormal();
						return true;
					}
				}
			}

			return false;
		}
		else { // outside corner 
			FHitResult CornerHitResult;
			if (TraceRelativeToWall(Character, CornerHitResult, InOutState.PrimaryTraceNormal, 
				InOutState.CurrentWallLocation, 1.8f, -1.5f, 0.0f, 
				2.0f, EffectiveSide, Settings.TraceChannel)) {
				if (IsWallRunnable(CornerHitResult.GetActor(), Settings)) {
					float CornerAngle = 0.0f;
					if (CheckCornerAngle(InOutState.PrimaryTraceNormal, CornerHitResult
						.ImpactNormal, Settings, CornerAngle)) {
						InOutState.SecondaryTraceNormal = CornerHitResult.ImpactNormal;
						InOutState.AverageWallNormal = (InOutState.AverageWallNormal + InOutState.SecondaryTraceNormal).GetSafeNormal();
						return true;
					}
					return false;
				}
			}

			return false;
		}
	}

	return false;
}
bool UWallRunUtilityLibrary::CheckCornerAngle(const FVector& FirstWallNormal, const FVector& SecondWallNormal,
	const FWallRunSettings& Settings, float& OutAngle) {
	const FVector FirstNorm2D = FirstWallNormal.GetSafeNormal2D();
	const FVector SecondNorm2D = SecondWallNormal.GetSafeNormal2D();

	const float Dot = FMath::Clamp(FirstNorm2D.Dot(SecondNorm2D), -1.0f, 1.0f);
	OutAngle = FMath::RadiansToDegrees(FMath::Acos(Dot));

	return OutAngle <= Settings.CornerAngleLimit;
}

bool UWallRunUtilityLibrary::CheckTopArea(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState, 
	const float DeltaTime) {
	if (!Character ) {
		return false;
	}

	InOutState.bWallTopReached = false;

	const float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector CharacterLocation = Character->GetActorLocation();

	const FVector TraceStart = CharacterLocation + FVector(0.0f, 0.0f, CapsuleHalfHeight * 0.65f);
	FVector TraceEnd = TraceStart + (InOutState.AverageWallNormal * CapsuleRadius * -1.7f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	FHitResult HitResult;
	if (Character->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, Settings.TraceChannel, QueryParams)) {
		if (IsWallRunnable(HitResult.GetActor(), Settings)) {
			InOutState.LastKnownSafeHeight = CharacterLocation.Z;
			return true;
		}
	}

	TraceEnd = TraceStart + (InOutState.AverageWallNormal * CapsuleRadius * -1.1f);
	if (Character->GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, Settings.TraceChannel, 
		FCollisionShape::MakeSphere(CapsuleRadius * 0.6f), QueryParams)) {
		if (IsWallRunnable(HitResult.GetActor(), Settings)) {
			if (TraceStart.Z > HitResult.ImpactPoint.Z) {
				if (InOutState.bIsWallRunning) {
					const float DistanceAboveWall = TraceStart.Z - HitResult.ImpactPoint.Z;
					BlockWallRunAscent(Character, InOutState, Settings, CharacterLocation.Z - DistanceAboveWall, DeltaTime);
					InOutState.LastKnownSafeHeight = CharacterLocation.Z - DistanceAboveWall;
					return true;
				}
				return false;
			}
			InOutState.LastKnownSafeHeight = CharacterLocation.Z;
			return true;
		}

		if (!InOutState.bIsWallRunning) {
			return false;
		}
		AdjustActorHeight(Character, InOutState.LastKnownSafeHeight, Settings, DeltaTime);
		return false;
	}

	if (!InOutState.bIsWallRunning) {
		return false;
	}
	AdjustActorHeight(Character, InOutState.LastKnownSafeHeight, Settings, DeltaTime);
	return false;
}
bool UWallRunUtilityLibrary::CheckBottomArea(const ACharacter* Character, const FWallRunSettings& Settings, const FWallRunState& State) {
	if (!Character || !Character->GetCapsuleComponent() || !Character->GetWorld()) {
		return false;
	}

	const float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();

	const FVector TraceStart = Character->GetActorLocation() - FVector(0.0f, 0.0f, CapsuleHalfHeight * 0.85f);
	const FVector TraceEnd = TraceStart + (State.AverageWallNormal * -2.0f * CapsuleRadius);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	FHitResult HitResult;
	const bool bHit = Character->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, Settings.TraceChannel, 
		QueryParams);
	const bool bValidWallAtBottom = bHit && IsWallRunnable(HitResult.GetActor(), Settings);
	return bValidWallAtBottom;
}

bool UWallRunUtilityLibrary::TraceRelativeToWall(const ACharacter* Character, FHitResult& OutHitResult, 
	const FVector& CurrentWallNormal, const FVector& WallLocation, float EndOffsetAwayFromWall, float StartOffsetBackwards, 
	float StartOffsetAwayFromWall, float EndOffsetBackwards, EWallRunSide Side, ECollisionChannel TraceChannel) {
	if (!Character || !Character->GetCapsuleComponent() || !Character->GetWorld()) {
		return false;
	}

	const float CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float SideZ = (Side == EWallRunSide::Right) ? 1.0f : -1.0f;
	const FVector ForwardDirection = CurrentWallNormal.Cross(FVector(0.0f, 0.0f, SideZ));

	const FVector StartFwdOffset = ForwardDirection * CapsuleRadius * StartOffsetBackwards;
	const FVector StartSideOffset = CurrentWallNormal * CapsuleRadius * StartOffsetAwayFromWall;
	const FVector TraceStart = WallLocation + StartFwdOffset + StartSideOffset;

	const FVector EndFwdOffset = ForwardDirection * CapsuleRadius * EndOffsetBackwards;
	const FVector EndSideOffset = -CurrentWallNormal * CapsuleRadius * EndOffsetAwayFromWall;
	const FVector TraceEnd = TraceStart + EndSideOffset + EndFwdOffset;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	const bool bHit = Character->GetWorld()->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, TraceChannel, QueryParams);
	return bHit;
}

void UWallRunUtilityLibrary::HugWall(ACharacter* Character, const FWallRunSettings& Settings, const FWallRunState& State, 
	const float DeltaTime) {
	if (!Character) {
		return;
	}

	const float DistanceFromWall = FMath::Max(State.DistanceFromWall, 0.0f);
	const float Step = FMath::Min(DistanceFromWall, Settings.CapsuleAdjustmentSpeed * DeltaTime);
	if (Step > 0.0f) {
		Character->AddActorWorldOffset(State.AverageWallNormal * -Step, true);
	}
}
void UWallRunUtilityLibrary::UpdateMeshWallOffset(const ACharacter* Character, const FWallRunSettings& Settings, 
	FWallRunState& InOutState, const float DeltaTime) {
	if (!Character) {
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh) {
		return;
	}

	if (!InOutState.bDefaultMeshLocationInitialized) {
		InOutState.DefaultMeshRelativeLocation = Mesh->GetRelativeLocation();
		InOutState.bDefaultMeshLocationInitialized = true;
	}

	const FVector WorldDirTowardsWall = -InOutState.AverageWallNormal;
	const FVector LocalDirTowardsWall = Character->GetActorTransform().InverseTransformVectorNoScale(WorldDirTowardsWall);

	const float WallLateralSign = (LocalDirTowardsWall.Y >= 0.0f) ? 1.0f : -1.0f;
	const float TargetOffsetY = WallLateralSign * Settings.MeshWallOffset;

	InOutState.CurrentMeshOffset = FMath::FInterpTo(InOutState.CurrentMeshOffset, TargetOffsetY, DeltaTime, Settings.MeshInterpSpeed);

	FVector NewRelativeLoc = InOutState.DefaultMeshRelativeLocation;
	NewRelativeLoc.Y += InOutState.CurrentMeshOffset;
	Mesh->SetRelativeLocation(NewRelativeLoc);
}
void UWallRunUtilityLibrary::ResetMeshWallOffset(const ACharacter* Character, FWallRunState& InOutState, const float DeltaTime) {
	if (!Character || !InOutState.bDefaultMeshLocationInitialized) {
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh) {
		return;
	}

	if (DeltaTime > 0.0f) {
		InOutState.CurrentMeshOffset = FMath::FInterpTo(InOutState.CurrentMeshOffset, 0.0f, DeltaTime, 14.0f);
		if (FMath::Abs(InOutState.CurrentMeshOffset) < 0.1f) {
			InOutState.CurrentMeshOffset = 0.0f;
		}
	}
	else {
		InOutState.CurrentMeshOffset = 0.0f;
	}

	FVector NewRelativeLoc = InOutState.DefaultMeshRelativeLocation;
	NewRelativeLoc.Y += InOutState.CurrentMeshOffset;
	Mesh->SetRelativeLocation(NewRelativeLoc);
}

void UWallRunUtilityLibrary::UpdateWallRunMovement(ACharacter* Character, const FWallRunSettings& Settings, 
	FWallRunState& InOutState, const float DeltaTime) {
	if (!Character || !Character->GetCharacterMovement()) {
		return;
	}

	HugWall(Character, Settings, InOutState, DeltaTime);
	UpdateMeshWallOffset(Character, Settings, InOutState, DeltaTime);

	const FVector CrossProd = InOutState.AverageWallNormal.Cross(FVector(0.0f, 0.0f, 
		(InOutState.WallSide == EWallRunSide::Right) ? -1.0f : 1.0f));

	constexpr float ArcStartTime = 0.0f;
	constexpr float ArcEndTime = 2.5f;
	constexpr float InitialArcMultiplier = 1.0f; // Upward pitch at start
	constexpr float TerminalArcMultiplier = -2.0f; // downward pitch as run progresses

	const float MappedWallRunDuration = FMath::GetMappedRangeValueClamped(FVector2D(ArcStartTime, ArcEndTime),
		FVector2D(InitialArcMultiplier, TerminalArcMultiplier), InOutState.CurrentWallRunDuration);
	float FinalArcAmount = Settings.ArcAmount * MappedWallRunDuration;
	if (InOutState.bWallTopReached) {
		FinalArcAmount = FMath::Min(FinalArcAmount, 0.0f);
	}
	FVector WorldDirection = FVector(CrossProd.X, CrossProd.Y, FinalArcAmount);
	WorldDirection.Normalize();

	Character->AddMovementInput(WorldDirection, 2.0f, false);

	if (Character->GetVelocity().Size2D() < Settings.WallRunSpeed) {
		const FVector WorldDirectionNormalized = WorldDirection.GetSafeNormal();
		Character->GetCharacterMovement()->Velocity *= 1.05f;
		Character->GetCharacterMovement()->Velocity = WorldDirectionNormalized * Character->GetCharacterMovement()->Velocity.Size();
	}

	const FRotator CharRot = Character->GetActorRotation();
	const FRotator TargetRotation = CrossProd.ToOrientationRotator();
	Character->SetActorRotation(FMath::RInterpConstantTo(CharRot, TargetRotation, DeltaTime, Settings.MaxWallRunTurnRate));

	InOutState.CurrentWallRunDuration += DeltaTime;

	CheckTopArea(Character, Settings, InOutState, DeltaTime);
	CheckBottomArea(Character, Settings, InOutState);
	UpdateAnimationValues(Character, InOutState, DeltaTime);
}
void UWallRunUtilityLibrary::ApplyWallRunMovementSettings(const ACharacter* Character, const FWallRunSettings& Settings, 
	FWallRunState& InOutState) {
	if (!Character) {
		return;
	}

	UCharacterMovementComponent* CMC = Character->GetCharacterMovement();

	InOutState.SavedMaxFlySpeed = CMC->MaxFlySpeed;
	InOutState.bSavedOrientRotation = CMC->bOrientRotationToMovement;
	InOutState.SavedMaxAcceleration = CMC->MaxAcceleration;
	InOutState.SavedBrakingDeceleration = CMC->BrakingDecelerationFlying;

	CMC->SetMovementMode(MOVE_Flying);
	CMC->MaxFlySpeed = Settings.WallRunSpeed;
	CMC->bOrientRotationToMovement = false;
	CMC->MaxAcceleration = Settings.WallRunAcceleration;
	CMC->BrakingDecelerationFlying = Settings.WallRunBrakingDeceleration;
	CMC->Velocity = FVector::ZeroVector;

	InOutState.CurrentWallRunDuration = 0.0f;
	InOutState.PreviousRotation = Character->GetActorRotation();
}
void UWallRunUtilityLibrary::RestoreMovementSettings(const ACharacter* Character, const FWallRunState& State, 
	const EMovementMode NewMovementMode) {
	if (!Character) {
		return;
	}

	UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
	CMC->MaxFlySpeed = State.SavedMaxFlySpeed;
	CMC->bOrientRotationToMovement = State.bSavedOrientRotation;
	CMC->MaxAcceleration = State.SavedMaxAcceleration;
	CMC->BrakingDecelerationFlying = State.SavedBrakingDeceleration;
	CMC->SetMovementMode(NewMovementMode);

	if (Character->GetVelocity().Size2D() > CMC->MaxWalkSpeed) {
		FVector2D NormVelocity2D = FVector2D(Character->GetVelocity());
		NormVelocity2D.Normalize();
		CMC->Velocity = FVector(NormVelocity2D * CMC->MaxWalkSpeed, 0.0f);
	}

	if (!State.bIsWallJumping) {
		CMC->StopMovementImmediately();
	}
}

void UWallRunUtilityLibrary::CalculateWallJumpVelocity(const FWallRunSettings& Settings, const FWallRunState& State, 
	FVector& OutLaunchVelocity, FRotator& OutTargetRotation) {
	const FVector WallForward = State.AverageWallNormal.Cross(FVector(0.0f, 0.0f, 
		(State.WallSide == EWallRunSide::Right) ? -1.0f : 1.0f)).GetSafeNormal();

	const float JumpAngleRad = FMath::DegreesToRadians(Settings.WallJumpAngle);

	const FVector JumpForwardVector = WallForward * FMath::Cos(JumpAngleRad);
	const FVector JumpRightVector = State.AverageWallNormal * FMath::Sin(JumpAngleRad);
	const FVector JumpHorizontalDir = (JumpForwardVector + JumpRightVector).GetSafeNormal();

	OutLaunchVelocity = (JumpHorizontalDir * Settings.WallJumpHorizontalVelocity) + FVector(0.0f, 0.0f, 
		Settings.WallJumpVerticalVelocity);
	OutTargetRotation = JumpHorizontalDir.Rotation();
}
void UWallRunUtilityLibrary::ApplyWallJumpMovementSettings(const ACharacter* Character, const FWallRunSettings& Settings, 
	FWallRunState& InOutState) {
	if (!Character) {
		return;
	}

	UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
	InOutState.SavedGravityScale = CMC->GravityScale;
	InOutState.SavedAirControl = CMC->AirControl;

	CMC->GravityScale = Settings.WallJumpGravity;
	CMC->AirControl = 0.0f;

	InOutState.bSavedOverrideBrakingDecelerationFalling = Settings.bOverrideBrakingDecelerationFalling;
	if (InOutState.bSavedOverrideBrakingDecelerationFalling) {
		InOutState.SavedBrakingDecelerationFalling = CMC->BrakingDecelerationFalling;
		CMC->BrakingDecelerationFalling = Settings.WallJumpBrakingDecelerationFalling;
	}
}
void UWallRunUtilityLibrary::RestoreWallJumpMovementSettings(const ACharacter* Character, const FWallRunState& State) {
	if (!Character) {
		return;
	}

	UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
	CMC->GravityScale = State.SavedGravityScale;
	CMC->AirControl = State.SavedAirControl;

	if (State.bSavedOverrideBrakingDecelerationFalling) {
		CMC->BrakingDecelerationFalling = State.SavedBrakingDecelerationFalling;
	}
}
void UWallRunUtilityLibrary::UpdateWallJumpRotation(ACharacter* Character, const FWallRunSettings& Settings, 
	FWallRunState& InOutState, const float DeltaTime) {
	if (!Character || !InOutState.bIsWallJumping || !InOutState.bIsRotatingForWallJump || DeltaTime <= 0.0f) {
		return;
	}

	const float RotationTime = FMath::Max(Settings.WallJumpRotationTime, UE_KINDA_SMALL_NUMBER);
	InOutState.CurrentWallJumpTimeForRotation += DeltaTime;

	const float Alpha = FMath::Clamp(InOutState.CurrentWallJumpTimeForRotation / RotationTime, 0.0f, 1.0f);
	Character->SetActorRotation(FQuat::Slerp(Character->GetActorQuat(), InOutState.TargetWallJumpRotation.Quaternion(), Alpha));

	if (Alpha >= 1.0f) {
		InOutState.bIsRotatingForWallJump = false;
		InOutState.CurrentWallJumpTimeForRotation = 0.0f;
	}
}

void UWallRunUtilityLibrary::UpdateAnimationValues(const ACharacter* Character, FWallRunState& InOutState, const float DeltaTime) {
	if (!Character) {
		return;
	}

	const FVector NormalizedVelocity = Character->GetVelocity().GetSafeNormal();
	const float DotProd = NormalizedVelocity.Dot(FVector(NormalizedVelocity.X, NormalizedVelocity.Y, 0.0f));
	InOutState.CurrentArcAngle = UKismetMathLibrary::DegAcos(DotProd) * FMath::Sign(Character->GetVelocity().Z);

	const FRotator CurrentRotation = Character->GetActorRotation();
	const FVector DeltaRotVector = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, InOutState.PreviousRotation).Vector();

	float TurnRate = UKismetMathLibrary::MapRangeClamped((DeltaRotVector * 100.f).Y, -10.0f, 10.0f, 
		-1.0f, 1.0f);
	TurnRate = (InOutState.WallSide == EWallRunSide::Right) ? TurnRate : -TurnRate;
	TurnRate = FMath::Max(TurnRate, 0.0f);

	InOutState.CurrentTurnRate = FMath::FInterpTo(InOutState.CurrentTurnRate, TurnRate, DeltaTime > 0.0f ? 
		DeltaTime : 0.01f, 10.0f);
	InOutState.PreviousRotation = CurrentRotation;
}

bool UWallRunUtilityLibrary::CanStartWallRun(const ACharacter* Character, const FWallRunState& State, const UAbilitySystemComponent* ASC, 
	const FGameplayTagContainer* BlockingTags) {
	if (!Character || !State.bWallRunningAllowed || State.bIsWallRunning) {
		return false;
	}

	if (ASC && BlockingTags && ASC->HasAnyMatchingGameplayTags(*BlockingTags)) {
		return false;
	}

	const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	return Movement && Movement->MovementMode == MOVE_Falling;
}
void UWallRunUtilityLibrary::BeginWallRun(const ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState,
	const EWallRunSide Side) {
	InOutState.bIsWallRunning = true;
	InOutState.WallSide = Side;
	InOutState.CurrentWallRunDuration = 0.0f;
	InOutState.bSpeedTooLow = false;

	ApplyWallRunMovementSettings(Character, Settings, InOutState);
	SetWallRunTagCount(Character, (Side == EWallRunSide::Left) ? WallRunGameplayTags::Status_WallRunning_Left :
		WallRunGameplayTags::Status_WallRunning_Right, false);
}
void UWallRunUtilityLibrary::EndWallRun(const ACharacter* Character, FWallRunState& InOutState, const EMovementMode NewMovementMode,
	const float DisableDuration) {
	InOutState.bIsWallRunning = false;

	SetWallRunTagCount(Character, WallRunGameplayTags::Status_WallRunning_Left, true);
	SetWallRunTagCount(Character, WallRunGameplayTags::Status_WallRunning_Right, true);

	RestoreMovementSettings(Character, InOutState, NewMovementMode);

	if (DisableDuration > 0.0f) {
		InOutState.bWallRunningAllowed = false;
	}
}

bool UWallRunUtilityLibrary::BeginWallJump(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState,
	const FGameplayTag& JumpingTag, FVector& OutLaunchVelocity, const TFunction<void()>& OnWallRunStopped) {
	if (!Character || !InOutState.bIsWallRunning) {
		return false;
	}

	FRotator TargetRotation;
	CalculateWallJumpVelocity(Settings, InOutState, OutLaunchVelocity, TargetRotation);

	InOutState.bIsWallJumping = true;
	InOutState.TargetWallJumpRotation = TargetRotation;
	InOutState.bIsRotatingForWallJump = true;
	InOutState.CurrentWallJumpTimeForRotation = 0.0f;

	OnWallRunStopped();

	ApplyWallJumpMovementSettings(Character, Settings, InOutState);
	Character->LaunchCharacter(OutLaunchVelocity, true, true);

	SetWallRunTagCount(Character, JumpingTag, false);
	return true;
}
bool UWallRunUtilityLibrary::EndWallJump(const ACharacter* Character, FWallRunState& InOutState, const FGameplayTag& JumpingTag) {
	if (!InOutState.bIsWallJumping) {
		return false;
	}

	InOutState.bIsWallJumping = false;
	InOutState.bIsRotatingForWallJump = false;
	InOutState.CurrentWallJumpTimeForRotation = 0.0f;

	SetWallRunTagCount(Character, JumpingTag, true);
	RestoreWallJumpMovementSettings(Character, InOutState);
	return true;
}

EWallRunStopReason UWallRunUtilityLibrary::TickActiveWallRun(ACharacter* Character, const FWallRunSettings& Settings,
	FWallRunState& InOutState, const float DeltaTime) {
	if (Settings.WallRunDurationLimit > 0.0f && InOutState.CurrentWallRunDuration >= Settings.WallRunDurationLimit) {
		return EWallRunStopReason::Timeout;
	}

	if (!DetectWall(Character, InOutState.WallSide, Settings, InOutState)) {
		return EWallRunStopReason::LostWall;
	}

	const FVector WallTraceStart = Character->GetActorLocation();
	const FVector WallTraceEnd = WallTraceStart + (InOutState.AverageWallNormal * -200.0f);

	FHitResult WallCheckHit;
	FCollisionQueryParams WallCheckParams;
	WallCheckParams.AddIgnoredActor(Character);

	if (!Character->GetWorld()->LineTraceSingleByChannel(WallCheckHit, WallTraceStart, WallTraceEnd, Settings.TraceChannel,
		WallCheckParams)) {
		return EWallRunStopReason::LostWall;
	}

	UpdateWallRunMovement(Character, Settings, InOutState, DeltaTime);
	InOutState.PreviousVelocity = Character->GetVelocity();
	return EWallRunStopReason::None;
}
float UWallRunUtilityLibrary::WallRunStopLockoutDuration(const EWallRunStopReason Reason) {
	switch (Reason) {
	case EWallRunStopReason::Timeout: {
		return 0.5f;
	}
	case EWallRunStopReason::LostWall: {
		return 0.35f;
	}
	default: {
		return 0.0f;
	}
	}
}
void UWallRunUtilityLibrary::TickWallJumpIdle(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState,
	const float DeltaTime, const TFunction<void(bool)>& OnSetTickStatus) {
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement) {
		return;
	}

	if (FMath::Abs(InOutState.CurrentMeshOffset) > 0.01f) {
		ResetMeshWallOffset(Character, InOutState, DeltaTime);

		if (FMath::Abs(InOutState.CurrentMeshOffset) <= 0.01f && !InOutState.bIsWallJumping && Movement->MovementMode != MOVE_Falling) {
			OnSetTickStatus(false);
		}
	}

	UpdateWallJumpRotation(Character, Settings, InOutState, DeltaTime);
}
bool UWallRunUtilityLibrary::TryFindWallRunStart(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState,
	const float DeltaTime) {
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement || !InOutState.bWallRunningAllowed || Movement->MovementMode != MOVE_Falling) {
		return false;
	}

	const float HorizontalSpeed = Character->GetVelocity().Size2D();
	const bool bHasMovementInput = !Character->GetLastMovementInputVector().IsNearlyZero() ||
		!Movement->GetCurrentAcceleration().IsNearlyZero();
	if (HorizontalSpeed < Settings.MinStartSpeed && !bHasMovementInput) {
		return false;
	}

	const float CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector TraceStart = Character->GetActorLocation() + (Character->GetActorForwardVector() * CapsuleRadius * 0.5f);

	FRotator CameraRotation = Character->GetActorRotation();
	if (const AController* Controller = Character->GetController()) {
		CameraRotation = Controller->GetControlRotation();
	}

	const FVector TraceEnd = TraceStart + (CameraRotation.Vector() * CapsuleRadius * 2.0f);

	FHitResult FrontHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	if (Character->GetWorld()->SweepSingleByChannel(FrontHit, TraceStart, TraceEnd, FQuat::Identity, Settings.TraceChannel,
		FCollisionShape::MakeSphere(CapsuleRadius * Settings.PrimaryTraceRadiusMultiplier), QueryParams)) {
		if (IsWallRunnable(FrontHit.GetActor(), Settings)) {
			const EWallRunSide Side = DetermineWallSideFromCamera(Character, FrontHit.ImpactNormal);
			if (DetectWall(Character, Side, Settings, InOutState)) {
				float FrontCornerAngle = 0.0f;
				if (CheckCornerAngle(FrontHit.ImpactNormal, InOutState.PrimaryTraceNormal, Settings, 
					FrontCornerAngle)) {
					if (CheckBottomArea(Character, Settings, InOutState) &&
						CheckTopArea(Character, Settings, InOutState, DeltaTime)) {
						return true;
					}
				}
			}
		}
	}

	if (DetectWall(Character, EWallRunSide::Right, Settings, InOutState)) {
		if (CheckBottomArea(Character, Settings, InOutState) && CheckTopArea(Character, Settings, InOutState, DeltaTime)) {
			return true;
		}
	}

	if (DetectWall(Character, EWallRunSide::Left, Settings, InOutState)) {
		if (CheckBottomArea(Character, Settings, InOutState) && CheckTopArea(Character, Settings, InOutState, DeltaTime)) {
			return true;
		}
	}

	return false;
}

bool UWallRunUtilityLibrary::UpdateSpeedMonitor(const ACharacter* Character, FWallRunState& InOutState) {
	if (!Character || !InOutState.bIsWallRunning) {
		return false;
	}

	const float Speed = Character->GetVelocity().Size2D();
	if (Speed < 300.0f) {
		if (InOutState.bSpeedTooLow) {
			InOutState.bSpeedTooLow = false;
			return true;
		}
		InOutState.bSpeedTooLow = true;
	}
	else {
		InOutState.bSpeedTooLow = false;
	}

	return false;
}
void UWallRunUtilityLibrary::SetWallJumpAirControl(const ACharacter* Character, const FWallRunSettings& Settings) {
	if (Character && Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->AirControl = Settings.WallJumpAirControl;
	}
}
void UWallRunUtilityLibrary::SetWallRunTagCount(const ACharacter* Character, const FGameplayTag& Tag, const bool bRemove) {
	if (!Character || !Tag.IsValid()) {
		return;
	}

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character)) {
		ASC->SetLooseGameplayTagCount(Tag, bRemove ? 0 : 1);
	}
}

void UWallRunUtilityLibrary::HandleMovementModeChanged(const ACharacter* Character, FWallRunState& InOutState, const 
	TFunction<void(bool)>& OnSetTickStatus, const TFunction<void()>& OnStopWallJump) {
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement) {
		return;
	}

	switch (Movement->MovementMode) {
	case MOVE_Falling: {
		OnSetTickStatus(true);
		InOutState.PreviousVelocity = Character->GetVelocity();
		break;
	}
	case MOVE_Flying: {
		if (!InOutState.bIsWallRunning) {
			OnSetTickStatus(false);
			if (InOutState.bIsWallJumping) {
				OnStopWallJump();
			}
		}
		break;
	}
	default: {
		if (InOutState.bIsWallJumping) {
			OnStopWallJump();
		}
		if (FMath::Abs(InOutState.CurrentMeshOffset) <= 0.01f) {
			OnSetTickStatus(false);
		}
		break;
	}
	}
}
void UWallRunUtilityLibrary::HandleCharacterHit(const ACharacter* Character, const FWallRunState& InOutState, const FHitResult& Hit,
	FTimerHandle& LockoutTimerHandle, const TFunction<void()>& OnStopWallRun, const TFunction<void()>& OnLockoutCleared) {
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement || !Movement->IsWalkable(Hit)) {
		return;
	}

	if (InOutState.bIsWallRunning) {
		const float BelowFeet = Character->GetActorLocation().Z - Character->GetCapsuleComponent()->
			GetScaledCapsuleHalfHeight_WithoutHemisphere();

		if (Hit.ImpactPoint.Z <= BelowFeet) {
			OnStopWallRun();
		}
	}
	else if (LockoutTimerHandle.IsValid()) {
		Character->GetWorld()->GetTimerManager().ClearTimer(LockoutTimerHandle);
		OnLockoutCleared();
	}
}

void UWallRunUtilityLibrary::StartWallRunTimers(const UWorld* World, const FWallRunSettings& Settings, FTimerHandle& DurationHandle,
	FTimerHandle& SpeedHandle, const FTimerDelegate& OnTimeout, const FTimerDelegate& OnSpeedCheck) {
	if (!World) {
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	if (Settings.WallRunDurationLimit > 0.0f) {
		TimerManager.SetTimer(DurationHandle, OnTimeout, Settings.WallRunDurationLimit, false);
	}
	TimerManager.SetTimer(SpeedHandle, OnSpeedCheck, 0.25f, true);
}
void UWallRunUtilityLibrary::ClearWallRunTimers(const UWorld* World, FTimerHandle& DurationHandle, FTimerHandle& SpeedHandle) {
	if (!World) {
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(DurationHandle);
	TimerManager.ClearTimer(SpeedHandle);
}
