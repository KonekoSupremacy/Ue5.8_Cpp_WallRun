# UE5 C++ Wall Running System

A clean, modular, and performant Wall Running & Wall Jumping system built in C++ for Unreal Engine 5. Supports both standalone **Actor Component (`UWallRunComponent`)** and **Gameplay Ability System (`UWallRunGameplayAbility`)** workflows with seamless Animation Blueprint integration.

---

## Architecture Overview

All animations are handled directly inside the **Animation Blueprint (ABP)** rather than being hardcoded into C++ animation montages. This architecture provides several major advantages:
- **Clean Locomotion Blending:** Wall running acts as an in-air locomotion state with proper entry/exit blend times rather than overriding montage slots.
- **IK & Control Rig Compatibility:** Full control over Foot IK and Control Rig bone chains to avoid feet pinning to the ground or pulling unnaturally while running on elevated walls.
- **Dynamic Play Rate & Banking:** Animation playback speeds scale fluidly with character velocity, and vertical arc angles drive procedural spine lean.

---

## Animation Blueprint (ABP) Integration Guide

### 1. Reparent Your Anim Blueprint
The system includes `UWallRunAnimInstance`, which automatically finds the character's `UWallRunComponent` (or active `UWallRunGameplayAbility`) and synchronizes wall-run state every frame.

1. Open your Character's Animation Blueprint (e.g. `ABP_Manny` / `ABP_Quinn`).
2. Go to **File > Reparent Blueprint**.
3. Select **`WallRunAnimInstance`**.
4. If your project already uses a custom `UAnimInstance` C++ base, you can instead cast to `UWallRunComponent` or query `FWallRunState` in your Event Graph using `TryGetPawnOwner()->FindComponentByClass<UWallRunComponent>()`.

---

### 2. Available Anim Instance Variables
The following properties are exposed directly to Blueprint in `UWallRunAnimInstance`:

| Variable | Type | Description |
| :--- | :--- | :--- |
| `bIsWallRunning` | `Boolean` | `true` when the character is actively wall running. |
| `bIsWallJumping` | `Boolean` | `true` while executing a launch off the wall. |
| `WallSide` | `EWallRunSide` | Enum: `Left` or `Right`. Identifies which side of the character is against the wall. |
| `WallRunAnimPlayRate` | `Float` | Dynamic playback rate multiplier (scaled from `0.95` to `1.25` based on speed). |
| `CurrentArcAngle` | `Float` | Vertical arc pitch angle (positive when rising, negative when descending). |
| `CurrentTurnRate` | `Float` | Yaw angular turning velocity (useful for banking into curved walls). |
| `CurrentSpeed` | `Float` | Current character movement speed in Unreal Units/sec. |

---

### 3. Setting Up the State Machine (Locomotion / In-Air)

In your Main Locomotion State Machine (or In-Air State Machine):

```
+----------------+      bIsWallRunning == true       +----------------+
|                | --------------------------------> |                |
| Falling/In-Air |                                   |  Wall Running  |
|                | <-------------------------------- |                |
+----------------+      bIsWallRunning == false      +----------------+
```

#### Inside the `Wall Running` State:
1. Add a **Blend Poses by `EWallRunSide`** (or **Blend Poses by Bool** checking `WallSide == EWallRunSide::Right`).
2. Wire your **Wall Run Right** animation into the `Right` pose.
3. Wire your **Wall Run Left** animation into the `Left` pose.
4. Connect `WallRunAnimPlayRate` to the **Play Rate** input pin of both animation sequence player nodes.
5. Set transition crossfade duration to `0.15s` - `0.2s` with **Cubic** or **Hermite** easing for smooth entry/exit blending.

---

### 4. Handling Foot IK / Control Rig (Crucial Step)

