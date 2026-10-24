# GAS Setup

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

Wall running through `UGA_WallRun`, a Gameplay Ability granted to your character's Ability System Component. Use this when your project already runs on GAS and you want wall run state as gameplay tags. Follow the steps in order, start to finish. You do not need to read the Component guide.

Want the simpler setup without an Ability System Component? Use the [Component Setup](setup-component.md).

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
- [Gameplay tags](#gameplay-tags)
- [Block wall running with tags](#block-wall-running-with-tags)
- [Troubleshooting](#troubleshooting)

## Step 1: Requirements

- Unreal Engine 5.8 and a C++ project
- The Wall Run System plugin, from Fab or this repository's `Plugins/WallRunSystem` folder
- Enhanced Input set up in your project
- The Gameplay Ability System in use, or at least enabled
- A character with an Ability System Component

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
- `GameplayAbilities`, `GameplayTags`: your character owns the Ability System Component and checks the wall run tags.
- `EnhancedInput`: the ability binds the wall jump to your jump input action.

## Step 4: Build

Close the editor, regenerate project files, and build the editor target. The gameplay tags are native, so there is no tag config to edit.

## Step 5: Set up your character

Pick one path.

### Path A: use the included character

The included characters ship in the plugin's `WallRunSample` module, with ready made Blueprints in the plugin content under `WallRunSystem Content/ThirdPerson/Blueprints`. Turn on Show Plugin Content in the Content Browser settings to see them. Subclass or duplicate those Blueprints, or follow the steps below to make your own.

`AGASPlayerCharacter` owns an Ability System Component, grants its `DefaultAbilities` when possessed, activates them once input is set up, and ticks every active ability that implements `ITickableAbility`.

**1. Create the ability Blueprint**

1. Content Browser, right click, Blueprint Class, and pick `GA_WallRun` (the C++ class `UGA_WallRun`) as the parent.
2. Name it, for example `BP_GA_WallRun`.
3. Open it. Its Settings are in Class Defaults, covered in [Step 7](#step-7-tune-the-settings).

**2. Create the character Blueprint**

1. Create a Blueprint from `GASPlayerCharacter`, for example `BP_GAS_Character`.
2. Select the Mesh component, assign your skeletal mesh, and place it at the bottom of the capsule facing +X.
3. In Class Defaults, under GAS, add your ability Blueprint to Default Abilities.

**3. Assign the input actions**

In Class Defaults, under Input:

| Property | Asset |
| :--- | :--- |
| Jump Action | An Input Action, Digital (bool), for example `IA_Jump` |
| Move Action | An Input Action, Axis2D, for example `IA_Move` |
| Look Action | An Input Action, Axis2D, for example `IA_MouseLook` |

The ability reads this Jump Action through `IWallRunInputProvider` to trigger the wall jump, so it must be assigned. The plugin has these under `WallRunSystem Content/Input/Actions`, with the mapping context `WallRunSystem Content/Input/IMC_Default`.

**4. Create the controller**

1. Create a Blueprint from `WallRunPlayerController`.
2. In Class Defaults, add your mapping context (for example `IMC_Default`) to Default Mapping Contexts.

**5. Create the game mode**

1. Create or open a Game Mode Blueprint.
2. Set Default Pawn Class to your new character and Player Controller Class to your new controller.
3. Set the level's Game Mode Override, or the project default, to that Game Mode.

### Path B: use your own character

**1. Check your character**

- It has a `UAbilitySystemComponent`, and `InitAbilityActorInfo(this, this)` runs when it is possessed.
- The ability needs your jump Input Action to bind the wall jump. Give it one of two ways:
  - Set `JumpInputAction` in the ability Blueprint's Class Defaults, under Wall Run, Input.
  - Or implement `IWallRunInputProvider` on your character and return your jump action. Used when `JumpInputAction` is empty.

  ```cpp
  #include "WallRunInputProvider.h"

  class AMyCharacter : public ACharacter, public IWallRunInputProvider {
  	// ...
  	virtual UInputAction* GetWallRunJumpAction() const override { return JumpAction; }
  };
  ```

  Neither fits? Call `StartWallJump()` on the ability yourself from your jump input.

**2. Tick the ability**

Gameplay Abilities do not tick on their own. `UGA_WallRun` implements `ITickableAbility`, and your character must drive it every frame:

```cpp
void AMyCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities()) {
		if (!Spec.IsActive()) {
			continue;
		}

		if (ITickableAbility* Tickable = Cast<ITickableAbility>(Spec.GetPrimaryInstance())) {
			Tickable->TickAbility(DeltaSeconds);
		}
	}
}
```

Without this the character latches on and then does nothing.

**3. Grant and activate the ability**

Activate after the input component exists, because activation is where the jump binding happens:

```cpp
void AMyCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_WallRun::StaticClass()));
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	AbilitySystemComponent->TryActivateAbilityByClass(UGA_WallRun::StaticClass());
}
```

The ability stays active for the whole life of the character. It listens for movement mode changes and hits, and starts and stops the actual wall run itself.

**4. Handle jump and movement**

Ignore normal jump and movement input while the wall run tags are present:

```cpp
bool AMyCharacter::CanMove() const {
	return !AbilitySystemComponent->HasMatchingGameplayTag(WallRunGameplayTags::Status_WallRunning);
}

void AMyCharacter::DoJump() {
	if (AbilitySystemComponent->HasMatchingGameplayTag(WallRunGameplayTags::Status_WallRunning) ||
		AbilitySystemComponent->HasMatchingGameplayTag(WallRunGameplayTags::Status_WallJumping)) {
		return;
	}

	Super::DoJump();
}
```

The wall jump itself is fired by the ability's own jump binding.

## Step 6: Mark which walls can be run on

By default every actor that blocks the trace channel can be run on, except actors tagged `NoWallRun`.

- To block a wall, add the actor tag `NoWallRun`.
- To flip it so only chosen walls work, turn on `bRequiresTag` and add the actor tag `WallRun` to each runnable wall. Both tag names are configurable.

Walls must block the channel set in `TraceChannel`, Visibility by default.

## Step 7: Tune the settings

Open your ability Blueprint, Class Defaults, Wall Run category, Settings. Every property is listed in [Configuration](configuration.md). Settings are Edit Defaults Only on the ability, so change them on the Blueprint class, not at runtime on an instance. Good first values to adjust:

- `WallRunSpeed`, how fast you travel along the wall
- `WallRunDurationLimit`, seconds before you drop, 0 means unlimited
- `WallJumpAngle`, `WallJumpHorizontalVelocity`, `WallJumpVerticalVelocity`, the launch off the wall
- `CornerAngleLimit`, the largest angle between two walls the run will follow

## Step 8: Add animation

`UWallRunAnimInstance` first looks for a `UWallRunComponent`, and if there is none it finds the Ability System Component and reads the state from the granted `UGA_WallRun`. Nothing extra to wire for GAS. Follow the [Animation Setup Guide](animation-setup-guide.md), then come back here.

## Step 9: Check it works

1. Place a tall, flat wall with collision in your level.
2. Play, jump toward the wall while moving forward.
3. Use the console command `showdebug abilitysystem` to see `Status.WallRunning.Left` or `.Right` appear while latched.
4. Press jump to launch away. `Status.WallJumping` appears briefly.

## Gameplay tags

All tags are native, registered in C++ by the plugin. No ini edit is needed.

| Tag | Meaning |
| :--- | :--- |
| `Status.WallRunning` | Parent tag, matches while wall running on either side |
| `Status.WallRunning.Left` | Wall run active, wall on the left, set as a loose tag |
| `Status.WallRunning.Right` | Wall run active, wall on the right, set as a loose tag |
| `Status.WallJumping` | Set during the wall jump launch |
| `Status.BlockingAction.WallRun` | If present on the character, new wall runs are refused |
| `Event.Movement.WallRun` | Declared for your own event usage |
| `Event.Movement.WallJump` | Declared for your own event usage |

C++ access is through the `WallRunGameplayTags` namespace in `WallRunGameplayTags.h`, for example `WallRunGameplayTags::Status_WallRunning`.

## Block wall running with tags

`UGA_WallRun` carries a `TagsBlockingWallRun` container, which defaults to `Status.BlockingAction.WallRun`. A wall run cannot start while the character has any tag in that container.

- Apply `Status.BlockingAction.WallRun` from a stun, ledge grab or cutscene effect to disable wall running temporarily.
- Add your own tags to `TagsBlockingWallRun` in the ability Blueprint's Class Defaults for more cases.

## Troubleshooting

| Symptom | Likely cause |
| :--- | :--- |
| "cannot open include file" | `WallRunSystem` is missing from your `Build.cs`, see [Step 3](#step-3-update-buildcs) |
| Linker errors for wall run classes | Same cause, see [Step 3](#step-3-update-buildcs) |
| Plugin missing from the Plugins window | The folder is nested too deep, the `.uplugin` must sit at `Plugins/WallRunSystem/WallRunSystem.uplugin` |
| Never starts | The ability was never granted or activated, check Default Abilities, or the plugin is not enabled |
| Starts then freezes | The ability is not being ticked, see [Path B](#path-b-use-your-own-character) |
| Wall jump does nothing | No jump action reaches the ability, set `JumpInputAction` or implement `IWallRunInputProvider`, see [Path B](#path-b-use-your-own-character) |
| Cannot start while stunned | `Status.BlockingAction.WallRun` is on the character, which is intended |
| Animation never changes | Anim Class is not parented to `WallRunAnimInstance`, see the [Animation Setup Guide](animation-setup-guide.md) |
