# Setup Guide

[Back to README](../README.md) | [Setup Guide](setup-guide.md) | [Animation Setup Guide](animation-setup-guide.md) | [FAQ](faq.md) | [Configuration](configuration.md)

Two steps: do the common installation below, then follow either the Component setup or the GAS setup. After that, continue with the [Animation Setup Guide](animation-setup-guide.md).

## On this page

- [Requirements](#requirements)
- [Copy the source files](#copy-the-source-files)
- [Update Build.cs](#update-buildcs)
- [Update your .uproject](#update-your-uproject)
- [Rename the API macro](#rename-the-api-macro)
- [Rebuild](#rebuild)
- [Choose your setup](#choose-your-setup)
  - [Component setup](setup-component.md)
  - [GAS setup](setup-gas.md)

## Requirements

- Unreal Engine 5.8, with a C++ project
- Enhanced Input (the base character classes bind to it)
- The `GameplayAbilities` plugin enabled, even if you only use the component setup, because the shared utility library includes the GAS headers

## Copy the source files

Copy these into your project's module `Source/<YourModule>/` folder, keeping the folder layout:

| Path | Needed for |
| :--- | :--- |
| `WallRunSystem/` (whole folder) | Both setups |
| `WallRunGameplayTags.h` and `.cpp` | Both setups |
| `TickableAbility.h` | GAS setup |
| `PlayerCharacter.h` and `.cpp` | Optional base character, needed by `UGA_WallRun` for jump input |
| `WallRunPlayerCharacter.h` and `.cpp` | Optional, component setup base character |
| `GASPlayerCharacter.h` and `.cpp` | Optional, GAS setup base character |
| `WallRunPlayerController.h` and `.cpp` | Optional, adds your input mapping contexts |
| `WallRun.h` and `.cpp` | Only if you want the `LogWallRun` category used by `PlayerCharacter` |

If you use your own character class instead of the optional base classes, skip those rows and follow the "your own character" section of your chosen setup.

## Update Build.cs

Add these modules to your module's `Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
	"EnhancedInput",
	"GameplayAbilities",
	"GameplayTags",
	"GameplayTasks"
});

PublicIncludePaths.AddRange(new string[] {
	"YourModule",
	"YourModule/WallRunSystem"
});
```

The include paths let the wall run files find each other with plain `#include "WallRunTypes.h"` style includes.

## Update your .uproject

Enable the GAS plugin:

```json
"Plugins": [
	{
		"Name": "GameplayAbilities",
		"Enabled": true
	}
]
```

Make sure there is no trailing comma after the last plugin entry, strict JSON parsers reject it.

## Rename the API macro

The files use `WALLRUN_API`. Replace it with your own module's macro, for example `MYGAME_API`, across all copied headers. Use a project-wide find and replace.

## Rebuild

Close the editor, regenerate project files, and build the editor target. The native gameplay tags (`Status.WallRunning` and friends) register themselves in code, so there is no tag ini to edit.

## Choose your setup

| | [Component setup](setup-component.md) | [GAS setup](setup-gas.md) |
| :--- | :--- | :--- |
| Core class | `UWallRunComponent` | `UGA_WallRun` |
| Needs an Ability System Component | No | Yes |
| State exposed as gameplay tags | Only if the character has an ASC | Yes |
| Block wall running from other systems | Your own checks | Tag `Status.BlockingAction.WallRun` |
| Best for | Most projects, quickest to set up | Projects already built on GAS |

Not sure? Start with the [Component setup](setup-component.md).

Jump straight to a section:

- Component: [included character](setup-component.md#path-a-use-the-included-character), [your own character](setup-component.md#path-b-add-it-to-your-own-character), [wall tags](setup-component.md#mark-which-walls-can-be-run-on), [events](setup-component.md#listen-to-events), [troubleshooting](setup-component.md#troubleshooting)
- GAS: [included character](setup-gas.md#path-a-use-the-included-character), [your own character](setup-gas.md#path-b-add-it-to-your-own-character), [gameplay tags](setup-gas.md#gameplay-tags), [blocking with tags](setup-gas.md#block-wall-running-with-tags), [troubleshooting](setup-gas.md#troubleshooting)

When your setup works, continue with the [Animation Setup Guide](animation-setup-guide.md).
