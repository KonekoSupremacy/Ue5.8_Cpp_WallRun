# Component Setup

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

Wall running through `UWallRunComponent`, a normal actor component on your character. Follow the steps in order, start to finish. You do not need to read the GAS guide.

Want the Ability System version instead? Use the [GAS Setup](setup-gas.md).

## On this page

- [Step 1: Requirements](#step-1-requirements)
- [Step 2: Copy the source files](#step-2-copy-the-source-files)
- [Step 3: Update Build.cs](#step-3-update-buildcs)
- [Step 4: Update your .uproject](#step-4-update-your-uproject)
- [Step 5: Rename the API macro and add the log category](#step-5-rename-the-api-macro-and-add-the-log-category)
- [Step 6: Build](#step-6-build)
- [Step 7: Set up your character](#step-7-set-up-your-character)
  - [Path A: use the included character](#path-a-use-the-included-character)
  - [Path B: use your own character](#path-b-use-your-own-character)
- [Step 8: Mark which walls can be run on](#step-8-mark-which-walls-can-be-run-on)
- [Step 9: Tune the settings](#step-9-tune-the-settings)
- [Step 10: Add animation](#step-10-add-animation)
- [Step 11: Check it works](#step-11-check-it-works)
- [Listen to events](#listen-to-events)
- [Troubleshooting](#troubleshooting)

## Step 1: Requirements

- Unreal Engine 5.8 and a C++ project
- Enhanced Input set up in your project
- A character that derives from `ACharacter`

## Step 2: Copy the source files

Copy these into your module's `Source/<YourModule>/` folder and keep the folder layout:

| Path | Why |
| :--- | :--- |
| `WallRunSystem/` (the whole folder) | The component, shared logic, settings, and the animation instance |
| `WallRunGameplayTags.h` and `.cpp` | Native gameplay tags used by the shared logic |
| `TickableAbility.h` | Needed because the `WallRunSystem` folder also holds the GAS ability, which compiles but stays unused here |
| `PlayerCharacter.h` and `.cpp` | Needed by the same ability file, and the base class of the included character |
| `WallRunPlayerCharacter.h` and `.cpp` | Path A only, the included character with the component |
| `WallRunPlayerController.h` and `.cpp` | Path A only, optional, adds your input mapping context |

Do **not** copy `WallRun.cpp`. It contains `IMPLEMENT_PRIMARY_GAME_MODULE`, which your project already has. Step 5 covers the one thing you need from it.

Keep the `WallRunSystem` folder whole. It contains the GAS ability, which stays unused in this setup but still has to compile.

## Step 3: Update Build.cs

Add this to your module's `Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
	"EnhancedInput",
	"GameplayAbilities",
	"GameplayTags",
	"GameplayTasks"
});

PublicIncludePaths.Add("YourModule");
```

Replace `YourModule` with your module's folder name under `Source/`. The path is relative to your project's `Source` folder.

Why each line is there:

- `GameplayAbilities`, `GameplayTags`, `GameplayTasks`: the shared logic reads and sets gameplay tags, and the ability file is part of the folder. Your game does not need to use the Ability System, it just has to compile.
- `EnhancedInput`: `PlayerCharacter` and the ability file bind Enhanced Input actions.
- `PublicIncludePaths`: files inside `WallRunSystem/` include `PlayerCharacter.h`, `WallRunGameplayTags.h` and `TickableAbility.h` from the module root by plain name. The module root is not on the include path by default, so you add it once. Without it you get "cannot open include file".

## Step 4: Update your .uproject

Enable the plugin the shared code compiles against:

```json
"Plugins": [
	{
		"Name": "GameplayAbilities",
		"Enabled": true
	}
]
```

Do not leave a trailing comma after the last plugin entry.

## Step 5: Rename the API macro and add the log category

1. Replace `WALLRUN_API` with your module's macro, for example `MYGAME_API`, in every copied header. A project-wide find and replace does it.
2. `PlayerCharacter.cpp` includes `WallRun.h` for the `LogWallRun` log category. Create a small `WallRun.h` with just this:

```cpp
#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWallRun, Log, All);
```

3. Add this line to any one `.cpp` file in your module, for example your module's main `.cpp`:

```cpp
DEFINE_LOG_CATEGORY(LogWallRun)
```

## Step 6: Build

Close the editor, regenerate project files, and build the editor target. The gameplay tags are native, so there is no tag config to edit.

## Step 7: Set up your character

Pick one path.

### Path A: use the included character

`AWallRunPlayerCharacter` already has a `UWallRunComponent`, routes jump to wall jump, and ignores movement input while wall running.

**1. Create the character Blueprint**

1. Content Browser, right click, Blueprint Class.
2. Search for `WallRunPlayerCharacter` and pick it as the parent.
3. Name it, for example `BP_WallRunCharacter`.
4. Open it, select the Mesh component, assign your skeletal mesh, and place it at the bottom of the capsule facing +X.

**2. Assign the input actions**

In Class Defaults, under Input:

| Property | Asset |
| :--- | :--- |
| Jump Action | An Input Action, Digital (bool), for example `IA_Jump` |
| Move Action | An Input Action, Axis2D, for example `IA_Move` |
| Look Action | An Input Action, Axis2D, for example `IA_MouseLook` |

The sample project has these under `Content/Input/Actions`, with the mapping context `Content/Input/IMC_Default`.

**3. Create the controller**

1. Create a Blueprint from `WallRunPlayerController`.
2. In Class Defaults, add your mapping context (for example `IMC_Default`) to Default Mapping Contexts.

**4. Create the game mode**

1. Create or open a Game Mode Blueprint.
2. Set Default Pawn Class to your new character and Player Controller Class to your new controller.
3. Set the level's Game Mode Override, or the project default, to that Game Mode.

### Path B: use your own character

Your character must derive from `ACharacter`. The component finds its owner when the game starts and hooks the character's movement mode and hit events itself.

**1. Add the component**

In C++:

```cpp
// MyCharacter.h
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<UWallRunComponent> WallRunComponent;

// MyCharacter.cpp, in the constructor
WallRunComponent = CreateDefaultSubobject<UWallRunComponent>("WallRunComponent");
```

Or in a Blueprint: Components panel, Add, search for Wall Run Component.

**2. Route jump to wall jump**

```cpp
void AMyCharacter::DoJump() {
	if (WallRunComponent && WallRunComponent->IsWallRunning()) {
		WallRunComponent->StartWallJump();
		return;
	}

	Jump();
}
```

In Blueprints, branch on `Is Wall Running` from your jump input. Call `Start Wall Jump` on true and `Jump` on false.

**3. Ignore movement input while wall running**

The system drives velocity along the wall, so skip normal movement meanwhile:

```cpp
void AMyCharacter::Move(const FInputActionValue& Value) {
	if (WallRunComponent && WallRunComponent->IsWallRunning()) {
		return;
	}

	// normal movement here
}
```

Look input should stay enabled.

## Step 8: Mark which walls can be run on

By default every actor that blocks the trace channel can be run on, except actors tagged `NoWallRun`.

- To block a wall, add the actor tag `NoWallRun`.
- To flip it so only chosen walls work, turn on `bRequiresTag` and add the actor tag `WallRun` to each runnable wall. Both tag names are configurable.

Walls must block the channel set in `TraceChannel`, Visibility by default.

## Step 9: Tune the settings

Select the Wall Run Component, open the Wall Run category in Details, and expand Settings. Every property is listed in [Configuration](configuration.md). Good first values to adjust:

- `WallRunSpeed`, how fast you travel along the wall
- `WallRunDurationLimit`, seconds before you drop, 0 means unlimited
- `WallJumpAngle`, `WallJumpHorizontalVelocity`, `WallJumpVerticalVelocity`, the launch off the wall
- `CornerAngleLimit`, the largest angle between two walls the run will follow

## Step 10: Add animation

The system does not play montages. Your Animation Blueprint reads the wall run state. Follow the [Animation Setup Guide](animation-setup-guide.md), then come back here.

## Step 11: Check it works

1. Place a tall, flat wall with collision in your level.
2. Play, run at the wall, jump while moving toward it with forward input.
3. You should latch on, run along the wall, and drop after `WallRunDurationLimit`.
4. Press jump while wall running to launch away.

Wall running only starts while the character is falling, and needs either movement input or horizontal speed above `MinStartSpeed`.

## Listen to events

The component broadcasts these Blueprint-assignable delegates:

| Event | When it fires |
| :--- | :--- |
| `OnWallRunStarted(Side)` | A wall run begins |
| `OnWallSideChanged(NewSide)` | The wall switches sides, for example around a corner |
| `OnWallRunEnded` | The wall run stops for any reason |
| `OnWallJump(LaunchVelocity)` | A wall jump launches |

Handy for sound, camera effects and particles. Query state any time with `IsWallRunning`, `IsWallJumping`, `GetWallSide`, `GetArcAngle` and `GetTurnRate`.

## Troubleshooting

| Symptom | Likely cause |
| :--- | :--- |
| "cannot open include file" | The module root is missing from `PublicIncludePaths`, see [Step 3](#step-3-update-buildcs) |
| Duplicate module or linker error | `WallRun.cpp` was copied, see [Step 2](#step-2-copy-the-source-files) |
| Error about `WALLRUN_API` | The macro was not renamed, see [Step 5](#step-5-rename-the-api-macro-and-add-the-log-category) |
| Nothing happens near walls | The wall does not block the `TraceChannel`, has the `NoWallRun` tag, or `bRequiresTag` is on without the `WallRun` tag |
| Latches but you cannot steer | Expected, movement input is ignored while wall running |
| Jump does nothing while wall running | Jump is not routed to `StartWallJump`, see [Path B](#path-b-use-your-own-character) |
| Feet stretch toward the floor | Foot IK is still active, see the Foot IK section of the [Animation Setup Guide](animation-setup-guide.md#disable-foot-ik-while-wall-running) |
