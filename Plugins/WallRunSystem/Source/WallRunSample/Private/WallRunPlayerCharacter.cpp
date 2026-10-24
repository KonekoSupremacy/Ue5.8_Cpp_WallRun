// Copyright: KonekoSupremacy

// Game Includes
#include "WallRunPlayerCharacter.h"
#include "WallRunComponent.h"

AWallRunPlayerCharacter::AWallRunPlayerCharacter() {
	PrimaryActorTick.bCanEverTick = true;

	WallRunComponent = CreateDefaultSubobject<UWallRunComponent>("WallRunComponent");
}

bool AWallRunPlayerCharacter::CanMove() const {
	if (WallRunComponent && WallRunComponent->IsWallRunning()) {
		return false;
	}
	return true;
}

void AWallRunPlayerCharacter::DoJump() {
	if (WallRunComponent && WallRunComponent->IsWallRunning()) {
		WallRunComponent->StartWallJump();
		return;
	}

	Super::DoJump();
}
