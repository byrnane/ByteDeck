# TrimUI Platform Notes

These notes capture the runtime facts that are already confirmed for ByteDeck on stock TrimUI Smart Pro S.

## Confirmed Runtime Model

- device family: TrimUI Smart Pro S
- stock platform id used by TrimUI assets and SDK: `tg5050`
- device target architecture: `aarch64`
- SD card root on device: `/mnt/SDCARD`
- supported app entry format on stock firmware:
  - `Apps/<AppName>/config.json`
  - `Apps/<AppName>/launch.sh`

ByteDeck currently deploys as:

```text
/mnt/SDCARD/Apps/ByteDeck/
```

## Confirmed Build Inputs

The official Smart Pro S SDK provides:

- `aarch64-none-linux-gnu-` cross toolchain
- target sysroot
- SDL2 headers and libraries
- SDL2 CMake package files

Recommended host workflow:

- extract the SDK inside WSL or another Linux environment under `local/sdk/trimui/`
- build the ARM binary through `scripts/build-trimui-wsl.sh`
- stage the SD overlay through `scripts/package-trimui.ps1`

## Confirmed App Launch Behavior

ByteDeck is launched through the stock `Apps` menu.

The packaged wrapper:

- sets `BYTEDECK_*` path overrides
- points ByteDeck at the SD card roots
- enables execute mode for launcher scripts
- exports `BYTEDECK_INPUT_BACKEND=joystick`
- extends `LD_LIBRARY_PATH` with `Apps/ByteDeck/lib`

## Confirmed Input Behavior

On hardware, TrimUI buttons are exposed in a way that is more reliable through raw SDL joystick events than through SDL game-controller mapping.

Current device setting:

- `BYTEDECK_INPUT_BACKEND=joystick`

This avoids duplicate or conflicting input events from the `X360 Controller` SDL mapping seen on the device.

## Confirmed Emulator Handoff Behavior

ByteDeck does not launch emulators directly.

Instead it forwards to stock emulator scripts under:

```text
/mnt/SDCARD/Emus/<System>/launch.sh
```

Current validated mappings:

- `nes -> Emus/FC/launch.sh`
- `snes -> Emus/SFC/launch.sh`
- `megadrive -> Emus/MD/launch.sh`
- `psp -> Emus/PPSSPP/launch.sh`

Important runtime detail:

- ByteDeck must fully shut down SDL before handing off to RetroArch-based stock launchers
- otherwise RetroArch can fail with framebuffer and video initialization errors such as Vulkan `KHR_display` / pageflip failures

This handoff is now implemented as a deferred launch after application shutdown.

## Path Conventions

Stock firmware uses the official SD layout such as:

- `Apps/`
- `Emus/`
- `RetroArch/`
- `Roms/`

ByteDeck uses that layout only for firmware integration.

Inside the ROM root, ByteDeck still keeps its own system naming and scanner model. For example:

- `Roms/nes`
- `Roms/megadrive`
- `Roms/psp`

ByteDeck does not adopt stock per-system folder names as its internal library model.

## Practical Deployment Summary

1. Start from an official stock SD base.
2. Build the ARM binary with WSL.
3. Stage `out/package/trimui-sd-overlay/Apps/ByteDeck`.
4. Copy that folder to `SDCARD/Apps/ByteDeck`.
5. Launch ByteDeck from the stock `Apps` menu.

## Still Intentionally Out Of Scope

- replacing the stock system launcher at boot
- depending on spruceOS, NextUI or CrossMix runtime behavior
- changing ByteDeck's internal ROM structure to match stock TrimUI system naming
