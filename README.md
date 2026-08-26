# Crash Team Racing: High Octane
<img src="screenshots/game1.jpg"></img><br>
Crash Team Racing: High Octane is a sourceport for PSVita of Crash Team Racing based on the [ctr-native](https://github.com/CTR-tools/ctr-native) project.
It focuses on new features, enhancements and optimization.

## Features

- True widescreen with no stretching.
- Internal resolution of the renderer bumped to 960x544.
- MSAA 4x for anti-aliasing.
- Penta Penguin has its stats set to its PAL/NTSC-J counterpart (6/6/6).
- Playable Nitrous Oxide (Unlockable via the original Spyro 2 Demo cheatcode). (Credits: [Original mod](https://github.com/CTR-tools/CTR-ModSDK/tree/main/mods/Modules/OxideFix))
- Reserves Meter (Credits: [Original mod](https://github.com/CTR-tools/CTR-ModSDK/tree/main/mods/Modules/ReservesMeter))
- Customizable Cups (Credits: [Original mod](https://github.com/CTR-tools/CTR-ModSDK/tree/main/mods/Modules/CustomCups))
- Multilanguage support.
- Super and Ultra Difficulty modes for Arcade mode.
- USF will show as blue fire (similar to CTR: Nitro Fueled). (Credits: [Original mod](https://github.com/CTR-tools/CTR-ModSDK/tree/main/mods/Modules/BlueFire))
- Mirror mode option.

## Known Issues

- The primitives clipper sometimes erases primitives that should instead be present (Was an issue also in Vanilla game but it's more evident with widescreen on).
- When missiles are used, framerate can tank due to its effect.
- Splitscreen is disabled (this is due to the fact it runs at 10 FPS in 2P mode right now due to GPU pipeline being still pretty heavy).
- The demo cutscene gets slightly de-synced during Oxide speech.

## Special controls bindings

- L2 and R2 are also mapped on right analog left/right to let PSVita use those controls.

## How to Install

- Install the .vpk.
- Dump your US copy of `Crash Team Racing` for PS1 and place the bin file in `ux0:data/ctr/assets/ctr-u.bin`.

## Credits

- Standard-Republic for the Livearea assets.
- All the folks involved in ctr-native and the decompilation efforts of CTR.
