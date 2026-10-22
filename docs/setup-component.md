# Component setup

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

Wall running through `UWallRunComponent`, a normal actor component on your character. No Ability System needed in your game.

## On this page

- [Pick a path](#pick-a-path)
- [Path A: use the included character](#path-a-use-the-included-character)
  - [Create the character Blueprint](#create-the-character-blueprint)
  - [Assign the input actions](#assign-the-input-actions)
  - [Set up the controller and game mode](#set-up-the-controller-and-game-mode)
- [Path B: add it to your own character](#path-b-add-it-to-your-own-character)
  - [Add the component](#add-the-component)
  - [Route jump to wall jump](#route-jump-to-wall-jump)
  - [Block movement input while wall running](#block-movement-input-while-wall-running)
- [Tune the settings](#tune-the-settings)
- [Mark which walls can be run on](#mark-which-walls-can-be-run-on)
- [Listen to events](#listen-to-events)
- [Hook up animation](#hook-up-animation)
- [Check it works](#check-it-works)
- [Troubleshooting](#troubleshooting)

## Pick a path

| Path | Use it when |
| :--- | :--- |
| [A: included character](#path-a-use-the-included-character) | You are starting fresh, or happy to reparent your character Blueprint |
| [B: your own character](#path-b-add-it-to-your-own-character) | You already have a character class you do not want to change |

Finish [Setup Guide](setup-guide.md) before either path.

## Path A: use the included character

`AWallRunPlayerCharacter` already owns a `UWallRunComponent`, routes jump to wall jump, and stops movement input while wall running.

### Create the character Blueprint

1. Content Browser, right click, Blueprint Class.
2. Search for `WallRunPlayerCharacter` and pick it as the parent.
3. Name it, for example `BP_WallRunCharacter`.
4. Open it, select the Mesh component, assign your skeletal mesh and set the transform so the mesh sits at the capsule bottom, facing +X.
5. Set the Anim Class on the mesh. See [Hook up animation](#hook-up-animation).

### Assign the input actions

In the Blueprint's Class Defaults, under Input, fill in:

| Property | Asset |
| :--- | :--- |
| Jump Action | An Input Action of type Digital (bool), for example `IA_Jump` |
| Move Action | An Input Action of type Axis2D, for example `IA_Move` |
| Look Action | An Input Action of type Axis2D, for example `IA_MouseLook` |

The sample project has these under `Content/Input/Actions`, with the mapping context in `Content/Input/IMC_Default`.

### Set up the controller and game mode

1. Create a Blueprint from `WallRunPlayerController`.
2. In its Class Defaults, add your mapping context (for example `IMC_Default`) to Default Mapping Contexts.
3. Create or open your Game Mode Blueprint and set Default Pawn Class to your new character and Player Controller Class to your new controller.
4. Set the level's Game Mode Override, or the project default, to that Game Mode.

## Path B: add it to your own character

Your character must derive from `ACharacter`. The component finds its owner at `BeginPlay` and hooks the character's movement mode and hit events itself.

### Add the component

In C++:

```cpp
// MyCharacter.h
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<UWallRunComponent> WallRunComponent;

// MyCharacter.cpp, in the constructor
WallRunComponent = CreateDefaultSubobject<UWallRunComponent>("WallRunComponent");
```

Or in a Blueprint: Components panel, Add, search for Wall Run Component.

### Route jump to wall jump

Wall jump is triggered by calling `StartWallJump()` while wall running. Do it from your jump input:

```cpp
void AMyCharacter::DoJump() {
	if (WallRunComponent && WallRunComponent->IsWallRunning()) {
		WallRunComponent->StartWallJump();
		return;
	}

	Jump();
}
```

In Blueprints, branch on `Is Wall Running` on your jump input event, call `Start Wall Jump` on true and `Jump` on false.

### Block movement input while wall running

The system drives your velocity along the wall, so ignore normal movement input meanwhile:

```cpp
void AMyCharacter::Move(const FInputActionValue& Value) {
	if (WallRunComponent && WallRunComponent->IsWallRunning()) {
		return;
	}

	// normal movement here
}
```

Look input should stay enabled.

## Tune the settings

Select the Wall Run Component, open the Wall Run category in Details, expand Settings. Every property is listed in [Configuration](configuration.md). Good first values to adjust:

- `WallRunSpeed`, how fast you travel along the wall
- `WallRunDurationLimit`, seconds before you drop, 0 means unlimited
- `WallJumpAngle`, `WallJumpHorizontalVelocity`, `WallJumpVerticalVelocity`, the launch off the wall
- `CornerAngleLimit`, how sharp a wall corner you will follow

## Mark which walls can be run on

By default every actor with blocking collision on the trace channel can be run on, except actors tagged `NoWallRun`.

- To block specific walls, add the actor tag `NoWallRun`.
- To flip it so only chosen walls work, set `bRequiresTag` to true and add the actor tag `WallRun` (the name comes from `RequiredTag`) to each runnable wall.

The walls must block the trace channel set in `TraceChannel`, which defaults to Visibility.

## Listen to events

The component broadcasts these Blueprint-assignable delegates:

| Event | When it fires |
| :--- | :--- |
| `OnWallRunStarted(Side)` | A wall run begins |
| `OnWallSideChanged(NewSide)` | The wall switches sides, for example around a corner |
| `OnWallRunEnded` | The wall run stops for any reason |
| `OnWallJump(LaunchVelocity)` | A wall jump launches |

Handy for sound, camera effects and particles. Query state any time with `IsWallRunning`, `IsWallJumping`, `GetWallSide`, `GetArcAngle` and `GetTurnRate`.

## Hook up animation

The system does not play montages. Your Animation Blueprint reads the wall run state instead. Follow [Animation Setup Guide](animation-setup-guide.md), which covers reparenting to `UWallRunAnimInstance`, the exposed variables and the state machine.

## Check it works

1. Place a tall, flat wall with collision in your level.
2. Play, run at the wall, jump while moving toward it and pressing forward.
3. You should latch on, run along the wall, and drop after `WallRunDurationLimit`.
4. Press jump while wall running to launch away.

Wall running only starts while the character is falling, and needs either movement input or horizontal speed above `MinStartSpeed`.

## Troubleshooting

| Symptom | Likely cause |
| :--- | :--- |
| Nothing happens near walls | The wall does not block the `TraceChannel`, or has the `NoWallRun` tag, or `bRequiresTag` is on without the `WallRun` tag |
| Latches but you cannot steer | Expected, movement input is ignored while wall running |
| Jump does nothing while wall running | Jump is not routed to `StartWallJump`, see [Route jump to wall jump](#route-jump-to-wall-jump) |
| Feet stretch toward the floor | Foot IK is still active, see the Foot IK section of [Animation Setup Guide](animation-setup-guide.md) |
| Compile error about `WALLRUN_API` | Rename the macro, see [Setup Guide](setup-guide.md#rename-the-api-macro) |
