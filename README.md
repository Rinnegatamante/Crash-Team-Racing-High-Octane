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
- Mirror mode option: Play any track specular.
- Boss Fight option: Challenge Adventure mode bossfights on any track.
- AdHoc Netplay support for two PSVitas multiplayer without the need of a router.
- Several vanilla game bugfixes (eg: PVS related glitches and Penta-Penguin wrong mask powerup HUD icon).
- Ghost Replay feature: Replay all your ghost datas as if you're seeing the run being played live.
- Increased ghost data limits: No more 7 ghosts globally, now there are 7 ghosts data slot per track.
- Stats viewer for characters in the character selection screen.

## Known Issues

- The primitives clipper sometimes erases primitives that should instead be present (Was an issue also in Vanilla game but it's more evident with widescreen on).
- Splitscreen is disabled (this is due to the fact it runs at 10 FPS in 2P mode right now due to GPU pipeline being still pretty heavy).
- The demo cutscene gets slightly de-synced during Oxide speech.

## Special controls bindings

- L2 and R2 are also mapped on right analog left/right to let PSVita use those controls.

## How to Install

- Install the .vpk.
- Dump your US copy of `Crash Team Racing` for PS1 and place the bin file in `ux0:data/ctr/assets/ctr-u.bin`.

## Changelog

### v.1.1

- Made so that the ghosts aren't limited anymore to 7 globally. You can now have 7 ghosts per track.
- Added a Ghost Replay feature that allows you to replay ghost data as if the race is running in single person. (Available only for ghost data generated from v.1.1 or higher)
- Refactored the main menu with submenus so that it's easier to navigate.
- Added a stats viewer in the character selection screen when playing in single player.
- Added Boss Fight mode. This mode allows you to play against the bosses from Adventure mode on any track.
- Optimized GPU workload by optimizing all the various shader variants used by the renderer: this improves overall framerate.
- Added AdHoc netplay: currently limited only to Arcade - Single Track mode, this allows for router-less 2 Vitas netplay.
- Optimized the missiles powerup rendering effect. Now there won't be anymore framedrops when missiles are on screen.
- Fixed a bug in vanilla game that was causing Penta Penguin powerup HUD to show Uka-Uka instead of Aku-Aku.
- Fixed a bug causing the Uka-Uka/Aku-Aku powerup to occasionally enter in stale setups, resulting in audio glitches (eg: powerup music playing permanently or playing when you were recovered from an out of track).
- Added ability to skip the intro from the very first frame of the SCEA copyright screen by pressing START.

## Credits

- Standard-Republic for the Livearea assets.
- All the folks involved in ctr-native and the decompilation efforts of CTR.
