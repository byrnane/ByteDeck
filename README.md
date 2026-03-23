# ByteDeck

[English](#en) | [Русский](#ru)

<a id="en"></a>
## English

ByteDeck is a lightweight custom launcher for **TrimUI Smart Pro S**.

It runs on top of the stock firmware, scans ROM folders, reads `gamelist.xml`, builds its own library cache and launches games through the stock emulator scripts.

### What ByteDeck Does

- scans ROM folders from the SD card
- merges ROM files with `gamelist.xml`
- shows systems, games and apps in its own UI
- launches games through the stock TrimUI emulator wrappers
- uses fixed screen templates with theme-controlled presentation

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

### Development

Windows build:

```bat
ByteDeck.bat build-windows
```

Windows build and run:

```bat
ByteDeck.bat dev-windows
```

Windows run without rebuild:

```bat
ByteDeck.bat run-windows
```

TrimUI Smart Pro S build:

- WSL with Ubuntu
- `cmake` and `ninja-build` installed inside WSL
- official SDK extracted to `local/sdk/trimui_sps`

```bat
ByteDeck.bat build-trimui_sps
```

Cleanup:

```bat
ByteDeck.bat clean
```

### Project Docs

- [Theming Guide](./docs/THEMING.md)
- [Theming Guide (Russian)](./docs/THEMING.ru.md)
- [Architecture](./docs/ARCHITECTURE.md)
- [Architecture (Russian)](./docs/ARCHITECTURE.ru.md)
- [Platform Notes](./docs/PLATFORM_NOTES.md)
- [Platform Notes (Russian)](./docs/PLATFORM_NOTES.ru.md)
- [TrimUI SPS Packaging](./device/trimui_sps/README.md)
- [TrimUI SPS Packaging (Russian)](./device/trimui_sps/README.ru.md)
- [UI Reference HTML](./ui_reference/index.html)
- [Roadmap](./TODO.md)

<a id="ru"></a>
## Русский

ByteDeck — это лёгкий кастомный лаунчер для **TrimUI Smart Pro S**.

Он работает поверх стоковой прошивки, сканирует папки с ROM-файлами, читает `gamelist.xml`, строит собственный кеш библиотеки и запускает игры через штатные скрипты эмуляторов.

### Что умеет ByteDeck

- сканировать ROM-папки с SD-карты
- объединять реальные ROM-файлы и данные из `gamelist.xml`
- показывать системы, игры и приложения в собственном интерфейсе
- запускать игры через штатные TrimUI-обёртки
- использовать фиксированные шаблоны экранов и темы, которые управляют оформлением

### Установка на TrimUI Smart Pro S

1. Подготовь SD-карту со стоковой структурой.
2. Собери пакет для консоли:

```bat
ByteDeck.bat build-trimui_sps
```

3. Скопируй готовую папку приложения:

```text
dist/trimui_sps/Apps/ByteDeck -> SDCARD/Apps/ByteDeck
```

4. Вставь SD-карту в консоль.
5. Открой раздел `Apps` в штатном лаунчере и запусти `ByteDeck`.

### Где должны лежать ROM'ы

ByteDeck использует свои названия систем внутри корня с ROM-файлами. Обычно структура такая:

```text
SDCARD/Roms/nes
SDCARD/Roms/snes
SDCARD/Roms/megadrive
SDCARD/Roms/psp
```

Если для системы используется `gamelist.xml`, положи его рядом с ROM-файлами в папке этой системы.

### Управление

- крестовина или стрелки: перемещение
- `A` / `Enter`: выбрать
- `B` / `Escape`: назад
- `Menu` / `Q`: выход

### Разработка

Сборка под Windows:

```bat
ByteDeck.bat build-windows
```

Сборка и запуск под Windows:

```bat
ByteDeck.bat dev-windows
```

Запуск Windows-сборки без пересборки:

```bat
ByteDeck.bat run-windows
```

Сборка под TrimUI Smart Pro S:

- WSL с Ubuntu
- `cmake` и `ninja-build` внутри WSL
- официальный SDK, распакованный в `local/sdk/trimui_sps`

```bat
ByteDeck.bat build-trimui_sps
```

Очистка:

```bat
ByteDeck.bat clean
```

### Документация

- [Гайд по темам](./docs/THEMING.ru.md)
- [Theming Guide](./docs/THEMING.md)
- [Архитектура](./docs/ARCHITECTURE.ru.md)
- [Architecture](./docs/ARCHITECTURE.md)
- [Заметки по платформе](./docs/PLATFORM_NOTES.ru.md)
- [Platform Notes](./docs/PLATFORM_NOTES.md)
- [Упаковка для TrimUI SPS](./device/trimui_sps/README.ru.md)
- [TrimUI SPS Packaging](./device/trimui_sps/README.md)
- [HTML-референс интерфейса](./ui_reference/index.html)
- [План работ](./TODO.md)