> [!WARNING]  
> If using UE5 Mannequin or Metahumans with a Foot IK Control Rig (such as `CR_Mannequin_FootIK` in `ABP_Manny`), the IK trace by default casts downward towards the ground. When wall running mid-air, the traces may detect the floor 500 units below and pull the character's feet down into an unnatural or stretched pose.

#### How to Disable Foot IK During Wall Run:
1. In the **AnimGraph**, find the **Control Rig** node responsible for Foot IK.
2. Select the node and locate the **Alpha** pin in the Details panel.
3. Drive the Alpha pin with a float:
   - If `bIsWallRunning` is `true`, blend Alpha to `0.0`.
   - If `bIsWallRunning` is `false`, blend Alpha to `1.0`.
4. Alternatively, wrap the Control Rig node in a **Blend Poses by Bool** node:
   - **True (bIsWallRunning):** Bypass the Control Rig node directly to the output pose.
   - **False:** Run through the Foot IK Control Rig node.

---

### 5. Procedural Lean & Spine Tilt (Optional Enhancement)

To give characters extra weight and immersion when running along curved surfaces or arcing up/down:
1. In the AnimGraph, place a **Transform (Modify) Bone** node targeting `spine_01` or `spine_02`.
2. Map `CurrentArcAngle` to **Rotation Pitch**:
   - Upward arc tilts character slightly upward; downward descent tilts forward.
3. Map `CurrentTurnRate` to **Rotation Roll**:
   - Banking into curved inside/outside corners.
4. Set **Alpha** driven by `bIsWallRunning` with smooth interpolation (`FInterpTo`).

---

## C++ Core Components

### 1. `UWallRunComponent` (Standard Actor Component)
- Attach to any `ACharacter`.
- Automatically binds to `OnMovementModeChanged` and `OnActorHit`.
- Handles wall detection, ray traces, collision clearance, corner navigation, and smooth lateral mesh positioning.
- Exposes delegates:
  - `OnWallRunStarted(EWallRunSide Side)`
  - `OnWallRunEnded()`
  - `OnWallJump(const FVector& LaunchVelocity)`
  - `OnWallSideChanged(EWallRunSide NewSide)`

### 2. `UWallRunGameplayAbility` (Gameplay Ability System)
- Inherits from `UGameplayAbility` (`InstancedPerActor`).
- Granted to player characters using an `AbilitySystemComponent`.
- Manages ability activation/cancellation, gameplay tags (`Status.WallRunning`, `Status.WallJumping`), and block tags (`Status.BlockingAction.HorizontalWallRun`).

### 3. `UWallRunUtilityLibrary`
- Stateless, pure static function library implementing all core tracing, velocity projection, corner angle limits, and arc calculations used by both the component and ability.

---

## Configuration (`FWallRunSettings`)

Key properties configurable in Details / Defaults:
- **`WallRunSpeed`** (`800.0`): Target forward wall-running velocity.
- **`WallRunAcceleration`** (`50000.0`): Instantaneous response when latching onto a wall.
- **`ArcAmount`** (`0.65`): Vertical arc shape (`0.0` = flat horizontal line, `1.0` = high arc).
- **`WallRunDurationLimit`** (`3.0s`): Maximum time allowed on a single wall before dropping (`0` = unlimited).
- **`WallJumpAngle`** (`45.0` deg): Launch trajectory angle outward from the wall.
- **`WallJumpHorizontalVelocity`** (`800.0`) & **`WallJumpVerticalVelocity`** (`500.0`): Launch impulse strengths.
- **`MeshWallOffset`** (`5.0` cm): Smooth lateral mesh adjustment to eliminate visual floating without clipping the capsule.
- **`MinStartSpeed`** (`200.0`): Minimum horizontal speed required to initiate a wall run if no movement input is provided. Prevents triggering a wall run when performing a neutral jump in place beside a wall.
- **`bRequiresTag`** (`false`): When `true`, only surfaces with actor tag `HorizontalWallRun` allow wall running. Surfaces with `NoWallRun` are always blocked.
