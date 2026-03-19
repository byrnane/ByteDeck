# ByteDeck Architecture

This document describes the current structure after the first successful desktop build and the first successful TrimUI Smart Pro S bring-up.

## Main Layers

### `src/platform`

Responsibilities:

- resolve working roots for ROMs, BIOS, apps, collections, config, scripts and cache
- load user settings
- write runtime logs

This layer is the only place that knows how repository paths and packaged runtime paths become concrete filesystem locations.

### `src/data`

Responsibilities:

- define normalized library models
- serialize the library cache to JSON

Main entities:

- `GameItem`
- `AppItem`
- `Collection`
- `SystemEntry`
- `LibraryData`

### `src/core`

Responsibilities:

- scan ROM roots
- parse `gamelist.xml`
- merge metadata with real files
- create fallback items
- scan apps and collections
- save normalized cache

### `src/ui`

Responsibilities:

- render screens with SDL2
- map input to navigation actions
- keep screen-specific navigation logic local to each screen

### `src/launch`

Responsibilities:

- provide one launch interface for games and apps
- map ByteDeck system ids to stock TrimUI emulator scripts
- defer launch until SDL is fully shut down on device

This is the boundary between ByteDeck and stock firmware runtime.

### `src/app`

Responsibilities:

- initialize subsystems
- load library data
- own SDL window and renderer
- dispatch events to the current screen
- coordinate delayed handoff to emulator launch scripts

## Repository Zones

### Tracked source

- `src/`
- `scripts/`
- `config/`
- `device/trimui_sps/`
- `docs/`
- `cmake/`

### Local-only assets

- `local/references/`
- `local/sdk/trimui_sps/`

### Generated outputs

- `out/` for internal build output and runtime cache
- `dist/` for ready-to-use results

## Build And Packaging Model

### Windows

- build tree: `out/host/<Config>/`
- ready package: `dist/windows/`
- runtime cache/logs for dev runs: `out/runtime/`

### TrimUI Smart Pro S

- build tree: `out/trimui_sps/<Config>/`
- ready SD overlay: `dist/trimui_sps/`
- package template: `device/trimui_sps/package-root/`

## Runtime Flow

### Startup

1. `main.cpp` creates `Application`.
2. `Application::initialize()` resolves paths and starts logging.
3. `LibraryScanner` builds `LibraryData`.
4. Cache is written to the resolved cache root.
5. SDL window, renderer and input devices are initialized.
6. `MainMenuScreen` is pushed into the screen manager.

### Navigation

1. SDL events become `NavigationInput`.
2. The current screen returns a `ScreenAction`.
3. `Application` applies the action and changes screen stack or prepares a launch request.

### Game Launch

1. The selected game is passed into `LaunchService`.
2. `LaunchService` resolves the stock launch script and target path.
3. On desktop mock mode it only logs.
4. On device execute mode it returns a deferred launch request.
5. `Application` exits the main loop and shuts down SDL.
6. After shutdown, `LaunchService` hands control to the stock launcher script.

That delayed handoff is required so RetroArch-based launchers do not fail on framebuffer or video initialization.
