# ByteDeck

ByteDeck is a lightweight SDL2 launcher for TrimUI Smart Pro S. The project currently has two working targets:

- desktop build for daily development on Windows
- device build for stock TrimUI firmware as `Apps/ByteDeck`

Current baseline is already validated on hardware:

- the app appears in the stock `Apps` menu
- launcher UI starts on device
- input works through SDL joystick events
- ROM scanning works
- stock emulator handoff works

## Repository Layout

The repository now follows three zones:

- source and tracked config in the repo root
- local-only heavy assets under `local/`
- all generated outputs under `out/`

Tracked project structure:

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
```

Local-only structure:

```text
local/
  archives/
  references/
  sdk/
    trimui/
```

Generated structure:

```text
out/
  host/
    Release/
  trimui/
    Release/
  package/
    trimui-sd-overlay/
  runtime/
```

Development content roots stay in the repo root:

- `roms/`
- `bios/`
- `Apps/`
- `collections/`

These paths remain local and ignored by git.

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
- or install `vcpkg` in a common path such as `C:\vcpkg` or `D:\vcpkg`

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

Default host output:

- binary: `out/host/Release/bytedeck.exe`
- runtime cache and logs: `out/runtime/`

### Linux

Install host dependencies first:

```bash
sudo apt install build-essential cmake libsdl2-dev
```

Then build manually if needed:

```bash
cmake -S . -B out/host/Release
cmake --build out/host/Release
./out/host/Release/bytedeck
```

## TrimUI Device Build

The supported device path is stock firmware with ByteDeck installed as:

```text
Apps/ByteDeck/
```

### Requirements

- WSL with Ubuntu on the Windows host
- official TrimUI Smart Pro S SDK extracted under `local/sdk/trimui/`
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

- ARM binary: `out/trimui/Release/bytedeck`
- SD overlay: `out/package/trimui-sd-overlay/Apps/ByteDeck`

### Deploy To SD

1. Start from an official stock SD base.
2. Copy `out/package/trimui-sd-overlay/Apps/ByteDeck` to `SDCARD/Apps/ByteDeck`.
3. Insert the card into the console.
4. Launch `ByteDeck` from the stock `Apps` menu.

Important:

- ByteDeck keeps its own ROM folder scheme inside the SD card ROM root.
- On device, the wrapper points ByteDeck to `Roms/`, but ByteDeck still expects folders such as `Roms/nes`, `Roms/megadrive`, `Roms/psp`.
- Stock TrimUI folder naming is used only for firmware integration, not as ByteDeck's internal library model.

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
- this avoids framebuffer and video conflicts when launching RetroArch-based systems

## Cleanup

Safe to delete:

- `out/`
- legacy generated paths such as `build/`, `build-trimui/`, `dist/`

Convenience cleanup:

- `scripts/clean-generated.bat`
- `scripts/clean-generated.ps1`

## Notes

- `local/` holds SDKs, archives and reference material and is never committed.
- `cmake/toolchains/` stays in git because it contains build-system source files, not external toolchains.
- The official SDK archive should be extracted in WSL or another Linux environment, not with plain Windows `tar`.
- GitHub Actions only validates host builds; device packaging is verified locally.

## Development Hygiene

- `CONTRIBUTING.md`
- `SECURITY.md`
