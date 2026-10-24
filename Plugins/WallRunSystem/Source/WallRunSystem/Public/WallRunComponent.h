// Copyright: KonekoSupremacy

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallRunTypes.h"
#include "WallRunComponent.generated.h"

// Forward Declarations
class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWallRunStartedSignature, EWallRunSide, Side);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWallRunEndedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWallJumpSignature, const FVector&, LaunchVelocity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWallSideChangedSignature, EWallRunSide, NewSide);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WALLRUNSYSTEM_API UWallRunComponent : public UActorComponent {
	GENERATED_BODY()

public:
	//================================================================================================================
	// FUNCTIONS
	//================================================================================================================
	UWallRunComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Wall Run")
	bool StartWallRun(EWallRunSide Side);
	UFUNCTION(BlueprintCallable, Category = "Wall Run")
	void StopWallRun(EMovementMode NewMovementMode = MOVE_Falling, float DisableDuration = 0.0f);
	
	UFUNCTION(BlueprintCallable, Category = "Wall Run")
	bool StartWallJump();
	UFUNCTION(BlueprintCallable, Category = "Wall Run")
	void StopWallJump();

	UFUNCTION(BlueprintPure, Category = "Wall Run")
	FORCEINLINE bool IsWallRunning() const { return State.bIsWallRunning; }
	UFUNCTION(BlueprintPure, Category = "Wall Run")
	FORCEINLINE bool IsWallJumping() const { return State.bIsWallJumping; }

	FORCEINLINE const FWallRunState& GetWallRunState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "Wall Run")
	FORCEINLINE EWallRunSide GetWallSide() const { return State.WallSide; }

	UFUNCTION(BlueprintPure, Category = "Wall Run")
	FORCEINLINE float GetArcAngle() const { return State.CurrentArcAngle; }
	UFUNCTION(BlueprintPure, Category = "Wall Run")
	FORCEINLINE float GetTurnRate() const { return State.CurrentTurnRate; }
	
	//================================================================================================================
	// PROPERTIES & VARIABLES
	//================================================================================================================
	/** Configurable wall run properties. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run")
	FWallRunSettings Settings;

	/** Current runtime state of wall running. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run")
	FWallRunState State;

	UPROPERTY(BlueprintAssignable, Category = "Wall Run|Events")
	FOnWallRunStartedSignature OnWallRunStarted;
	UPROPERTY(BlueprintAssignable, Category = "Wall Run|Events")
	FOnWallSideChangedSignature OnWallSideChanged;
	UPROPERTY(BlueprintAssignable, Category = "Wall Run|Events")
	FOnWallRunEndedSignature OnWallRunEnded;

	UPROPERTY(BlueprintAssignable, Category = "Wall Run|Events")
	FOnWallJumpSignature OnWallJump;

protected:
	//================================================================================================================
	// FUNCTIONS
	//================================================================================================================
	UFUNCTION()
	void OnCharacterMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);

	UFUNCTION()
	void OnCharacterHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	void SetTickStatus(bool bShouldTick);
	void EventTick(float DeltaTime);
	void WallRunTimeout();
	void EnableDelayedAirControl() const;
	void MonitorSpeed();
	void ResetWallRunLockout();

	//================================================================================================================
	// PROPERTIES & VARIABLES
	//================================================================================================================
	UPROPERTY()
	TObjectPtr<ACharacter> CharacterOwner = nullptr;

	FTimerHandle WallRunDurationTimerHandle;
	FTimerHandle SpeedMonitorTimerHandle;
	FTimerHandle AirControlTimerHandle;
	FTimerHandle LockoutTimerHandle;
};
