# ByteDeck Architecture

This document describes the current project shape after the first working desktop build and the first successful TrimUI device bring-up.

## Design Goals

ByteDeck is intentionally split into small layers:

- library discovery must stay independent from UI
- UI must not embed emulator-specific logic
- device integration must stay outside the main application code
- stock TrimUI behavior should be reused only at the launch boundary

## Module Overview

### `src/platform`

Responsibilities:

- detect working roots for `roms`, `bios`, `Apps`, `collections`, `cache`, `scripts`, `config`
- load user settings
- write runtime logs

Key files:

- `src/platform/paths.cpp`
- `src/platform/logger.cpp`
- `src/platform/user_settings.cpp`

The platform layer is the only place that knows how repository layout and packaged device layout are resolved into concrete filesystem paths.

### `src/data`

Responsibilities:

- define normalized in-memory models
- serialize the library cache to JSON

Key files:

- `src/data/models.hpp`
- `src/data/models.cpp`

Main entities:

- `GameItem`
- `AppItem`
- `Collection`
- `SystemEntry`
- `LibraryData`

These models do not scan the filesystem and do not render UI.

### `src/core`

Responsibilities:

- scan ROM roots
- parse `gamelist.xml`
- merge metadata with real files
- create fallback items when metadata is missing
- scan apps and collections
- save normalized cache

Key files:

- `src/core/library_scanner.cpp`
- `src/core/gamelist_parser.cpp`

This layer turns real files into `LibraryData`.

### `src/ui`

Responsibilities:

- render screens with SDL2
- map keyboard and controller input to navigation actions
- keep navigation logic local to screens

Key files:

- `src/ui/screen.hpp`
- `src/ui/screen_manager.cpp`
- `src/ui/navigation_input.hpp`
- `src/ui/text_renderer.cpp`
- `src/ui/image_texture.cpp`
- `src/ui/screens/*`

The UI works on `LibraryData` and callbacks. It does not scan ROMs and does not know how emulator scripts are resolved.

### `src/launch`

Responsibilities:

- provide one launch interface for games and apps
- map ByteDeck system ids to stock TrimUI emulator scripts
- defer device launch until the SDL app is fully shut down

Key files:

- `src/launch/launch_service.hpp`
- `src/launch/launch_service.cpp`
- `scripts/launch_item.sh`

This is the boundary between ByteDeck and the stock firmware runtime.

### `src/app`

Responsibilities:

- initialize subsystems
- load library data
- own SDL window and renderer
- dispatch events to the current screen
- coordinate delayed handoff into emulator launch scripts

Key files:

- `src/app/application.hpp`
- `src/app/application.cpp`

`Application` is the composition root. It wires the layers together but should stay thin.

## Runtime Flow

### Startup

1. `main.cpp` creates `Application`.
2. `Application::initialize()` resolves paths and starts logging.
3. `LibraryScanner` scans ROMs, apps and collections.
4. Normalized cache is written to `cache/library.json`.
5. SDL window, renderer and input devices are initialized.
6. `MainMenuScreen` is pushed into the screen manager.

### Navigation

1. SDL events are translated into `NavigationInput`.
2. The current screen returns a `ScreenAction`.
3. `Application` applies the action:
   - push a screen
   - pop a screen
   - quit
   - open a game browser
   - prepare a launch request

### Game Launch

1. `GameBrowserScreen` invokes a callback with the selected `GameItem`.
2. `LaunchService` resolves the stock launch script and target path.
3. On desktop mock mode, it only logs the command.
4. On TrimUI execute mode, it returns a deferred `LaunchRequest`.
5. `Application` exits its main loop and shuts down SDL.
6. After shutdown, `LaunchService::execute_prepared()` replaces the process with `/bin/sh scripts/launch_item.sh ...`.
7. `scripts/launch_item.sh` forwards the request to stock `Emus/*/launch.sh`.

The delayed handoff is important. Launching RetroArch while ByteDeck still owns the framebuffer causes video initialization failures on device.

## Packaging Boundary

TrimUI-specific files live outside `src/`:

- `device/trimui/package-root/Apps/ByteDeck/config.json`
- `device/trimui/package-root/Apps/ByteDeck/launch.sh`
- `scripts/package-trimui.ps1`
- `scripts/build-trimui-wsl.sh`

This keeps stock firmware integration and host-side staging logic separate from the main application code.

## Path Model

ByteDeck supports two distinct path worlds:

- repository-local development roots such as `roms/` and `bios/`
- packaged device roots supplied by `BYTEDECK_*` environment variables

The wrapper script inside `Apps/ByteDeck/launch.sh` sets these variables explicitly on TrimUI, so the C++ code does not need device-specific hardcoded paths.

## Current Technical Constraints

- UI text rendering is custom and intentionally minimal
- settings and localization are still skeletal
- supported game-system launch mappings are currently hardcoded in `scripts/launch_item.sh`
- stock TrimUI emulator scripts remain the execution backend for now

## Near-Term Extension Points

- replace placeholder settings screen with a real settings UI
- move launch mappings from shell script logic into data-driven configuration
- expand per-system support beyond the first validated systems
- add richer presentation metadata and assets without moving scan logic into the UI
