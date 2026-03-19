# TrimUI SPS Platform Notes

These notes capture the confirmed runtime facts for ByteDeck on stock TrimUI Smart Pro S.

## Confirmed Runtime Model

- device target name in this repository: `trimui_sps`
- stock platform id used by TrimUI assets and SDK: `tg5050`
- target architecture: `aarch64`
- SD card root on device: `/mnt/SDCARD`
- stock app entry format:
  - `Apps/<AppName>/config.json`
  - `Apps/<AppName>/launch.sh`

ByteDeck currently deploys as:

```text
/mnt/SDCARD/Apps/ByteDeck/
```

## SDK And Build

Canonical SDK location in this repo:

```text
local/sdk/trimui_sps/
```

Build flow:

- public entrypoint: `scripts/build-trimui_sps.ps1`
- internal WSL builder: `scripts/_build-trimui_sps-wsl.sh`
- tracked CMake toolchain file: `cmake/toolchains/trimui_sps-aarch64-linux-gnu.cmake`

## Input

On hardware, SDL joystick input is more reliable than SDL game-controller mapping.

Current device setting:

- `BYTEDECK_INPUT_BACKEND=joystick`

## Emulator Handoff

ByteDeck does not launch emulators directly.

It forwards to stock emulator scripts under:

```text
/mnt/SDCARD/Emus/<System>/launch.sh
```

Validated mappings:

- `nes -> Emus/FC/launch.sh`
- `snes -> Emus/SFC/launch.sh`
- `megadrive -> Emus/MD/launch.sh`
- `psp -> Emus/PPSSPP/launch.sh`

Important runtime detail:

- ByteDeck must fully shut down SDL before handing off to RetroArch-based launchers
- otherwise RetroArch can fail on framebuffer and video initialization

## Path Conventions

Stock firmware uses roots such as:

- `Apps/`
- `Emus/`
- `RetroArch/`
- `Roms/`

ByteDeck uses stock naming only for firmware integration.

Inside the ROM root, ByteDeck still keeps its own system naming, for example:

- `Roms/nes`
- `Roms/megadrive`
- `Roms/psp`
