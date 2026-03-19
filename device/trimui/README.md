# TrimUI Device Packaging

This directory contains the stock-firmware packaging files for ByteDeck on TrimUI Smart Pro S.

## Deployment Model

ByteDeck is packaged as a normal stock app:

```text
Apps/ByteDeck/
```

The packaged app entry files are:

- `device/trimui/package-root/Apps/ByteDeck/config.json`
- `device/trimui/package-root/Apps/ByteDeck/launch.sh`

## What The Wrapper Does

`launch.sh` is responsible for:

- resolving the SD card root
- exporting `BYTEDECK_*` path overrides
- enabling execute mode for the launch adapter
- forcing joystick input backend on device
- setting `LD_LIBRARY_PATH`
- starting `bin/bytedeck`

## Build And Stage

Recommended path on Windows:

1. Extract the official SDK in WSL under `local/sdk/trimui/`.
2. Build the ARM binary:

```bash
./scripts/build-trimui-wsl.sh --clean
```

3. Stage the SD overlay:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package-trimui.ps1
```

Or run the full flow at once:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\dev-trimui.ps1 -Clean
```

Explorer wrapper:

```bat
scripts\dev-trimui.bat --clean
```

## Staged Output

Packaging produces:

```text
out/package/trimui-sd-overlay/
  Apps/
    ByteDeck/
      bin/
      lib/
      config/
      assets/
      scripts/
      cache/
      config.json
      launch.sh
```

Copy `out/package/trimui-sd-overlay/Apps/ByteDeck` to the SD card under `Apps/ByteDeck`.

## Runtime Assumptions

- stock firmware
- SD root mounted as `/mnt/SDCARD`
- stock emulator launch scripts available under `Emus/`
- official ROM root exposed to ByteDeck through `BYTEDECK_ROMS_ROOT`

## Current Notes

- device launch from the stock `Apps` menu is validated
- input on device is validated through the SDL joystick backend
- stock emulator handoff is validated
- packaged runtime libraries should continue to live in `Apps/ByteDeck/lib`
