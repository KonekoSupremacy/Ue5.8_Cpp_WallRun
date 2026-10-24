// Copyright: KonekoSupremacy

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace WallRunGameplayTags {
	// Status Tags
	WALLRUNSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_WallRunning);
	WALLRUNSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_WallRunning_Left);
	WALLRUNSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_WallRunning_Right);
	WALLRUNSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_WallJumping);

	// Blocking Tags
	WALLRUNSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_BlockingAction_WallRun);

	// Event Tags
	WALLRUNSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Movement_WallJump);
	WALLRUNSYSTEM_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Movement_WallRun);
}
