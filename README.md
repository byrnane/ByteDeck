# ByteDeck

ByteDeck is a lightweight SDL2-based launcher skeleton for TrimUI Smart Pro S. This repository currently contains the first development increment defined in the project spec:

- repository structure for launcher code, configs, assets, scripts, cache and docs
- CMake-based build setup for `C++17`
- desktop simulation entry point with an SDL2 window and main loop
- screen stack with basic UI navigation
- normalized data models for games, apps, collections and system entries
- ROM scanner based on real files in `roms/`
- `gamelist.xml` parsing with metadata merge and fallback entries
- unified cache output to `cache/library.json`
- thumbnail loading and UTF-8 text rendering for desktop simulation

The launch adapter integration and full UI browser screens will be expanded in later steps.

## Current Repository Layout

```text
ByteDeck/
  assets/
  cache/
    logs/
  collections/
  config/
    i18n/
  docs/
  scripts/
  src/
    app/
    core/
    platform/
    ui/
  roms/
  bios/
  Apps/
```

## Dependencies

- SDL2
- tinyxml2
- nlohmann/json
- CMake 3.16+
- C++17 compiler

`tinyxml2` and `nlohmann/json` are fetched automatically by CMake if package discovery fails. SDL2 is expected to be installed on the host system.

## Build

### Linux

Install SDL2 development files first. Package names vary by distro, but the common equivalent is:

```bash
sudo apt install build-essential cmake libsdl2-dev
```

Then configure and build:

```bash
cmake -S . -B build
cmake --build build
```

Run:

```bash
./build/bytedeck
```

### Windows

Install:

- CMake
- Visual Studio Build Tools or Visual Studio with C++
- SDL2 development package or SDL2 via vcpkg

The repository includes PowerShell helper scripts for the common Windows workflow.

Recommended setup:

- set `VCPKG_ROOT` to your `vcpkg` directory, or install `vcpkg` into `C:\vcpkg`
- run the helper scripts from the repository root

One-command dev loop:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\dev-windows.ps1
```

Explorer-friendly wrappers are also included, so you can double-click these files directly:

- `scripts/dev-windows.bat`
- `scripts/dev-windows-clean.bat`
- `scripts/build-windows.bat`
- `scripts/build-windows-clean.bat`
- `scripts/run-windows.bat`

Recommended usage:

- `dev-windows.bat` for normal fast iteration
- `dev-windows-clean.bat` after dependency, CMake or toolchain changes
- `build-windows-clean.bat` when you want a clean rebuild without auto-run

Build only:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1
```

Run an existing build:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run-windows.ps1
```

Optional parameters:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Config Debug
powershell -ExecutionPolicy Bypass -File .\scripts\dev-windows.ps1 -Clean
```

Example with vcpkg:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
.\build\Release\bytedeck.exe
```

## Current Controls

- Arrow keys or D-Pad: move selection
- `Enter` / controller `A`: select
- `Escape` / controller `B`: back
- `F11`: toggle fullscreen
- `Q`: quit desktop simulation

Navigation currently implemented:

- Main Menu
- Games screen with visible systems
- Game Browser with left list and right preview panel
- Apps screen with list and preview panel
- placeholder Settings screen

## Current UI State

The current build opens a desktop window and renders:

- Games
- Settings
- Apps

The selected item is highlighted visually, and the current selection is reflected in the window title.

The game browser currently shows:

- game list on the left
- metadata and thumbnail panel on the right

## Current Library Scan Behavior

On startup the launcher now:

- scans supported ROM roots under `roms/`
- detects valid ROMs by system-specific extension
- parses `gamelist.xml` when present
- merges metadata only for ROMs that exist on disk
- creates fallback entries for ROMs missing from `gamelist.xml`
- scans `Apps/*/manifest.json` when present
- scans `collections/*.json` when present
- writes normalized cache to `cache/library.json`

The scanner is designed to tolerate:

- missing `gamelist.xml`
- malformed XML
- missing thumbnails
- broken app manifests
- empty `Apps/` and `collections/`

## Configuration

User settings are stored in `config/user_settings.json`.

Translations live in `config/i18n/translations.csv`.

## Notes

- The sample `roms/` and `bios/` directories in the repository are treated as source test data for later scanner work.
- Paths are resolved relative to the repository root in desktop simulation.
- Platform-specific TrimUI discovery notes are tracked in `docs/PLATFORM_NOTES.md`.
- `roms/` and `bios/` are intentionally ignored by git and stay local to each developer machine.

## Development Hygiene

- See `CONTRIBUTING.md` for local workflow and commit guidance.
- See `SECURITY.md` for security reporting guidance.
- CI runs CMake configure and build checks on push and pull request.
