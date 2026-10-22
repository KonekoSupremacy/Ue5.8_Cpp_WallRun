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

[Requirements](setup-component.md#step-1-requirements), [copy files](setup-component.md#step-2-copy-the-source-files), [Build.cs](setup-component.md#step-3-update-buildcs), [.uproject](setup-component.md#step-4-update-your-uproject), [API macro and log](setup-component.md#step-5-rename-the-api-macro-and-add-the-log-category), [build](setup-component.md#step-6-build), [character](setup-component.md#step-7-set-up-your-character), [walls](setup-component.md#step-8-mark-which-walls-can-be-run-on), [settings](setup-component.md#step-9-tune-the-settings), [animation](setup-component.md#step-10-add-animation), [check it works](setup-component.md#step-11-check-it-works), [troubleshooting](setup-component.md#troubleshooting)

### GAS Setup

[Open the full GAS Setup](setup-gas.md), or jump to a step:

[Requirements](setup-gas.md#step-1-requirements), [copy files](setup-gas.md#step-2-copy-the-source-files), [Build.cs](setup-gas.md#step-3-update-buildcs), [.uproject](setup-gas.md#step-4-update-your-uproject), [API macro and log](setup-gas.md#step-5-rename-the-api-macro-and-add-the-log-category), [build](setup-gas.md#step-6-build), [character](setup-gas.md#step-7-set-up-your-character), [walls](setup-gas.md#step-8-mark-which-walls-can-be-run-on), [settings](setup-gas.md#step-9-tune-the-settings), [animation](setup-gas.md#step-10-add-animation), [check it works](setup-gas.md#step-11-check-it-works), [gameplay tags](setup-gas.md#gameplay-tags), [troubleshooting](setup-gas.md#troubleshooting)

## What the two setups share

The two setups differ in the character and activation steps, but the `Build.cs` and the copied core files are nearly the same. That is not an oversight:

- The shared logic in `WallRunSystem/` reads and sets gameplay tags, and the animation instance can read either setup. So both setups need the `GameplayAbilities`, `GameplayTags` and `GameplayTasks` modules, even if your game never uses the Ability System in the Component Setup.
- The `WallRunSystem` folder holds both the component and the ability. The one you do not use still compiles, so keep the folder whole.
- `PublicIncludePaths.Add("YourModule")` is required in both. Files inside `WallRunSystem/` include `PlayerCharacter.h`, `WallRunGameplayTags.h` and `TickableAbility.h` from the module root by plain name, and the module root is not on the include path by default.

Do not copy `WallRun.cpp` in either setup. It contains your module's `IMPLEMENT_PRIMARY_GAME_MODULE`. Each setup's Step 5 shows what to add instead.

## After setup

Continue with the [Animation Setup Guide](animation-setup-guide.md). Questions and fixes are in the [FAQ](faq.md), and every setting is listed in [Configuration](configuration.md).
