# TrimUI Smart Pro S Custom Launcher

## Goal

Build a lightweight custom launcher for TrimUI Smart Pro S that runs on top of the stock Linux system and is deployed entirely from the SD card. The launcher provides a unified UI for games, collections and apps.

The launcher must:

- work without replacing the stock firmware
- run entirely from SD card
- support RetroArch and PPSSPP
- use Batocera / EmulationStation style ROM directory structure
- read metadata from gamelist.xml

## Supported Systems (v1)

Game systems:

- NES (RetroArch)
- SNES (RetroArch)
- Mega Drive (RetroArch)
- PSP (PPSSPP standalone)

Non‑game categories:

- Apps

## Main Menu

Main menu entries:

- Games
- Settings
- Apps

Games menu contains:

- NES
- SNES
- Mega Drive
- PSP
- Collections

Important rule: Systems are shown in the UI only if they contain at least one valid item.

For game systems this means at least one ROM file exists. For Apps this means at least one valid app manifest exists. Empty systems must not appear in the UI.

## Directory Structure

Expected SD card layout:

```
SDCARD/
  MyLauncher/
    bin/
    config/
    assets/
    scripts/
    cache/

  ROMS/
    nes/
    snes/
    megadrive/
    psp/

  BIOS/

  Apps/

  collections/
```

## ROM Structure

Each system has its own folder:

```
ROMS/<system_id>/
```

Example:

```
ROMS/nes/
  Contra.nes
  Super Mario Bros.nes
  gamelist.xml

ROMS/snes/
  Chrono Trigger.sfc
  gamelist.xml
```

### gamelist.xml

Launcher reads metadata from gamelist.xml.

Supported fields:

- path
- name
- desc
- thumbnail
- image
- video
- rating
- releasedate
- developer
- publisher
- genre
- players

For v1 UI uses only **thumbnail**.

### Behavior

ROM file scanning is the source of truth for whether a game exists.

System visibility rule:

- a game system is shown only if at least one valid ROM file is found in its directory
- `gamelist.xml` does not make a system visible by itself
- metadata entries that point to missing files must be ignored or marked broken, but must not count as valid games

Library build order:

1. scan ROM files on disk using supported extensions
2. if at least one ROM exists, the system is considered non-empty and can appear in the UI
3. if `gamelist.xml` exists, read metadata and match entries to real files
4. create fallback entries for ROM files that are missing from `gamelist.xml`
5. if `gamelist.xml` is missing or invalid, build entries from filenames only

If `gamelist.xml` exists:

- read metadata
- verify ROM files against real files on disk

If `gamelist.xml` is missing:

- fallback to scanning ROM files
- create entries using filename

Launcher must tolerate:

- missing fields
- invalid XML
- missing media

## Supported File Extensions

NES:

- .nes
- .zip

SNES:

- .sfc
- .smc
- .zip

Mega Drive:

- .md
- .bin
- .gen
- .zip

PSP:

- .iso
- .cso

Launcher does **not unpack archives**.

## BIOS Directory

Some systems require BIOS files.

All BIOS files are placed in:

```
BIOS/
```

Launcher does not validate BIOS files in v1 but must expose this directory to emulators.

## Emulator Architecture

Launcher does not contain emulator logic.

Instead it calls adapter scripts.

Example call:

```
launch_item.sh <system_id> <path>
```

### Backends

RetroArch:

- NES
- SNES
- Mega Drive

PPSSPP standalone:

- PSP

Adapter script resolves:

- emulator binary
- RetroArch core
- launch arguments

## RetroArch Launch Model

Adapter launches RetroArch using the configured core and ROM path.

Example concept:

```
retroarch -L <core> <rom>
```

Launcher must not embed RetroArch logic internally.

## PPSSPP Launch Model

PSP games are launched via PPSSPP standalone binary.

Adapter script calls PPSSPP with ROM path.

## Apps and Tools

Apps live in the following directory:

```
Apps/
```

Each application has its own directory:

```
Apps/Clock/
```

Each directory must contain a manifest:

```
manifest.json
```

### Manifest Structure

Example:

```
{
  "id": "clock",
  "type": "app",
  "name": "Clock",
  "description": "Simple clock utility",
  "icon": "icon.png",
  "launch": {
    "type": "script",
    "path": "run.sh"
  }
}
```

Launcher scans the Apps directory at startup.

Apps are displayed in the Apps menu only if at least one valid manifest is found.

## Collections

Collections are stored in:

```
collections/
```

Collections are JSON files referencing items from the library.

Example:

```
favorites.json
last_played.json
```

### Collection Structure

```
{
  "id": "favorites",
  "name": "Favorites",
  "icon": "star.png",
  "items": [
    "nes:contra",
    "snes:chrono_trigger"
  ]
}
```

Built‑in collections:

- favorites
- last\_played

Launcher updates these automatically.

## Library Cache

Launcher builds a unified library cache from:

- ROM directories
- `gamelist.xml`
- Apps manifests
- Collections

Cache stored in:

```
cache/library.json
```

Cache avoids rescanning filesystem on every startup.

The cache must store normalized items only after they have been validated against real files on disk.

For game systems:

- only ROM files found on disk count as valid items
- metadata from `gamelist.xml` augments existing ROMs
- fallback file-based entries must be created for ROMs without metadata

For Apps:

- only directories with a valid `manifest.json` count as valid items
- broken manifests must be skipped and logged

## UI and Design Principles

