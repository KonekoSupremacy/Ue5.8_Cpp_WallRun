# Animation Setup Guide

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

Animations are driven from your Animation Blueprint, not from C++ montages. Wall running is an in-air locomotion state with proper blend times, and Foot IK stays under your control.

This works the same for the [component setup](setup-component.md) and the [GAS setup](setup-gas.md).

## On this page

- [Reparent your Anim Blueprint](#reparent-your-anim-blueprint)
- [Available variables](#available-variables)
- [Set up the state machine](#set-up-the-state-machine)
- [Disable Foot IK while wall running](#disable-foot-ik-while-wall-running)
- [Procedural lean and spine tilt](#procedural-lean-and-spine-tilt)
- [Using your own AnimInstance](#using-your-own-animinstance)

## Reparent your Anim Blueprint

`UWallRunAnimInstance` looks for a `UWallRunComponent` on the pawn when the animation initializes. If there is none, it finds the Ability System Component and reads the state from the granted `UGA_WallRun`. It then updates its variables every frame.

1. Open your character's Animation Blueprint, for example `ABP_Manny` or `ABP_Quinn`.
2. File, Reparent Blueprint.
3. Choose `WallRunAnimInstance`.
4. Compile. The variables below now appear in the Event Graph and AnimGraph.

## Available variables

| Variable | Type | Description |
| :--- | :--- | :--- |
| `bIsWallRunning` | Boolean | True while the character is wall running |
| `bIsWallJumping` | Boolean | True while launching off the wall |
| `WallSide` | `EWallRunSide` | `Left` or `Right`, which side the wall is on |
| `WallRunAnimPlayRate` | Float | Play rate scaled by speed, default 0.95 to 1.25 |
| `CurrentArcAngle` | Float | Vertical arc pitch, positive when rising |
| `CurrentTurnRate` | Float | Yaw turning speed, useful for banking |
| `CurrentSpeed` | Float | Current speed in cm/s |

Two more are editable, so you can tune the play rate mapping per character:

| Variable | Default | Description |
| :--- | :--- | :--- |
| `PlayRateSpeedRange` | 500 to 900 | Speed range that maps to the play rate range |
| `PlayRateRange` | 0.95 to 1.25 | Play rate at the low and high end of the speed range |

## Set up the state machine

In your locomotion or in-air state machine, add a Wall Running state:

```
Falling  --- bIsWallRunning == true --->  Wall Running
Falling  <-- bIsWallRunning == false ---  Wall Running
```

Inside the Wall Running state:

1. Add Blend Poses by Enum, set to `EWallRunSide`, or Blend Poses by Bool checking `WallSide == Right`.
2. Plug your Wall Run Right animation into the Right pose.
3. Plug your Wall Run Left animation into the Left pose.
4. Connect `WallRunAnimPlayRate` to the Play Rate pin of both sequence players.
5. Set transition blend times to 0.15 to 0.2 seconds with Cubic or Hermite easing.

For the wall jump, either add a transition on `bIsWallJumping` into your jump or falling state, or let the normal falling state take over.

## Disable Foot IK while wall running

The Foot IK Control Rig traces downward for the ground. While you run on a wall high above the floor, it finds the floor far below and stretches the legs toward it. Turn it off during wall runs:

1. In the AnimGraph, find the Control Rig node that does Foot IK, for example `CR_Mannequin_FootIK` in `ABP_Manny`.
2. Drive its Alpha pin with a float. Blend to 0 when `bIsWallRunning` is true, and back to 1 when false.
3. Or wrap the node in Blend Poses by Bool, with the true branch bypassing the Control Rig and the false branch running it.

## Procedural lean and spine tilt

Optional, adds weight on curved and arcing walls:

1. Add a Transform (Modify) Bone node targeting `spine_01` or `spine_02`.
2. Map `CurrentArcAngle` to Rotation Pitch, so rising tilts up and falling tilts forward.
3. Map `CurrentTurnRate` to Rotation Roll, to bank into corners.
4. Drive Alpha from `bIsWallRunning` with smooth interpolation, for example `FInterpTo`.

## Using your own AnimInstance

If your project already has a custom C++ `UAnimInstance` base, you do not have to reparent. Read the state yourself.

Component setup:

```cpp
if (const UWallRunComponent* WallRun = TryGetPawnOwner()->FindComponentByClass<UWallRunComponent>()) {
	const FWallRunState& State = WallRun->GetWallRunState();
}
```

GAS setup: find the `UGA_WallRun` spec instance on the Ability System Component and call `GetWallRunState()`, as `UWallRunAnimInstance::NativeThreadSafeUpdateAnimation` does.
