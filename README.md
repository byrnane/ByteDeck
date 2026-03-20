# ByteDeck

ByteDeck is a lightweight custom launcher for **TrimUI Smart Pro S**.

It runs on top of the stock firmware, scans ROM folders, reads `gamelist.xml`, builds its own library cache and launches games through the stock emulator scripts.

- [Русская версия](./README.ru.md)
- [Theming Guide](./docs/THEMING.md)
- [Architecture](./docs/ARCHITECTURE.md)
- [Platform Notes](./docs/PLATFORM_NOTES.md)
- [TrimUI SPS Packaging](./device/trimui_sps/README.md)

## What ByteDeck Does

- scans ROM folders from the SD card
- merges ROM files with `gamelist.xml`
- shows systems, games and apps in its own UI
- launches games through the stock TrimUI emulator wrappers
- supports JSON-defined layouts and themes

## For Users

### Install On TrimUI Smart Pro S

1. Prepare a stock SD card layout.
2. Build the TrimUI package:

```bat
ByteDeck.bat build-trimui_sps
```

3. Copy the generated app folder:

```text
dist/trimui_sps/Apps/ByteDeck -> SDCARD/Apps/ByteDeck
```

4. Insert the SD card into the console.
5. Open `Apps` in the stock launcher and start `ByteDeck`.

### ROM Folder Layout

ByteDeck keeps its own system naming inside the ROM root. Typical folders look like this:

```text
SDCARD/Roms/nes
SDCARD/Roms/snes
SDCARD/Roms/megadrive
SDCARD/Roms/psp
```

If a system uses `gamelist.xml`, place it next to the ROM files inside that system folder.

### Controls

- D-Pad or arrow keys: move selection
- `A` / `Enter`: select
- `B` / `Escape`: back
- `Menu` / `Q`: quit

## For Developers

### Windows Build

```bat
ByteDeck.bat build-windows
```

Ready package:

```text
dist/windows
```

### Windows Build And Run

```bat
ByteDeck.bat dev-windows
```

### TrimUI Smart Pro S Build

Requirements:

- WSL with Ubuntu
- `cmake` and `ninja-build` installed inside WSL
- official SDK extracted to `local/sdk/trimui_sps`

Build:

```bat
ByteDeck.bat build-trimui_sps
```

Ready package:

```text
dist/trimui_sps
```

### Cleanup

```bat
ByteDeck.bat clean
```

## Project Docs

- [Theming Guide](./docs/THEMING.md)
- [Theming Guide (Russian)](./docs/THEMING.ru.md)
- [Architecture](./docs/ARCHITECTURE.md)
- [Architecture (Russian)](./docs/ARCHITECTURE.ru.md)
- [Platform Notes](./docs/PLATFORM_NOTES.md)
- [Platform Notes (Russian)](./docs/PLATFORM_NOTES.ru.md)
- [TrimUI SPS Packaging](./device/trimui_sps/README.md)
- [TrimUI SPS Packaging (Russian)](./device/trimui_sps/README.ru.md)
- [Roadmap](./TODO.md)
