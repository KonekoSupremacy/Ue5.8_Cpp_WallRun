// No Copyright!

// Game Includes
#include "WallRunUtilityLibrary.h"

// Engine Includes
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"

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

	const FVector Start = ActorLoc + (CamForward * CapsuleRadius * 0.5f);
	const float PrimarySideMultiplier = (Side == EWallRunSide::Right) ? 1.0f : -1.0f;
	const FVector PrimaryDir = CamRight * PrimarySideMultiplier;
	const FVector End = Start + (PrimaryDir * CapsuleRadius * 2.0f);

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
				.ImpactNormal, EffectiveSide, Settings, CornerAngle)) {
				// look ahead check to prevent false 0 deg
				FHitResult FarHitResult;
				if (TraceRelativeToWall(Character, FarHitResult, InOutState.PrimaryTraceNormal, InOutState
					.CurrentWallLocation, 3.0f, -2.5f, 0.75f, -2.5f, 
					EffectiveSide, Settings.TraceChannel)) {
					if (IsWallRunnable(FarHitResult.GetActor(), Settings)) {
						float FarAngle = 0.0f;
						if (!CheckCornerAngle(InOutState.PrimaryTraceNormal, FarHitResult
							.ImpactNormal, EffectiveSide, Settings, FarAngle)) {
							return false;
						}
					}
				}

				InOutState.SecondaryTraceNormal = SecondaryHitResult.ImpactNormal;
				InOutState.AverageWallNormal = (InOutState.AverageWallNormal + InOutState.SecondaryTraceNormal).GetSafeNormal();
				return true;
			}

			FHitResult BackupHitResult;
			if (TraceRelativeToWall(Character, BackupHitResult, InOutState.PrimaryTraceNormal, InOutState
				.CurrentWallLocation, 3.0f, -2.0f, 0.75f, -2.5f, 
				EffectiveSide, Settings.TraceChannel)) {
				if (IsWallRunnable(BackupHitResult.GetActor(), Settings)) {
					if (CheckCornerAngle(InOutState.PrimaryTraceNormal, BackupHitResult.ImpactNormal, 
						EffectiveSide, Settings, CornerAngle)) {
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
						.ImpactNormal, EffectiveSide, Settings, CornerAngle)) {
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
bool UWallRunUtilityLibrary::CheckCornerAngle(const FVector& FirstWallNormal, const FVector& SecondWallNormal, const EWallRunSide Side, 
	const FWallRunSettings& Settings, float& OutAngle) {
	const FVector FirstNorm2D = FirstWallNormal.GetSafeNormal2D();
	const FVector SecondNorm2D = SecondWallNormal.GetSafeNormal2D();

	const float Dot = FirstNorm2D.Dot(SecondNorm2D);
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));
	OutAngle = AngleDeg;

	const FVector Cross = FirstWallNormal.Cross(SecondWallNormal);
	const bool bIsLeftOutside = (Cross.Z < 0.0f && Side == EWallRunSide::Left);
	const bool bIsRightOutside = (Cross.Z >= 0.0f && Side == EWallRunSide::Right);

	const float AngleLimit = (bIsLeftOutside || bIsRightOutside) ? Settings.OutsideCornerAngleLimit : Settings.InsideCornerAngleLimit;
	return OutAngle <= AngleLimit;
}

bool UWallRunUtilityLibrary::CheckTopArea(ACharacter* Character, const FWallRunSettings& Settings, FWallRunState& InOutState) {
	if (!Character ) {
		return false;
	}

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
					const FVector FinalLocation(CharacterLocation.X, CharacterLocation.Y, CharacterLocation.Z - DistanceAboveWall);
					Character->SetActorLocation(FinalLocation, true);
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
		Character->SetActorLocation(FVector(CharacterLocation.X, CharacterLocation.Y, InOutState.LastKnownSafeHeight), true);
		return false;
	}

	if (!InOutState.bIsWallRunning) {
		return false;
	}
	Character->SetActorLocation(FVector(CharacterLocation.X, CharacterLocation.Y, InOutState.LastKnownSafeHeight), true);
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

void UWallRunUtilityLibrary::HugWall(ACharacter* Character, const FWallRunState& State) {
	if (!Character) {
		return;
	}

	const float DistanceFromWall = FMath::Max(State.DistanceFromWall, 0.0f);
	Character->AddActorWorldOffset(State.AverageWallNormal * -DistanceFromWall, true);
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
	FWallRunState& InOutState, const float DeltaTime, FVector& OutWorldDirection, FRotator& OutTargetRotation) {
	if (!Character || !Character->GetCharacterMovement()) {
		OutWorldDirection = FVector::ZeroVector;
		OutTargetRotation = FRotator::ZeroRotator;
		return;
	}

	HugWall(Character, InOutState);
	UpdateMeshWallOffset(Character, Settings, InOutState, DeltaTime);

	const FVector CrossProd = InOutState.AverageWallNormal.Cross(FVector(0.0f, 0.0f, 
		(InOutState.WallSide == EWallRunSide::Right) ? -1.0f : 1.0f));

	constexpr float ArcStartTime = 0.0f;
	constexpr float ArcEndTime = 2.5f;
	constexpr float InitialArcMultiplier = 1.0f; // Upward pitch at start
	constexpr float TerminalArcMultiplier = -2.0f; // downward pitch as run progresses

	const float MappedWallRunDuration = FMath::GetMappedRangeValueClamped(FVector2D(ArcStartTime, ArcEndTime),
		FVector2D(InitialArcMultiplier, TerminalArcMultiplier), InOutState.CurrentWallRunDuration);
	const float FinalArcAmount = Settings.ArcAmount * MappedWallRunDuration;
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
	OutTargetRotation = TargetRotation;
	if (!CharRot.Equals(TargetRotation, 1.0f)) {
		constexpr float MinInterpSpeed = 13.0f;
		constexpr float MaxInterpSpeed = 25.0f;
		const float VelocityFactor = FMath::GetMappedRangeValueClamped(
			FVector2D(800.0f, 1600.0f),
			FVector2D(MinInterpSpeed, MaxInterpSpeed),
			Character->GetVelocity().Length()
		);
		const float DistanceFactor = 1.0f - (CharRot - TargetRotation).GetNormalized().Yaw / 180.0f;
		const float RotationSpeed = FMath::Clamp(VelocityFactor * DistanceFactor, MinInterpSpeed, MaxInterpSpeed);

		Character->SetActorRotation(FMath::RInterpTo(CharRot, TargetRotation, 
			DeltaTime > 0.0f ? DeltaTime : 0.01f, RotationSpeed));
	}

	InOutState.CurrentWallRunDuration += DeltaTime;

	CheckTopArea(Character, Settings, InOutState);
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
