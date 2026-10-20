// Copyright: Elixir Studios

// Game Includes
#include "WallRunPlayerController.h"

// Engine Includes
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void AWallRunPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	if (IsLocalPlayerController()) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
			GetLocalPlayer())) {
			for (const UInputMappingContext* CurrentContext : DefaultMappingContexts) {
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}
