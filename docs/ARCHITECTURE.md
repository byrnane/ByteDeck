# ByteDeck Architecture

- [Русская версия](./ARCHITECTURE.ru.md)
- [Project README](../README.md#en)
- [Theming Guide](./THEMING.md)
- [Platform Notes](./PLATFORM_NOTES.md)

## Main Layers

### `src/platform`

Responsibilities:

- resolve concrete working paths
- load user settings
- write runtime logs

This layer knows where config, ROMs, apps, scripts and cache live in desktop and device modes.

### `src/data`

Responsibilities:

- define normalized library models
- serialize the library cache

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

- map SDL input to navigation actions
- keep screen-specific logic inside screen classes
- load declarative layouts from JSON
- load themes from JSON
- resolve bindings, assets and styles
- render the final UI through SDL

Important parts:

- `LayoutRegistry`
- `ThemeManager`
- `ThemeFontRenderer`
- `UiRenderer`
- `SettingsScreen`

### `src/launch`

Responsibilities:

- expose one launch interface for games and apps
- map ByteDeck system ids to stock TrimUI launch scripts
- defer handoff until SDL is fully shut down on device

### `src/app`

Responsibilities:

- initialize subsystems
- load library data
- create the SDL window and renderer
- own screen stack and UI runtime
- coordinate delayed emulator handoff

## Declarative UI Model

ByteDeck uses a code-first screen logic layer with a declarative render layer.

Screen classes still handle:

- navigation
- selection state
- launch callbacks
- bindings preparation

Screen classes no longer draw UI directly. Instead they provide:

- `screen_id()`
- `build_bindings()`
- `window_title()`

The render pipeline is:

```text
layout + bindings + theme -> SDL draw calls
```

## Layouts

Layouts live under:

```text
config/ui/screens/
```

The current node set is:

- `screen`
- `panel`
- `stack`
- `text`
- `image`
- `list`
- `rect`
- `spacer`

## Themes

Themes live under:

```text
themes/<theme-id>/
```

Theme entrypoint:

```text
themes/<theme-id>/theme.json
```

Themes currently control:

- colors
- spacing
- typography roles
- real font files with bitmap fallback
- background images
- system icons
- status bar styling
- style rules by type, class and id
- predefined screen variants

Theme assets are resolved relative to the theme folder.

## Text Rendering

ByteDeck currently supports two text paths:

1. theme fonts loaded from TTF or OTF files
2. built-in bitmap font fallback

This keeps the UI editable through themes without making custom fonts a hard runtime requirement.

## Repository Zones

Tracked project files:

- `src/`
- `scripts/`
- `config/`
- `device/trimui_sps/`
- `docs/`
- `cmake/`

Local-only data:

- `local/sdk/trimui_sps/`
- `local/references/`

Generated output:

- `out/` for internal build output and runtime cache
- `dist/` for ready-to-use packages

## Why `cmake/toolchains` And `local/sdk` Are Separate

`cmake/toolchains/` contains tracked build definitions used by CMake.

`local/sdk/trimui_sps/` contains the real external SDK and sysroot downloaded from TrimUI.

They are related, but they are not the same kind of data:

- `cmake/toolchains` is source code for the build system
- `local/sdk` is an external dependency

## Build Outputs

Windows:

- build tree: `out/host/<Config>/`
- ready package: `dist/windows/`

TrimUI Smart Pro S:

- build tree: `out/trimui_sps/<Config>/`
- ready SD overlay: `dist/trimui_sps/`

Desktop runtime cache and logs:

- `out/runtime/`

## Runtime Flow

Startup:

1. `main.cpp` creates `Application`.
2. `Application::initialize()` resolves paths and starts logging.
3. user settings select the active theme
4. `LayoutRegistry` loads screen JSON files
5. `ThemeManager` loads the active theme
6. `LibraryScanner` builds `LibraryData`
7. cache is written to the cache root
8. SDL window, renderer and input are initialized
9. `UiRenderer` renders the active screen

Game launch:

1. a screen requests a launch
2. `LaunchService` resolves the stock launcher script
3. on device, `Application` exits the SDL loop first
4. after SDL shutdown, ByteDeck hands off to the stock script

That delayed handoff is required so RetroArch-based launchers can initialize video correctly on the device.
