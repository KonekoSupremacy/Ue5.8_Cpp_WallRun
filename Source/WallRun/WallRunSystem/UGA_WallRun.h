// Copyright: Elixir Studios

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TickableAbility.h"
#include "WallRunTypes.h"
#include "UGA_WallRun.generated.h"

// Forward Declarations
class ACharacter;

UCLASS()
class WALLRUN_API UGA_WallRun : public UGameplayAbility, public ITickableAbility {
	GENERATED_BODY()

public:
	//================================================================================================================
	// FUNCTIONS
	//================================================================================================================
	UGA_WallRun();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void TickAbility(float DeltaSeconds) override;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Run")
	FWallRunSettings Settings;
	/** Runtime state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run")
	FWallRunState State;

	/** Tags add to char while wall jumping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wall Run|GAS")
	FGameplayTag WallJumpingTag;
	/** Tags that prevent this ability from wall runnin */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wall Run|GAS")
	FGameplayTagContainer TagsBlockingWallRun;

protected:
	//================================================================================================================
	// FUNCTIONS
	//================================================================================================================
	UFUNCTION()
	void OnCharacterMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);

	UFUNCTION()
	void OnCharacterHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	void OnJumpInputPressed();

	void SetTickStatus(bool bShouldTick);
	void WallRunTimeout();
	void EnableDelayedAirControl() const;
	void MonitorSpeed();
	void ResetWallRunLockout();

	//================================================================================================================
	// PROPERTIES & VARIABLES
	//================================================================================================================
	UPROPERTY(BlueprintReadOnly, Category = "Wall Run")
	TObjectPtr<ACharacter> CharacterAvatar = nullptr;

	FTimerHandle WallRunDurationTimerHandle;
	FTimerHandle SpeedMonitorTimerHandle;
	FTimerHandle AirControlTimerHandle;
	FTimerHandle LockoutTimerHandle;

	bool bTickEnabled = false;
	bool bJumpInputBound = false;
};
