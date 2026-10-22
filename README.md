# UE5 C++ Wall Running System

![Wall Run System, C++ wall run and wall jump for UE5](docs/images/thumbnail.png)

A modular, performant wall running and wall jumping system in C++ for Unreal Engine 5.8. Pick one of two setups: a standalone actor component, or a Gameplay Ability System ability. Both share the same core logic and the same Animation Blueprint integration.

Animations are not included. The Animation Blueprint setup is covered in the [Animation Setup Guide](#animation-setup-guide).

## On this page

1. [Setup Guide](#setup-guide)
2. [Animation Setup Guide](#animation-setup-guide)
3. [FAQ](#faq)
4. [Extras](#extras)
   - [Configuration](#configuration)
   - [License](#license)

## Setup Guide

Full guide: [docs/setup-guide.md](docs/setup-guide.md)

Two complete setups. Pick one and follow it from the first step to the last.

| | [Component Setup](docs/setup-component.md) | [GAS Setup](docs/setup-gas.md) |
| :--- | :--- | :--- |
| Core class | `UWallRunComponent` | `UGA_WallRun` |
| Your character needs an Ability System Component | No | Yes |
| Wall run state as gameplay tags | Only if the character has one | Yes |
| Block wall running from other systems | Your own checks | Tag `Status.BlockingAction.WallRun` |
| Best for | Most projects, quickest to set up | Projects already built on GAS |

Not sure? Start with the Component Setup.

**Component Setup steps:** [1 Requirements](docs/setup-component.md#step-1-requirements), [2 Copy files](docs/setup-component.md#step-2-copy-the-source-files), [3 Build.cs](docs/setup-component.md#step-3-update-buildcs), [4 .uproject](docs/setup-component.md#step-4-update-your-uproject), [5 API macro and log](docs/setup-component.md#step-5-rename-the-api-macro-and-add-the-log-category), [6 Build](docs/setup-component.md#step-6-build), [7 Character](docs/setup-component.md#step-7-set-up-your-character), [8 Walls](docs/setup-component.md#step-8-mark-which-walls-can-be-run-on), [9 Settings](docs/setup-component.md#step-9-tune-the-settings), [10 Animation](docs/setup-component.md#step-10-add-animation), [11 Check it works](docs/setup-component.md#step-11-check-it-works), [Events](docs/setup-component.md#listen-to-events), [Troubleshooting](docs/setup-component.md#troubleshooting)

**GAS Setup steps:** [1 Requirements](docs/setup-gas.md#step-1-requirements), [2 Copy files](docs/setup-gas.md#step-2-copy-the-source-files), [3 Build.cs](docs/setup-gas.md#step-3-update-buildcs), [4 .uproject](docs/setup-gas.md#step-4-update-your-uproject), [5 API macro and log](docs/setup-gas.md#step-5-rename-the-api-macro-and-add-the-log-category), [6 Build](docs/setup-gas.md#step-6-build), [7 Character](docs/setup-gas.md#step-7-set-up-your-character), [8 Walls](docs/setup-gas.md#step-8-mark-which-walls-can-be-run-on), [9 Settings](docs/setup-gas.md#step-9-tune-the-settings), [10 Animation](docs/setup-gas.md#step-10-add-animation), [11 Check it works](docs/setup-gas.md#step-11-check-it-works), [Gameplay tags](docs/setup-gas.md#gameplay-tags), [Block with tags](docs/setup-gas.md#block-wall-running-with-tags), [Troubleshooting](docs/setup-gas.md#troubleshooting)

## Animation Setup Guide

Full guide: [docs/animation-setup-guide.md](docs/animation-setup-guide.md)

All animation is handled in your Animation Blueprint, so wall running blends like any other locomotion state.

- [Reparent your Anim Blueprint](docs/animation-setup-guide.md#reparent-your-anim-blueprint)
- [Available variables](docs/animation-setup-guide.md#available-variables)
- [Set up the state machine](docs/animation-setup-guide.md#set-up-the-state-machine)
- [Disable Foot IK while wall running](docs/animation-setup-guide.md#disable-foot-ik-while-wall-running)
- [Procedural lean and spine tilt](docs/animation-setup-guide.md#procedural-lean-and-spine-tilt)
- [Using your own AnimInstance](docs/animation-setup-guide.md#using-your-own-animinstance)

## FAQ

Full FAQ: [docs/faq.md](docs/faq.md)

- [Component or GAS?](docs/faq.md#component-or-gas)
- [Do I need GAS if I pick the component?](docs/faq.md#do-i-need-gas-if-i-pick-the-component)
- [Why does the wall run not start?](docs/faq.md#why-does-the-wall-run-not-start)
- [Why can I not steer while wall running?](docs/faq.md#why-can-i-not-steer-while-wall-running)
- [How do I allow or block specific walls?](docs/faq.md#how-do-i-allow-or-block-specific-walls)
- [What does the corner angle limit do?](docs/faq.md#what-does-the-corner-angle-limit-do)
- [My character does not play any wall run animation](docs/faq.md#my-character-does-not-play-any-wall-run-animation)
- [Does it work in a packaged build?](docs/faq.md#does-it-work-in-a-packaged-build)

## Extras

### Configuration

Full reference: [docs/configuration.md](docs/configuration.md)

[General](docs/configuration.md#general), [collision](docs/configuration.md#collision), [corners](docs/configuration.md#corners), [movement](docs/configuration.md#movement), [wall jump](docs/configuration.md#wall-jump), [visuals](docs/configuration.md#visuals), [runtime state](docs/configuration.md#runtime-state)

### License

See [LICENSE.md](LICENSE.md).
