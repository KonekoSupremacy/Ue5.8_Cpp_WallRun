# GAS setup

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

Wall running through `UGA_WallRun`, a Gameplay Ability granted to your character's Ability System Component. Use this when your project is already built around GAS and you want wall run state expressed as gameplay tags.

## On this page

- [Pick a path](#pick-a-path)
- [Path A: use the included character](#path-a-use-the-included-character)
  - [Create the ability Blueprint](#create-the-ability-blueprint)
  - [Create the character Blueprint](#create-the-character-blueprint)
  - [Assign the input actions](#assign-the-input-actions)
  - [Set up the controller and game mode](#set-up-the-controller-and-game-mode)
- [Path B: add it to your own character](#path-b-add-it-to-your-own-character)
  - [Requirements for your character](#requirements-for-your-character)
  - [Tick the ability](#tick-the-ability)
  - [Grant and activate the ability](#grant-and-activate-the-ability)
  - [Handle jump and movement](#handle-jump-and-movement)
- [Gameplay tags](#gameplay-tags)
- [Block wall running with tags](#block-wall-running-with-tags)
- [Tune the settings](#tune-the-settings)
- [Mark which walls can be run on](#mark-which-walls-can-be-run-on)
- [Hook up animation](#hook-up-animation)
- [Check it works](#check-it-works)
- [Troubleshooting](#troubleshooting)

## Pick a path

| Path | Use it when |
| :--- | :--- |
| [A: included character](#path-a-use-the-included-character) | You are starting fresh, or happy to reparent your character Blueprint |
| [B: your own character](#path-b-add-it-to-your-own-character) | You already have a GAS character you do not want to change |

Finish [Setup Guide](setup-guide.md) before either path.

## Path A: use the included character

`AGASPlayerCharacter` owns an Ability System Component, grants its `DefaultAbilities` when possessed, activates them once input is set up, and ticks every active ability that implements `ITickableAbility`.

### Create the ability Blueprint

1. Content Browser, right click, Blueprint Class, pick `GA_WallRun` (the C++ class `UGA_WallRun`) as the parent.
2. Name it, for example `BP_GA_WallRun`.
3. Open it and adjust Settings in Class Defaults. See [Tune the settings](#tune-the-settings).

### Create the character Blueprint

1. Create a Blueprint from `GASPlayerCharacter`, for example `BP_GAS_Character`.
2. Assign the skeletal mesh and Anim Class, see [Hook up animation](#hook-up-animation).
3. In Class Defaults, under GAS, add your ability Blueprint to Default Abilities.

### Assign the input actions

Same as the component setup. In Class Defaults under Input:

| Property | Asset |
| :--- | :--- |
| Jump Action | Input Action, Digital (bool), for example `IA_Jump` |
| Move Action | Input Action, Axis2D, for example `IA_Move` |
| Look Action | Input Action, Axis2D, for example `IA_MouseLook` |

The ability binds to the same Jump Action to trigger the wall jump, so it must be assigned.

### Set up the controller and game mode

1. Create a Blueprint from `WallRunPlayerController` and add your mapping context to Default Mapping Contexts.
2. In your Game Mode, set Default Pawn Class to the new character and Player Controller Class to the new controller.
3. Make sure the level uses that Game Mode.

## Path B: add it to your own character

### Requirements for your character

- It has an `UAbilitySystemComponent`, and `InitAbilityActorInfo(this, this)` is called when possessed.
- It derives from `APlayerCharacter` (the included base class). `UGA_WallRun` reads `GetJumpAction()` from it to bind the wall jump input. If your character is not an `APlayerCharacter`, call `StartWallJump()` on the ability yourself from your jump input.
- The ability is activated after the input component exists, because activation is where the jump binding happens.

### Tick the ability

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

Without this, the character latches on and then does nothing.

### Grant and activate the ability

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

### Handle jump and movement

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

## Tune the settings

Open your ability Blueprint, Class Defaults, Wall Run category, Settings. All properties are in [Configuration](configuration.md). Settings are Edit Defaults Only on the ability, so change them on the Blueprint class, not at runtime on an instance.

## Mark which walls can be run on

Same rules as the component setup:

- Actors tagged `NoWallRun` are never runnable.
- With `bRequiresTag` on, only actors tagged `WallRun` (see `RequiredTag`) are runnable.
- Walls must block the `TraceChannel`, Visibility by default.

## Hook up animation

`UWallRunAnimInstance` first looks for a `UWallRunComponent`, and if there is none it finds the Ability System Component and reads the state from the granted `UGA_WallRun`. Nothing extra to wire for GAS. Follow [Animation Setup Guide](animation-setup-guide.md).

## Check it works

1. Place a tall, flat wall with collision in your level.
2. Play, jump toward the wall while moving forward.
3. Use the console command `showdebug abilitysystem` to see `Status.WallRunning.Left` or `.Right` appear while latched.
4. Press jump to launch away, `Status.WallJumping` appears briefly.

## Troubleshooting

| Symptom | Likely cause |
| :--- | :--- |
| Never starts | `GameplayAbilities` plugin or module missing, or the ability was never granted or activated, check Default Abilities |
| Starts then freezes | The ability is not being ticked, see [Tick the ability](#tick-the-ability) |
| Wall jump does nothing | Jump Action not assigned, or character is not an `APlayerCharacter` and does not call `StartWallJump()` |
| Cannot start while stunned | `Status.BlockingAction.WallRun` is on the character, which is intended |
| Animation never changes | Anim Class is not parented to `WallRunAnimInstance`, see [Animation Setup Guide](animation-setup-guide.md) |
