# Crash Team Racing: High Octane
<img src="screenshots/game1.jpg"></img><br>
Crash Team Racing: High Octane is a sourceport for PSVita of Crash Team Racing based on the [ctr-native](https://github.com/CTR-tools/ctr-native) project.
It focuses on new features, enhancements and optimization.

## Features

- True widescreen with no stretching.
- Internal resolution of the renderer bumped to 960x544.
- MSAA 4x for anti-aliasing.
- 60 FPS support.
- Penta Penguin has its stats set to its PAL/NTSC-J counterpart (6/6/6).
- Playable Nitrous Oxide (Unlockable via the original Spyro 2 Demo cheatcode). (Credits: [Original mod](https://github.com/CTR-tools/CTR-ModSDK/tree/main/mods/Modules/OxideFix))
- Reserves Meter (Credits: [Original mod](https://github.com/CTR-tools/CTR-ModSDK/tree/main/mods/Modules/ReservesMeter))
- Customizable Cups (Credits: [Original mod](https://github.com/CTR-tools/CTR-ModSDK/tree/main/mods/Modules/CustomCups))
- Multilanguage support.
- Super and Ultra Hard Difficulty modes for Arcade mode.
- USF will show as blue fire (similar to CTR: Nitro Fueled). (Credits: [Original mod](https://github.com/CTR-tools/CTR-ModSDK/tree/main/mods/Modules/BlueFire))
- Mirror mode option: Play any track specular.
- Boss Fight option: Challenge Adventure mode bossfights on any track.
- AdHoc Netplay support for two PSVitas multiplayer without the need of a router.
- Several vanilla game bugfixes (eg: PVS related glitches and Penta-Penguin wrong mask powerup HUD icon).
- Ghost Replay feature: Replay all your ghost datas as if you're seeing the run being played live.
- Increased ghost data limits: No more 7 ghosts globally, now there are 7 ghosts data slot per track.
- Stats viewer for characters in the character selection screen.
- Online leaderboard for Time Trials results.
- Splitscreen support for up to 4 players local multiplayer for PSTV users or PSVita users with MiniVitaTV.

## Online leaderboard

The full online leaderboard for Time Trails is available at this link: <a href="https://www.rinnegatamante.eu/ctr/leaderboard/">Online Leaderboard</a>.

## Known Issues

- The demo cutscene gets slightly de-synced during Oxide speech.

## Special controls bindings

- L2 and R2 are also mapped on right analog left/right to let PSVita use those controls.

## How to Install

- Install the .vpk.
- Dump your US copy of `Crash Team Racing` for PS1 and place the bin file in `ux0:data/ctr/assets/ctr-u.bin`.

## Changelog

### v.1.2

- Fixed N. Oxide portrait slideing in/out from the left instead of from the bottom in the Character Select screen.
- Fixed a bug causing big black glitched textures to show on screen under certain circumstances during singleplayer races.
- Optimized audio mixing and input handling code.
- Rewrote the whole renderer: now it's extremely closer to PSVita GPU architecture. (Average GPU workload per frame went from 31ms to 16ms)
- Rewrote renderer pipeline so that now works in a multi-threaded fashion (backend/frontend approach). This reduces overall CPU workload per frame from 22 ms to 13ms.
- Added 60 FPS support. (Available in the Options menu)
- Added support for multiple controllers on PSTV and PSVita with MiniVitaTV, allowing for local splitscreen games (up to 4 players).
- Made so that Sewer Speedway and Blizzard Bluffs environmental hazards are now deterministic. This also fixes broken ghosts on these specific tracks.
- Added an Online Leaderboard for Time Trial results. Your best scores will automatically be uploaded to it and you can watch ghosts of the top 5 scores worldwide.
- Fixed two different bugs both causing some tiles to be incorrectly clipped under certain circumstances.
- Made so that when an AdHoc connection is interrupted, the console will automatically return in Internet mode.
- Fixed a bug causing missiles used by enemy AIs to not be homing and instead always proceeding in a straight line.

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

## vitaGL flags for compilation

`HAVE_SHADER_CACHE=1 NO_DEBUG=1 READBACKS_SPEEDHACK=1 CIRCULAR_POOL_SPEEDHACK=1`

## Credits

- Standard-Republic for the Livearea assets.
- robin994 for helping testing splitscreen implementation.
- All the folks involved in ctr-native and the decompilation efforts of CTR.
