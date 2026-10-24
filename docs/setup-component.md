# Component Setup

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

Wall running through `UWallRunComponent`, a normal actor component on your character. Follow the steps in order, start to finish. You do not need to read the GAS guide.

Want the Ability System version instead? Use the [GAS Setup](setup-gas.md).

## On this page

- [Step 1: Requirements](#step-1-requirements)
- [Step 2: Install the plugin](#step-2-install-the-plugin)
- [Step 3: Update Build.cs](#step-3-update-buildcs)
- [Step 4: Build](#step-4-build)
- [Step 5: Set up your character](#step-5-set-up-your-character)
  - [Path A: use the included character](#path-a-use-the-included-character)
  - [Path B: use your own character](#path-b-use-your-own-character)
- [Step 6: Mark which walls can be run on](#step-6-mark-which-walls-can-be-run-on)
- [Step 7: Tune the settings](#step-7-tune-the-settings)
- [Step 8: Add animation](#step-8-add-animation)
- [Step 9: Check it works](#step-9-check-it-works)
- [Listen to events](#listen-to-events)
- [Troubleshooting](#troubleshooting)

## Step 1: Requirements

- Unreal Engine 5.8 and a C++ project
- The Wall Run System plugin, from Fab or this repository's `Plugins/WallRunSystem` folder
- Enhanced Input set up in your project
- A character that derives from `ACharacter`

## Step 2: Install the plugin

1. Close the editor.
2. Copy the `WallRunSystem` folder into your project's `Plugins/` folder, so you end up with `Plugins/WallRunSystem/WallRunSystem.uplugin`. Create `Plugins/` if it does not exist.
3. Open the project, go to Edit, Plugins, search for Wall Run System and make sure it is enabled. Restart if asked.

Installing to the engine works too. Fab's Install to Engine puts it under `Engine/Plugins/Marketplace/`, and you enable it the same way.

Enabling the plugin also enables `GameplayAbilities` and `EnhancedInput`, which it depends on. You do not need to edit your `.uproject` by hand.

## Step 3: Update Build.cs

Add `WallRunSystem` to your game module's `Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
	"EnhancedInput",
	"GameplayAbilities",
	"GameplayTags",
	"WallRunSystem"
});
```

Why each line is there:

- `WallRunSystem`: lets your code include `WallRunComponent.h`, `UGA_WallRun.h`, `WallRunGameplayTags.h` and the rest.
- `EnhancedInput`: your character binds its own input actions.
- `GameplayAbilities`, `GameplayTags`: only needed if your code reads the wall run gameplay tags. Leave them out otherwise.

## Step 4: Build

Close the editor, regenerate project files, and build the editor target. The gameplay tags are native, so there is no tag config to edit.

## Step 5: Set up your character

Pick one path.

### Path A: use the included character

The included characters ship in the plugin's `WallRunSample` module, with ready made Blueprints in the plugin content under `WallRunSystem Content/ThirdPerson/Blueprints`. Turn on Show Plugin Content in the Content Browser settings to see them. Subclass or duplicate those Blueprints, or follow the steps below to make your own.

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

The plugin has these under `WallRunSystem Content/Input/Actions`, with the mapping context `WallRunSystem Content/Input/IMC_Default`.

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

## Step 6: Mark which walls can be run on

By default every actor that blocks the trace channel can be run on, except actors tagged `NoWallRun`.

- To block a wall, add the actor tag `NoWallRun`.
- To flip it so only chosen walls work, turn on `bRequiresTag` and add the actor tag `WallRun` to each runnable wall. Both tag names are configurable.

Walls must block the channel set in `TraceChannel`, Visibility by default.

## Step 7: Tune the settings

Select the Wall Run Component, open the Wall Run category in Details, and expand Settings. Every property is listed in [Configuration](configuration.md). Good first values to adjust:

- `WallRunSpeed`, how fast you travel along the wall
- `WallRunDurationLimit`, seconds before you drop, 0 means unlimited
- `WallJumpAngle`, `WallJumpHorizontalVelocity`, `WallJumpVerticalVelocity`, the launch off the wall
- `CornerAngleLimit`, the largest angle between two walls the run will follow

## Step 8: Add animation

The system does not play montages. Your Animation Blueprint reads the wall run state. Follow the [Animation Setup Guide](animation-setup-guide.md), then come back here.

## Step 9: Check it works

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
| "cannot open include file" | `WallRunSystem` is missing from your `Build.cs`, see [Step 3](#step-3-update-buildcs) |
| Linker errors for wall run classes | Same cause, see [Step 3](#step-3-update-buildcs) |
| Plugin missing from the Plugins window | The folder is nested too deep, the `.uplugin` must sit at `Plugins/WallRunSystem/WallRunSystem.uplugin` |
| Nothing happens near walls | The wall does not block the `TraceChannel`, has the `NoWallRun` tag, or `bRequiresTag` is on without the `WallRun` tag |
| Latches but you cannot steer | Expected, movement input is ignored while wall running |
| Jump does nothing while wall running | Jump is not routed to `StartWallJump`, see [Path B](#path-b-use-your-own-character) |
| Feet stretch toward the floor | Foot IK is still active, see the Foot IK section of the [Animation Setup Guide](animation-setup-guide.md#disable-foot-ik-while-wall-running) |
