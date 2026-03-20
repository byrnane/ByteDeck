# TrimUI SPS Packaging

- [Русская версия](./README.ru.md)
- [Project README](../../README.md#en)
- [Platform Notes](../../docs/PLATFORM_NOTES.md)

## Purpose

This directory contains the tracked packaging template for **TrimUI Smart Pro S** on stock firmware.

ByteDeck is packaged as a normal stock application:

```text
Apps/ByteDeck/
```

## Tracked Package Files

- `device/trimui_sps/package-root/Apps/ByteDeck/config.json`
- `device/trimui_sps/package-root/Apps/ByteDeck/launch.sh`

## Build Result

Public build entrypoint:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-trimui_sps.ps1 -Clean
```

Build outputs:

- intermediate ARM build: `out/trimui_sps/Release/`
- ready SD overlay: `dist/trimui_sps/`

Copy to SD card:

```text
dist/trimui_sps/Apps/ByteDeck -> SDCARD/Apps/ByteDeck
```

## Wrapper Responsibilities

The packaged `launch.sh`:

- resolves the SD card roots
- exports `BYTEDECK_*` path overrides
- enables device execute mode
- forces joystick backend
- sets `LD_LIBRARY_PATH`
- starts `bin/bytedeck`

## Runtime Assumptions

- stock firmware
- SD root mounted as `/mnt/SDCARD`
- stock emulator scripts available under `Emus/`
- ROM root exposed through `BYTEDECK_ROMS_ROOT`
