# TrimUI SPS Packaging

This directory contains the stock-firmware packaging files for ByteDeck on TrimUI Smart Pro S.

## Deployment Model

ByteDeck is packaged as a normal stock app:

```text
Apps/ByteDeck/
```

Tracked package template files:

- `device/trimui_sps/package-root/Apps/ByteDeck/config.json`
- `device/trimui_sps/package-root/Apps/ByteDeck/launch.sh`

## Build Result

Public build entrypoint:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-trimui_sps.ps1 -Clean
```

This produces:

- intermediate ARM build in `out/trimui_sps/Release/`
- ready SD overlay in `dist/trimui_sps/`

Copy to SD:

- `dist/trimui_sps/Apps/ByteDeck -> SDCARD/Apps/ByteDeck`

## Wrapper Responsibilities

`launch.sh` inside the package:

- resolves SD card roots
- exports `BYTEDECK_*` path overrides
- enables execute mode
- forces joystick backend on device
- sets `LD_LIBRARY_PATH`
- starts `bin/bytedeck`

## Runtime Assumptions

- stock firmware
- SD root mounted as `/mnt/SDCARD`
- stock emulator launch scripts available under `Emus/`
- ROM root exposed to ByteDeck through `BYTEDECK_ROMS_ROOT`
