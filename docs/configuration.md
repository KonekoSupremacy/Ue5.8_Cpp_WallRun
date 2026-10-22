# Configuration

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

Every property of `FWallRunSettings`. On the component it is the Settings property, on the ability it is the Settings property in Class Defaults.

## On this page

- [General](#general)
- [Collision](#collision)
- [Corners](#corners)
- [Movement](#movement)
- [Wall jump](#wall-jump)
- [Visuals](#visuals)
- [Runtime state](#runtime-state)

## General

| Property | Default | Description |
| :--- | :--- | :--- |
| `bRequiresTag` | false | When true, only actors with `RequiredTag` can be run on |
| `RequiredTag` | `WallRun` | Actor tag required when `bRequiresTag` is on |
| `BlockingTag` | `NoWallRun` | Actor tag that always blocks wall running, used when `bRequiresTag` is off |

## Collision

| Property | Default | Description |
| :--- | :--- | :--- |
| `TraceChannel` | Visibility | Channel used for wall traces, walls must block it |
| `PrimaryTraceRadiusMultiplier` | 0.85 | Multiplier on the capsule radius for the main wall trace, 0.1 to 2.0 |
| `CapsuleClearance` | 5.0 | Minimum gap between capsule and wall surface, 5 to 50 |

## Corners

| Property | Default | Description |
| :--- | :--- | :--- |
| `CornerAngleLimit` | 85.0 | Maximum angle in degrees between two wall normals that the run will follow, applies to inside and outside corners, 0 to 90 |

How it works: when the trace finds a second wall ahead, the angle between the two wall normals, flattened to the horizontal plane, is compared against this limit. Above the limit the run ends instead of following the corner. Lower it to make the character drop at sharper corners.

## Movement

| Property | Default | Description |
| :--- | :--- | :--- |
| `WallRunSpeed` | 800 | Target speed along the wall |
| `WallRunAcceleration` | 50000 | How instantly you reach that speed when latching |
| `WallRunBrakingDeceleration` | 500 | Braking while wall running |
| `MaxWallRunTurnRate` | 360 | Max degrees per second the character turns to align with the wall, 30 to 1080 |
| `CapsuleAdjustmentSpeed` | 400 | Max speed in cm/s the capsule is repositioned to hug the wall, 50 to 2000 |
| `ArcAmount` | 0.65 | Vertical arc shape, 0 is a flat horizontal line, up to 2 |
| `WallRunDurationLimit` | 3.0 | Seconds before the run ends, 0 means unlimited |
| `MinStartSpeed` | 200 | Horizontal speed needed to start with no movement input, stops neutral jumps beside a wall latching |

## Wall jump

| Property | Default | Description |
| :--- | :--- | :--- |
| `WallJumpAngle` | 45 | Launch angle away from the wall, 90 is perpendicular, 45 is diagonal forward and outward |
| `WallJumpHorizontalVelocity` | 800 | Horizontal launch strength |
| `WallJumpVerticalVelocity` | 500 | Vertical launch strength |
| `WallJumpRotationTime` | 0.2 | Seconds to rotate into the jump direction, 0 is instant |
| `WallJumpGravity` | 1.5 | Gravity scale during the wall jump |
| `WallJumpAirControl` | 0.3 | Air control granted after the delay, 0 to 1 |
| `DelayedAirControlTime` | 0.35 | Seconds after launch before air control is granted |
| `bOverrideBrakingDecelerationFalling` | true | Override falling braking during the jump to keep forward momentum |
| `WallJumpBrakingDecelerationFalling` | 0 | Falling braking applied when the override is on, 0 means no mid-air braking |

## Visuals

| Property | Default | Description |
| :--- | :--- | :--- |
| `MeshWallOffset` | 5.0 | Sideways offset of the skeletal mesh toward the wall, closes visual gaps without moving the capsule, 0 to 50 |
| `MeshInterpSpeed` | 10 | How fast that offset blends in and out, 1 to 30 |

## Runtime state

`FWallRunState` is read only and exposed as State on both the component and the ability. Useful fields from Blueprints:

| Field | Description |
| :--- | :--- |
| `bIsWallRunning` | Currently wall running |
| `bIsWallJumping` | Currently in the wall jump launch |
| `bWallRunningAllowed` | False during the lockout after a jump or timeout |
| `bSpeedTooLow` | Speed dropped below the minimum while running |
| `WallSide` | Left or Right |
| `AverageWallNormal`, `PrimaryTraceNormal`, `SecondaryTraceNormal` | Wall normals used for steering |
| `CurrentWallRunDuration` | Seconds on the current wall |
| `CurrentArcAngle`, `CurrentTurnRate` | The same values the Anim Instance exposes |