The interface should be lightweight, readable and optimized for a handheld Linux console.

Design goals:

- fast and responsive
- visually clean
- easy to navigate with buttons only
- minimal nesting
- consistent layout across screens
- good readability on a small display

Visual principles:

- dark theme by default
- strong contrast between background, selected item and metadata panel
- large readable font sizes
- clear focus state for the selected item
- restrained animations only, no heavy transitions
- use thumbnails as the main visual accent
- avoid UI clutter and decorative noise

### Main Menu Layout

Main menu shows:

- Games
- Settings
- Apps

Recommended presentation:

- tile-based layout
- large selectable tiles
- clear focused tile state
- item count for Games and Apps when useful
- no right-side preview panel in the main menu unless it is trivial to support cleanly

### Games Screen

Shows visible game systems and Collections.

Only systems with at least one valid item are shown.

Recommended presentation:

- tile-based layout
- one tile per visible system or collection
- each tile may show name, icon and item count

### Systems Screen

Shows list of systems using the same tile-based layout.

### Game Browser

Layout:

Left side:

- list of games

Right side:

- thumbnail
- title
- description
- genre
- release date
- developer
- publisher
- rating shown as stars

Behavior:

- selected item updates the right panel immediately
- title and description must be length-limited in the UI and truncated cleanly when too long
- if thumbnail is missing, show a placeholder image
- fallback filename-based entries must still render cleanly
- rating should be rendered visually as stars when metadata is available

### Apps Screen

Layout matches the game browser where possible.

Left side:

- list of apps

Right side:

- icon
- title
- description

Apps screen is shown only if at least one valid app exists.

## Navigation

Controls:

- D‑Pad / Stick: navigation
- A: select / launch
- B: back
- Start: options

Launcher must be fully usable without touchscreen.

## Launch Flow

1. User selects item
2. Launcher resolves system
3. Launcher calls adapter script
4. Emulator starts
5. After exit control returns to launcher

## Error Handling

Launcher must not crash due to:

- malformed gamelist.xml
- missing ROM files
- missing thumbnails
- broken app manifests

Errors should be logged.

Logs stored in:

```
cache/logs/
```

## Performance Requirements

- fast startup
- smooth navigation
- library caching

## Non Goals (v1)

Not implemented in first version:

- PortMaster
- scraper
- video previews
- theme system
- emulator selection per game
- network services

## Implementation Recommendation

Preferred stack:

- C++17
- SDL2
- tinyxml2

Alternative:

- Rust + SDL2 equivalent

Avoid heavy frameworks.

## Data Model

The launcher should normalize all scanned content into a single internal model.

### Game Item

Fields:

- `id`
- `type = "game"`
- `system_id`
- `rom_path`
- `title`
- `description`
- `thumbnail`
- `genre`
- `players`
- `release_date`
- `metadata_source` (`gamelist` or `fallback`)

### App Item

Fields:

- `id`
- `type = "app"`
- `system_id = "apps"`
- `launch_target`
- `title`
- `description`
- `icon`

### Collection

Fields:

- `id`
- `name`
- `icon`
- `description`
- `items` (list of normalized item ids)

### System Entry

Fields:

- `id`
- `name`
- `type` (`game_system`, `collection_group`, `app_group`)
- `item_count`
- `visible`

## Localization

The launcher should support interface localization from the start, even if only one language is shipped in v1.

Requirements:

- all UI strings must be stored in translation data, not hardcoded in UI code
- translation keys should be stable and human-readable
- default language is English
- Russian should be supported by the format from the start
- missing translation keys must fall back to English

Preferred format:

```
config/i18n/translations.csv
```

CSV columns:

- `key`
- `en`
- `ru`

Example:

```csv
key,en,ru
menu.games,Games,Игры
menu.settings,Settings,Настройки
menu.apps,Apps,Приложения
collections.favorites,Favorites,Избранное
collections.last_played,Last Played,Последние
```

The loader may internally convert CSV rows into a runtime dictionary.

The current language choice should be stored in launcher settings.

## Settings

The Settings menu for v1 should include:

- language selection
- rescan library
- about
- optional simple launcher settings if platform access is not yet available

Settings presentation:

- simple flat list
- no nested categories in v1
- About is shown as a modal dialog, not a separate page

Recommended launcher-specific settings for v1:

- UI language
- show hidden metadata fields toggle if needed later
- description scroll speed or on/off behavior
- confirm before shutdown toggle

Platform-dependent settings such as brightness, volume and Wi-Fi may be added only if discovery confirms a reliable implementation path on the stock OS.

Settings storage:

```
config/user_settings.json
```

The launcher should separate:

- user settings
- system discovery/config files
- generated cache files

## Platform Discovery Requirements

Before implementation, document the following:

- SD card mount path
- stock launcher integration points
- how external apps are launched from the system
- paths to RetroArch binary and cores
- path to PPSSPP binary
- BIOS path expected by RetroArch and PPSSPP
- return-to-launcher behavior after emulator exit
- whether SDL2 is already available or must be bundled

Findings must be written to:

```
docs/PLATFORM_NOTES.md
```

## Development Steps

1. Platform discovery
2. Launcher skeleton
3. Library scanner based on real files
4. `gamelist.xml` parser and metadata matching
5. UI screens
6. Emulator adapter integration
7. Collections support
8. Apps scanning
9. Settings menu

## Deliverables

Repository must include:

- launcher source
- build instructions
- sample configs
- example ROM structure
- documentation

