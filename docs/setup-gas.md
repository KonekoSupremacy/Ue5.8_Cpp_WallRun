# GAS Setup

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

Wall running through `UGA_WallRun`, a Gameplay Ability granted to your character's Ability System Component. Use this when your project already runs on GAS and you want wall run state as gameplay tags. Follow the steps in order, start to finish. You do not need to read the Component guide.

Want the simpler setup without an Ability System Component? Use the [Component Setup](setup-component.md).

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
- [Gameplay tags](#gameplay-tags)
- [Block wall running with tags](#block-wall-running-with-tags)
- [Troubleshooting](#troubleshooting)

## Step 1: Requirements

- Unreal Engine 5.8 and a C++ project
- Enhanced Input set up in your project
- The Gameplay Ability System in use, or at least enabled
- A character with an Ability System Component

## Step 2: Copy the source files

Copy these into your module's `Source/<YourModule>/` folder and keep the folder layout:

| Path | Why |
| :--- | :--- |
| `WallRunSystem/` (the whole folder) | The ability, shared logic, settings, and the animation instance |
| `WallRunGameplayTags.h` and `.cpp` | The native gameplay tags |
| `TickableAbility.h` | The interface the character uses to tick the ability |
| `PlayerCharacter.h` and `.cpp` | Required. The ability reads the jump input action from it, and it is the base class of the included character |
| `GASPlayerCharacter.h` and `.cpp` | Path A only, the included character with an Ability System Component |
| `WallRunPlayerController.h` and `.cpp` | Path A only, optional, adds your input mapping context |

Do **not** copy `WallRun.cpp`. It contains `IMPLEMENT_PRIMARY_GAME_MODULE`, which your project already has. Step 5 covers the one thing you need from it.

Keep the `WallRunSystem` folder whole. It also contains `UWallRunComponent`, which stays unused in this setup but still has to compile.

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

- `GameplayAbilities`, `GameplayTags`, `GameplayTasks`: the ability, the Ability System Component calls, and the native tags.
- `EnhancedInput`: the ability binds the wall jump to your jump input action.
- `PublicIncludePaths`: files inside `WallRunSystem/` include `PlayerCharacter.h`, `WallRunGameplayTags.h` and `TickableAbility.h` from the module root by plain name. The module root is not on the include path by default, so you add it once. Without it you get "cannot open include file".

## Step 4: Update your .uproject

Enable the Ability System plugin:

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

`AGASPlayerCharacter` owns an Ability System Component, grants its `DefaultAbilities` when possessed, activates them once input is set up, and ticks every active ability that implements `ITickableAbility`.

**1. Create the ability Blueprint**

1. Content Browser, right click, Blueprint Class, and pick `GA_WallRun` (the C++ class `UGA_WallRun`) as the parent.
2. Name it, for example `BP_GA_WallRun`.
3. Open it. Its Settings are in Class Defaults, covered in [Step 9](#step-9-tune-the-settings).

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

The ability binds to the same Jump Action to trigger the wall jump, so it must be assigned. The sample project has these under `Content/Input/Actions`, with the mapping context `Content/Input/IMC_Default`.

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
- It derives from `APlayerCharacter`. `UGA_WallRun` reads `GetJumpAction()` from it to bind the wall jump input. If your character is not an `APlayerCharacter`, call `StartWallJump()` on the ability yourself from your jump input.

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

## Step 8: Mark which walls can be run on

By default every actor that blocks the trace channel can be run on, except actors tagged `NoWallRun`.

- To block a wall, add the actor tag `NoWallRun`.
- To flip it so only chosen walls work, turn on `bRequiresTag` and add the actor tag `WallRun` to each runnable wall. Both tag names are configurable.

Walls must block the channel set in `TraceChannel`, Visibility by default.

## Step 9: Tune the settings

Open your ability Blueprint, Class Defaults, Wall Run category, Settings. Every property is listed in [Configuration](configuration.md). Settings are Edit Defaults Only on the ability, so change them on the Blueprint class, not at runtime on an instance. Good first values to adjust:

- `WallRunSpeed`, how fast you travel along the wall
- `WallRunDurationLimit`, seconds before you drop, 0 means unlimited
- `WallJumpAngle`, `WallJumpHorizontalVelocity`, `WallJumpVerticalVelocity`, the launch off the wall
- `CornerAngleLimit`, the largest angle between two walls the run will follow

## Step 10: Add animation

`UWallRunAnimInstance` first looks for a `UWallRunComponent`, and if there is none it finds the Ability System Component and reads the state from the granted `UGA_WallRun`. Nothing extra to wire for GAS. Follow the [Animation Setup Guide](animation-setup-guide.md), then come back here.

## Step 11: Check it works

1. Place a tall, flat wall with collision in your level.
2. Play, jump toward the wall while moving forward.
3. Use the console command `showdebug abilitysystem` to see `Status.WallRunning.Left` or `.Right` appear while latched.
4. Press jump to launch away. `Status.WallJumping` appears briefly.

## Gameplay tags

All tags are native, registered in C++ from `WallRunGameplayTags.cpp`. No ini edit is needed.

| Tag | Meaning |
| :--- | :--- |
| `Status.WallRunning` | Parent tag, matches while wall running on either side |
| `Status.WallRunning.Left` | Wall run active, wall on the left, set as a loose tag |
| `Status.WallRunning.Right` | Wall run active, wall on the right, set as a loose tag |
| `Status.WallJumping` | Set during the wall jump launch |
| `Status.BlockingAction.WallRun` | If present on the character, new wall runs are refused |
| `Event.Movement.WallRun` | Declared for your own event usage |
| `Event.Movement.WallJump` | Declared for your own event usage |

C++ access is through the `WallRunGameplayTags` namespace, for example `WallRunGameplayTags::Status_WallRunning`.

## Block wall running with tags

`UGA_WallRun` carries a `TagsBlockingWallRun` container, which defaults to `Status.BlockingAction.WallRun`. A wall run cannot start while the character has any tag in that container.

- Apply `Status.BlockingAction.WallRun` from a stun, ledge grab or cutscene effect to disable wall running temporarily.
- Add your own tags to `TagsBlockingWallRun` in the ability Blueprint's Class Defaults for more cases.

## Troubleshooting

| Symptom | Likely cause |
| :--- | :--- |
| "cannot open include file" | The module root is missing from `PublicIncludePaths`, see [Step 3](#step-3-update-buildcs) |
| Duplicate module or linker error | `WallRun.cpp` was copied, see [Step 2](#step-2-copy-the-source-files) |
| Error about `WALLRUN_API` | The macro was not renamed, see [Step 5](#step-5-rename-the-api-macro-and-add-the-log-category) |
| Never starts | The ability was never granted or activated, check Default Abilities, or the plugin is not enabled |
| Starts then freezes | The ability is not being ticked, see [Path B](#path-b-use-your-own-character) |
| Wall jump does nothing | Jump Action not assigned, or the character is not an `APlayerCharacter` and does not call `StartWallJump()` |
| Cannot start while stunned | `Status.BlockingAction.WallRun` is on the character, which is intended |
| Animation never changes | Anim Class is not parented to `WallRunAnimInstance`, see the [Animation Setup Guide](animation-setup-guide.md) |
