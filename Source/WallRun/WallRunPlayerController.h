// Copyright: Elixir Studios

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WallRunPlayerController.generated.h"

// Forward Declarations
class UInputMappingContext;

UCLASS(abstract)
class WALLRUN_API AWallRunPlayerController : public APlayerController {
	GENERATED_BODY()

protected:
	//================================================================================================================
	// FUNCTIONS
	//================================================================================================================
	virtual void SetupInputComponent() override;

	//================================================================================================================
	// PROPERTIES & VARIABLES
	//================================================================================================================

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;
};
