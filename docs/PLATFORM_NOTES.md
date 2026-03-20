# TrimUI SPS Platform Notes

- [Русская версия](./PLATFORM_NOTES.ru.md)
- [Project README](../README.md#en)
- [Architecture](./ARCHITECTURE.md)
- [TrimUI SPS Packaging](../device/trimui_sps/README.md)

## Target

ByteDeck currently targets **TrimUI Smart Pro S** on stock firmware.

Repository device id:

```text
trimui_sps
```

Stock platform id used by TrimUI assets and SDK:

```text
tg5050
```

Architecture:

```text
aarch64
```

## SD Card Runtime Model

Confirmed SD card root on device:

```text
/mnt/SDCARD
```

Stock app entry format:

```text
Apps/<AppName>/config.json
Apps/<AppName>/launch.sh
```

ByteDeck deploys as:

```text
/mnt/SDCARD/Apps/ByteDeck
```

## SDK And Build

Canonical SDK location in this repository:

```text
local/sdk/trimui_sps
```

Build entrypoints:

- `scripts/build-trimui_sps.ps1`
- `scripts/_build-trimui_sps-wsl.sh`
- `cmake/toolchains/trimui_sps-aarch64-linux-gnu.cmake`

## Input

On hardware, SDL joystick input is the most reliable backend for ByteDeck.

Current device wrapper sets:

```text
BYTEDECK_INPUT_BACKEND=joystick
```

## Emulator Handoff

ByteDeck does not launch emulators directly.

It forwards to stock launcher scripts under:

```text
/mnt/SDCARD/Emus/<System>/launch.sh
```

Validated mappings:

- `nes -> Emus/FC/launch.sh`
- `snes -> Emus/SFC/launch.sh`
- `megadrive -> Emus/MD/launch.sh`
- `psp -> Emus/PPSSPP/launch.sh`

Important detail:

- ByteDeck must fully shut down SDL before handing off to RetroArch-based launchers
- otherwise video initialization can fail on the device

## ROM Root Conventions

Stock firmware exposes roots such as:

- `Apps/`
- `Emus/`
- `RetroArch/`
- `Roms/`

ByteDeck only uses stock naming for firmware integration.

Inside the ROM root, ByteDeck keeps its own system naming:

- `Roms/nes`
- `Roms/snes`
- `Roms/megadrive`
- `Roms/psp`
