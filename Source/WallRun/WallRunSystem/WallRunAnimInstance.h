// Copyright: KonekoSupremacy

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WallRunTypes.h"
#include "WallRunAnimInstance.generated.h"

class UAbilitySystemComponent;
// Forward Declarations
class UWallRunComponent;

UCLASS()
class WALLRUN_API UWallRunAnimInstance : public UAnimInstance {
	GENERATED_BODY()

public:
	//================================================================================================================
	// FUNCTIONS
	//================================================================================================================
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	//================================================================================================================
	// PROPERTIES & VARIABLES
	//================================================================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run")
	bool bIsWallRunning = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run")
	bool bIsWallJumping = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run")
	EWallRunSide WallSide = EWallRunSide::Right;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run")
	float CurrentArcAngle = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run")
	float CurrentTurnRate = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run")
	float CurrentSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Run")
	float WallRunAnimPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run")
	FVector2D PlayRateSpeedRange = FVector2D(500.0f, 900.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run")
	FVector2D PlayRateRange = FVector2D(0.95f, 1.25f);

protected:
	//================================================================================================================
	// PROPERTIES & VARIABLES
	//================================================================================================================
	UPROPERTY(BlueprintReadOnly, Category = "Wall Run")
	TWeakObjectPtr<UWallRunComponent> WallRunComponent;
	UPROPERTY(BlueprintReadOnly, Category = "Wall Run")
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
