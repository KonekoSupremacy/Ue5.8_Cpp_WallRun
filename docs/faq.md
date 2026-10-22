# FAQ

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

## On this page

**Choosing a setup**
- [Component or GAS?](#component-or-gas)
- [Do I need GAS if I pick the component?](#do-i-need-gas-if-i-pick-the-component)
- [Can I use both at once?](#can-i-use-both-at-once)

**Getting started**
- [Which engine version?](#which-engine-version)
- [Are animations included?](#are-animations-included)
- [Can I use my own character?](#can-i-use-my-own-character)
- [Why can I not place the base character in my level?](#why-can-i-not-place-the-base-character-in-my-level)

**Behaviour**
- [Why does the wall run not start?](#why-does-the-wall-run-not-start)
- [Why can I not steer while wall running?](#why-can-i-not-steer-while-wall-running)
- [How do I allow or block specific walls?](#how-do-i-allow-or-block-specific-walls)
- [How long can I run on a wall?](#how-long-can-i-run-on-a-wall)
- [What does the corner angle limit do?](#what-does-the-corner-angle-limit-do)
- [Why does the run end on a curved wall?](#why-does-the-run-end-on-a-curved-wall)
- [Does it work in multiplayer?](#does-it-work-in-multiplayer)

**Animation**
- [My character does not play any wall run animation](#my-character-does-not-play-any-wall-run-animation)
- [My character's feet stretch toward the floor](#my-characters-feet-stretch-toward-the-floor)

**Packaging**
- [Does it work in a packaged build?](#does-it-work-in-a-packaged-build)

## Choosing a setup

### Component or GAS?

Pick the [Component setup](setup-component.md) unless your project already uses the Gameplay Ability System. It is quicker to set up and needs no Ability System Component. Pick the [GAS setup](setup-gas.md) if you want wall run state as gameplay tags, or want other abilities to block wall running by tag.

### Do I need GAS if I pick the component?

Not in your game, but the code still needs it to compile. The shared utility library includes the Ability System headers, so the `GameplayAbilities` plugin and the `GameplayAbilities`, `GameplayTags` and `GameplayTasks` modules must be enabled. Your character does not need an Ability System Component. See the [Setup Guide](setup-guide.md#update-buildcs).

### Can I use both at once?

Do not. The animation instance looks for a `UWallRunComponent` first and only falls back to the ability if there is no component, so the ability's state would be ignored.

## Getting started

### Which engine version?

Unreal Engine 5.8. It has not been tested on other versions.

### Are animations included?

No. The system reads and exposes state, and your Animation Blueprint plays your own wall run animations. See the [Animation Setup Guide](animation-setup-guide.md).

### Can I use my own character?

Yes. Both setups have a "your own character" path: [component](setup-component.md#path-b-add-it-to-your-own-character) and [GAS](setup-gas.md#path-b-add-it-to-your-own-character). The component path works with any `ACharacter`. The GAS path needs an `APlayerCharacter` for the automatic wall jump input binding, or you call `StartWallJump()` yourself.

### Why can I not place the base character in my level?

`AWallRunPlayerCharacter`, `AGASPlayerCharacter` and `APlayerCharacter` are abstract. Create a Blueprint subclass and use that.

## Behaviour

### Why does the wall run not start?

All of these must be true:

1. The character is in the falling movement mode.
2. The wall blocks the trace channel, Visibility by default.
3. The wall is allowed: it does not have the `NoWallRun` actor tag, and if `bRequiresTag` is on it has the `WallRun` tag.
4. There is movement input, or the horizontal speed is at least `MinStartSpeed`.
5. A lockout from a previous wall jump or timeout is not still running.
6. GAS only: the character does not have `Status.BlockingAction.WallRun`.

### Why can I not steer while wall running?

By design, the system drives velocity along the wall, so normal movement input is ignored while running. Look input still works. Press jump to launch off the wall.

### How do I allow or block specific walls?

Add the actor tag `NoWallRun` to block a wall. To flip it so only chosen walls work, turn on `bRequiresTag` and add the actor tag `WallRun` to each runnable wall. Both tag names are configurable. See [Configuration](configuration.md#general).

### How long can I run on a wall?

`WallRunDurationLimit` seconds, 3 by default. Set it to 0 for unlimited. The run also ends if the speed drops too low, you reach the wall top, or the wall ends.

### What does the corner angle limit do?

When the wall turns into another wall, the system measures the horizontal angle between the two wall normals. If that angle is at most `CornerAngleLimit` the run follows the corner, otherwise it ends. One limit applies to both inside and outside corners. See [Configuration](configuration.md#corners).

### Why does the run end on a curved wall?

On a curve the wall normal keeps changing, so the angle to the next surface can exceed `CornerAngleLimit` partway round. Raise the limit, up to 90, to follow tighter curves. Flat walls at a known angle are easier to tune against.

### Does it work in multiplayer?

It is not covered or tested for multiplayer. Treat it as a single player system.

## Animation

### My character does not play any wall run animation

The Animation Blueprint must be reparented to `WallRunAnimInstance`, and its state machine needs a Wall Running state driven by `bIsWallRunning`. Follow [Reparent your Anim Blueprint](animation-setup-guide.md#reparent-your-anim-blueprint) and [Set up the state machine](animation-setup-guide.md#set-up-the-state-machine).

### My character's feet stretch toward the floor

Foot IK is tracing down to the ground far below. Fade its Alpha to 0 while `bIsWallRunning` is true. See [Disable Foot IK while wall running](animation-setup-guide.md#disable-foot-ik-while-wall-running).

## Packaging

### Does it work in a packaged build?

A Windows Shipping package on Unreal Engine 5.8 builds and runs, with wall running working in the packaged game. The gameplay tags are native, so there is no tag config to ship. Make sure the `GameplayAbilities` plugin is enabled in your `.uproject`, as described in the [Setup Guide](setup-guide.md#update-your-uproject).
