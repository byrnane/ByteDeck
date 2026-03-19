# ByteDeck

ByteDeck is a lightweight SDL2 launcher for TrimUI Smart Pro S. The project now has two working targets:

- desktop build for day-to-day development on Windows
- device build for stock TrimUI firmware as `Apps/ByteDeck`

Current baseline is already validated on hardware:

- the app appears in the stock `Apps` menu
- launcher UI starts on device
- input works through SDL joystick events
- ROM library scan works
- stock emulator handoff works

## Current Scope

ByteDeck currently provides:

- SDL2 application shell and screen stack
- ROM scan from real files
- `gamelist.xml` parsing and metadata merge
- normalized library cache in `cache/library.json`
- systems list and game browser UI
- app scan from `Apps/*/manifest.json`
- launch adapter that delegates game startup to stock `Emus/*/launch.sh`

Current gaps:

- settings screen is still a stub
- localization is only a foundation, not a complete user-facing system
- launch coverage is currently implemented for `nes`, `snes`, `megadrive`, `psp`

## Repository Layout

```text
ByteDeck/
  cmake/
    toolchains/
  config/
  device/
    trimui/
  docs/
  scripts/
  src/
    app/
    core/
    data/
    launch/
    platform/
    ui/
  Apps/
  bios/
  collections/
  roms/
```

Local development data:

- `roms/`
- `bios/`
- `Apps/`
- `references/`
- `toolchains/`

These paths are intentionally ignored by git.

## Architecture

High-level docs:

- `docs/ARCHITECTURE.md`
- `docs/PLATFORM_NOTES.md`
- `device/trimui/README.md`

Short version:

- `platform/` resolves paths, config and logging
- `core/` scans ROMs, XML and app manifests
- `data/` owns normalized library models and cache serialization
- `ui/` renders screens and maps navigation input
- `launch/` prepares and executes emulator/app handoff
- `device/trimui/` contains stock-firmware packaging files

## Desktop Build

### Windows

Requirements:

- Visual Studio 2022 or Build Tools with C++
- CMake
- `vcpkg`
- SDL2 installed through `vcpkg`

Recommended:

- set `VCPKG_ROOT`
- use the helper scripts from the repository root

Fast dev loop:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\dev-windows.ps1
```

Explorer wrappers:

- `scripts/dev-windows.bat`
- `scripts/dev-windows-clean.bat`
- `scripts/build-windows.bat`
- `scripts/build-windows-clean.bat`
- `scripts/run-windows.bat`

### Linux

Install host dependencies first:

```bash
sudo apt install build-essential cmake libsdl2-dev
```

Then build:

```bash
cmake -S . -B build
cmake --build build
./build/bytedeck
```

## TrimUI Device Build

The supported device path is stock firmware with ByteDeck installed as:

```text
Apps/ByteDeck/
```

### Requirements

- WSL with Ubuntu on the Windows host
- official TrimUI Smart Pro S SDK extracted under `toolchains/`
- `cmake` and `ninja-build` installed inside WSL

Install WSL build tools:

```bash
sudo apt update
sudo apt install -y cmake ninja-build
```

One-command device build and packaging:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\dev-trimui.ps1 -Clean
```

Explorer wrapper:

```bat
scripts\dev-trimui.bat --clean
```

Result:

- ARM binary: `build-trimui/bytedeck`
- SD overlay: `dist/trimui-sd-overlay/Apps/ByteDeck`

### Deploy To SD

1. Start from an official stock SD base.
2. Copy `dist/trimui-sd-overlay/Apps/ByteDeck` to `SDCARD/Apps/ByteDeck`.
3. Insert the card into the console.
4. Launch `ByteDeck` from the stock `Apps` menu.

Important:

- ByteDeck keeps its own ROM folder scheme inside the SD card ROM root.
- On device, the wrapper points ByteDeck to `Roms/`, but ByteDeck still expects system folders such as `Roms/nes`, `Roms/megadrive`, `Roms/psp`.
- Stock TrimUI folder naming is used only for integration with the firmware, not as ByteDeck's internal library model.

## Controls

- D-Pad or arrow keys: move selection
- `A` / `Enter`: select
- `B` / `Escape`: back
- `Menu` / `Q`: quit
- hold D-Pad or arrows: auto-repeat scroll
- `F11`: toggle fullscreen on desktop

## Launch Flow

Desktop:

- default mode is `mock`
- real execution can be enabled with `BYTEDECK_LAUNCH_MODE=execute`

TrimUI:

- packaged wrapper enables execution mode automatically
- ByteDeck shuts down SDL first, then hands off to stock emulator scripts
- this avoids framebuffer/video conflicts when launching RetroArch-based systems

## Generated Paths

Safe to delete:

- `build/`
- `build-trimui/`
- `dist/`
- `cache/library.json`
- `cache/logs/*.log`

Convenience cleanup:

- `scripts/clean-generated.bat`
- `scripts/clean-generated.ps1`

## Notes

- `roms/`, `bios/`, official SDK archives and reference projects stay local and are not committed.
- The official SDK archive should be extracted in WSL or another Linux environment, not with plain Windows `tar`.
- GitHub Actions only validates host builds; device packaging is verified locally.

## Development Hygiene

- `CONTRIBUTING.md`
- `SECURITY.md`
