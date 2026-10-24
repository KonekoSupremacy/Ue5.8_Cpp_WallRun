# Setup Guide

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

There are two complete setups. Pick one and follow it from the first step to the last. Each one is self-contained, so you never need to read the other.

## On this page

- [Pick your setup](#pick-your-setup)
- [What the two setups share](#what-the-two-setups-share)
- [After setup](#after-setup)

## Pick your setup

| | [Component Setup](setup-component.md) | [GAS Setup](setup-gas.md) |
| :--- | :--- | :--- |
| Core class | `UWallRunComponent` | `UGA_WallRun` |
| Your character needs an Ability System Component | No | Yes |
| Wall run state as gameplay tags | Only if the character has one | Yes |
| Block wall running from other systems | Your own checks | Tag `Status.BlockingAction.WallRun` |
| Best for | Most projects, quickest to set up | Projects already built on GAS |

Not sure? Start with the Component Setup.

### Component Setup

[Open the full Component Setup](setup-component.md), or jump to a step:

[Requirements](setup-component.md#step-1-requirements), [install the plugin](setup-component.md#step-2-install-the-plugin), [Build.cs](setup-component.md#step-3-update-buildcs), [build](setup-component.md#step-4-build), [character](setup-component.md#step-5-set-up-your-character), [walls](setup-component.md#step-6-mark-which-walls-can-be-run-on), [settings](setup-component.md#step-7-tune-the-settings), [animation](setup-component.md#step-8-add-animation), [check it works](setup-component.md#step-9-check-it-works), [troubleshooting](setup-component.md#troubleshooting)

### GAS Setup

[Open the full GAS Setup](setup-gas.md), or jump to a step:

[Requirements](setup-gas.md#step-1-requirements), [install the plugin](setup-gas.md#step-2-install-the-plugin), [Build.cs](setup-gas.md#step-3-update-buildcs), [build](setup-gas.md#step-4-build), [character](setup-gas.md#step-5-set-up-your-character), [walls](setup-gas.md#step-6-mark-which-walls-can-be-run-on), [settings](setup-gas.md#step-7-tune-the-settings), [animation](setup-gas.md#step-8-add-animation), [check it works](setup-gas.md#step-9-check-it-works), [gameplay tags](setup-gas.md#gameplay-tags), [troubleshooting](setup-gas.md#troubleshooting)

## What the two setups share

Both setups use the same plugin, `WallRunSystem`. It holds the component, the ability, the shared logic, the native gameplay tags and the animation instance. Install it once and use the class you picked.

- The plugin depends on `GameplayAbilities` and `EnhancedInput` and enables them for you. Your character only needs an Ability System Component in the GAS Setup.
- Your game module adds `WallRunSystem` to its `Build.cs` in both setups. That is the only code change needed to include the headers.
- The example characters, `APlayerCharacter`, `AWallRunPlayerCharacter` and `AGASPlayerCharacter`, live in a second module, `WallRunSample`. Your own code does not need it, add it to `Build.cs` only if you subclass those characters.

## Try the demo

The plugin ships a playable demo so you can see a working setup before building your own:

1. In the Content Browser, open the settings menu and turn on Show Plugin Content.
2. Open `WallRunSystem Content/ThirdPerson/Lvl_ThirdPerson`.
3. Press Play. The map already uses `BP_ThirdPersonGameMode` as its Game Mode Override.

| Asset | What it shows |
| :--- | :--- |
| `ThirdPerson/Blueprints/BP_ThirdPersonCharacter` | Component Setup, Path A |
| `ThirdPerson/Blueprints/BP_ThirdPersonCharacter_GAS` | GAS Setup, Path A, the demo game mode uses this one |
| `ThirdPerson/Blueprints/BP_GA_WallRun` | The ability Blueprint with tuned settings |
| `Characters/Mannequins/Anims/Unarmed/ABP_Unarmed` | An Animation Blueprint reparented to `WallRunAnimInstance` |
| `Input/` | The input actions and mapping context |

Open these next to the guide to see each step filled in.

## After setup

Continue with the [Animation Setup Guide](animation-setup-guide.md). Questions and fixes are in the [FAQ](faq.md), and every setting is listed in [Configuration](configuration.md).
