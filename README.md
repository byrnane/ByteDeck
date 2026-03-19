# ByteDeck

ByteDeck is a lightweight SDL2 launcher for TrimUI Smart Pro S.

Current baseline is already validated on hardware:

- the app appears in the stock `Apps` menu
- launcher UI starts on device
- input works
- ROM scanning works
- stock emulator handoff works

## Structure

The repository is split into three clear zones:

- source and tracked config in the repo root
- local-only heavy data in `local/`
- generated outputs in `out/`

Tracked project layout:

```text
ByteDeck/
  cmake/
    toolchains/
  config/
  device/
    trimui_sps/
  docs/
  scripts/
  src/
```

Local-only layout:

```text
local/
  references/
  sdk/
    trimui_sps/
```

Generated layout:

```text
out/
  host/
  trimui_sps/
  runtime/

dist/
  windows/
  trimui_sps/
```

Development content roots stay in the repo root:

- `roms/`
- `bios/`
- `Apps/`
- `collections/`

## Main Scripts

Public scripts are intentionally reduced to four workflows:

- `scripts/build-windows.ps1`
- `scripts/dev-windows.ps1`
- `scripts/build-trimui_sps.ps1`
- `scripts/clean.ps1`

Explorer wrappers:

- `scripts/build-windows.bat`
- `scripts/dev-windows.bat`
- `scripts/build-trimui_sps.bat`
- `scripts/clean.bat`

Internal helper:

- `scripts/_build-trimui_sps-wsl.sh`

## Root Launcher

There is also a root-level launcher for everyday use:

- `ByteDeck.bat`

It supports both modes:

- double-click in Explorer for a simple menu
- direct commands from terminal

Examples:

```bat
ByteDeck.bat dev-windows
ByteDeck.bat build-windows -Clean
ByteDeck.bat build-trimui_sps -Clean
ByteDeck.bat clean
```

## Desktop Workflow

### Build

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1
```

Result:

- intermediate build: `out/host/Release/`
- ready desktop package: `dist/windows/`

### Build And Run

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\dev-windows.ps1
```

What it does:

- builds the app
- stages `dist/windows/`
- runs the staged executable
- uses repository content roots (`roms`, `bios`, `Apps`, `collections`)
- writes desktop runtime cache/logs to `out/runtime/`

## TrimUI Smart Pro S Workflow

### Requirements

- WSL with Ubuntu
- `cmake` and `ninja-build` installed inside WSL
- official SDK extracted to `local/sdk/trimui_sps/`

Install WSL tools:

```bash
sudo apt update
sudo apt install -y cmake ninja-build
```

### Build

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-trimui_sps.ps1 -Clean
```

Result:

- intermediate ARM build: `out/trimui_sps/Release/`
- ready SD overlay: `dist/trimui_sps/`

Copy to SD:

- copy `dist/trimui_sps/Apps/ByteDeck` to `SDCARD/Apps/ByteDeck`

Important:

- ByteDeck still uses its own ROM folder naming inside the ROM root
- on stock SD layout this means folders such as `Roms/nes`, `Roms/megadrive`, `Roms/psp`
- stock naming is used only for integration with the firmware, not as ByteDeck's internal library model

## Controls

- D-Pad or arrow keys: move selection
- `A` / `Enter`: select
- `B` / `Escape`: back
- `Menu` / `Q`: quit
- hold D-Pad or arrows: auto-repeat scroll
- `F11`: toggle fullscreen on desktop

## Cleanup

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\clean.ps1
```

This removes generated paths only:

- `out/`
- `dist/`
- legacy `build/`, `build-trimui/`, `cache/`

## Notes

- `cmake/toolchains/` contains tracked CMake toolchain definitions
- `local/sdk/trimui_sps/` contains the real external SDK
- `dist/` is the ready-to-use result
- `out/` is internal build output

Additional docs:

- `docs/ARCHITECTURE.md`
- `docs/PLATFORM_NOTES.md`
- `device/trimui_sps/README.md`
