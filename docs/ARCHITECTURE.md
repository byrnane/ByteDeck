# ByteDeck Architecture

- [Русская версия](./ARCHITECTURE.ru.md)
- [Project README](../README.md#en)
- [Theming Guide](./THEMING.md)
- [Platform Notes](./PLATFORM_NOTES.md)
- [UI Reference HTML](../ui_reference/index.html)

## Overview

ByteDeck now uses a **fixed-template UI renderer**.

The runtime no longer treats `config/ui/screens/*.json` as the active source of truth for layout. Screen geometry is defined in code through a fixed app shell and a small set of screen renderers. Themes control presentation only.

The high-level model is:

```text
screen state + bindings + theme -> fixed screen renderer -> SDL draw calls
```

## Main Layers

### `src/platform`

Responsibilities:

- resolve working paths
- load user settings
- load translations
- read status information such as time and battery
- write runtime logs

### `src/data`

Responsibilities:

- normalized library models
- library cache serialization

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
- load themes and resolve theme values
- load fonts and images
- render the fixed app shell and screen templates through SDL

Important parts:

- `FixedUiRenderer`
- `ThemeManager`
- `ThemeFontRenderer`
- screen classes in `src/ui/screens/`

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
- own screen stack and theme runtime
- build shell bindings
- coordinate delayed emulator handoff

## UI Runtime Contract

## App Shell

Every screen uses the same shell:

- header at the top
- footer at the bottom
- content viewport in the middle

The shell is rendered by `FixedUiRenderer` before any screen-specific content.

Header content:

- brand
- current context path
- time
- battery

Footer content:

- contextual actions
- short hint text

## Fixed Screen Templates

The first generation of the fixed renderer includes:

- `main_menu`
- `games`
- `game_browser`
- `apps`
- `settings`
- `placeholder`

Each of these screens has a fixed composition. Themes may change colors, fonts, spacing, borders, icons and background images, but not the structural hierarchy of the screen.

## Screen Logic

Screen classes remain code-first. They still own:

- navigation
- selection state
- drill-in state
- launch requests
- translated strings
- bindings preparation

Each screen provides:

- `screen_id()`
- `build_bindings()`
- `window_title()`

Bindings no longer feed a generic layout tree. They feed a dedicated renderer for a known screen template.

## Theming Model

Themes live under:

```text
themes/<theme-id>/
```

Entrypoint:

```text
themes/<theme-id>/theme.json
```

Theme assets are always resolved relative to the theme folder.

The active schema is presentation-oriented. It controls:

- color tokens
- spacing tokens
- fonts
- typography roles
- shell styles
- shared component styles
- per-screen style sections
- system icons

The active default theme uses this structure:

```text
tokens
fonts
typography
system_icons
shell
components
screens
```

## HTML Reference Authoring

The visual contract for the new UI lives here:

```text
ui_reference/index.html
ui_reference/screens/*.html
```

These files are not used at runtime. They exist to:

- define the intended composition of each screen
- document which parts are built into the renderer
- document which parts are themeable
- provide a simple JavaScript mock for navigation states

Class naming in the HTML reference follows two namespaces:

- `bd-builtin-*` for structural parts owned by the renderer
- `bd-theme-*` for presentation parts that should be expressible in `theme.json`

## Legacy Layout Files

The old files under:

```text
config/ui/screens/
```

are kept only as reference during the transition. They are no longer part of the active runtime path.

The old `UiRenderer` and `LayoutRegistry` are also legacy reference code and are no longer built into the main application.

## Text And Images

### Text

ByteDeck supports two text paths:

1. theme fonts loaded from TTF or OTF files
2. built-in bitmap fallback

This keeps the UI themeable while still working if a custom font fails to load.

### Images

The renderer supports:

- theme assets
- system icons
- preview images
- placeholder rendering when an image is missing

Images are cached in the renderer and drawn with simple rect-based placement.

## Repository Zones

Tracked project files:

- `src/`
- `scripts/`
- `config/`
- `themes/`
- `device/trimui_sps/`
- `docs/`
- `cmake/`

Local-only data:

- `local/sdk/trimui_sps/`
- `local/references/`

Generated output:

- `out/` for internal build output and runtime cache
- `dist/` for ready-to-use packages

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
3. user settings select the active theme and language
4. `ThemeManager` loads the active theme
5. `TranslationCatalog` loads translations
6. `LibraryScanner` builds `LibraryData`
7. cache is written to the cache root
8. SDL window, renderer and input are initialized
9. `FixedUiRenderer` renders the shell and the active screen template

Game launch:

1. a screen requests a launch
2. `LaunchService` resolves the stock launcher script
3. on device, `Application` exits the SDL loop first
4. after SDL shutdown, ByteDeck hands off to the stock script

That delayed handoff is required so RetroArch-based launchers can initialize video correctly on the device.
