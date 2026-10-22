// Copyright: KonekoSupremacy

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace WallRunGameplayTags {
	// Status Tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_WallRunning);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_WallRunning_Left);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_WallRunning_Right);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_WallJumping);

	// Blocking Tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_BlockingAction_WallRun);

	// Event Tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Movement_WallJump);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Movement_WallRun);
}
