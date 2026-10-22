// Copyright: KonekoSupremacy

#include "WallRunGameplayTags.h"

namespace WallRunGameplayTags {
	// Status Tags
	UE_DEFINE_GAMEPLAY_TAG(Status_WallRunning, "Status.WallRunning");
	UE_DEFINE_GAMEPLAY_TAG(Status_WallRunning_Left, "Status.WallRunning.Left");
	UE_DEFINE_GAMEPLAY_TAG(Status_WallRunning_Right, "Status.WallRunning.Right");
	UE_DEFINE_GAMEPLAY_TAG(Status_WallJumping, "Status.WallJumping");

	// Blocking Tags
	UE_DEFINE_GAMEPLAY_TAG(Status_BlockingAction_WallRun, "Status.BlockingAction.WallRun");

	// Event Tags
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_WallJump, "Event.Movement.WallJump");
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_WallRun, "Event.Movement.WallRun");
}
