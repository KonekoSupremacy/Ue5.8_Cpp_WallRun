// Copyright: KonekoSupremacy

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "WallRunPlayerCharacter.generated.h"

// Forward Declarations
class UWallRunComponent;

UCLASS(abstract)
class WALLRUNSAMPLE_API AWallRunPlayerCharacter : public APlayerCharacter {
	GENERATED_BODY()

public:
	//================================================================================================================
	// FUNCTIONS
	//================================================================================================================
	AWallRunPlayerCharacter();

	virtual bool CanMove() const override;
	virtual void DoJump() override;

	FORCEINLINE UWallRunComponent* GetWallRunComponent() const { return WallRunComponent; }

	//================================================================================================================
	// PROPERTIES & VARIABLES
	//================================================================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWallRunComponent> WallRunComponent;
};
