# ByteDeck

ByteDeck - это легковесный кастомный лаунчер для **TrimUI Smart Pro S**.

Он работает поверх стоковой прошивки, сканирует ROM-папки, читает `gamelist.xml`, строит собственный кеш библиотеки и запускает игры через штатные скрипты эмуляторов.

- [English version](./README.md)
- [Гайд по темам](./docs/THEMING.ru.md)
- [Архитектура](./docs/ARCHITECTURE.ru.md)
- [Заметки по платформе](./docs/PLATFORM_NOTES.ru.md)
- [Упаковка для TrimUI SPS](./device/trimui_sps/README.ru.md)

## Что умеет ByteDeck

- сканировать ROM-папки с SD-карты
- объединять реальные ROM-файлы и данные из `gamelist.xml`
- показывать системы, игры и приложения в собственном интерфейсе
- запускать игры через штатные TrimUI-обёртки
- использовать JSON-описания экранов и тем

## Для пользователей

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

## Для разработки

### Сборка под Windows

```bat
ByteDeck.bat build-windows
```

Готовый пакет:

```text
dist/windows
```

### Сборка и запуск под Windows

```bat
ByteDeck.bat dev-windows
```

### Сборка под TrimUI Smart Pro S

Что нужно:

- WSL с Ubuntu
- `cmake` и `ninja-build` внутри WSL
- официальный SDK, распакованный в `local/sdk/trimui_sps`

Сборка:

```bat
ByteDeck.bat build-trimui_sps
```

Готовый пакет:

```text
dist/trimui_sps
```

### Очистка

```bat
ByteDeck.bat clean
```

## Документация

- [Гайд по темам](./docs/THEMING.ru.md)
- [Theming Guide](./docs/THEMING.md)
- [Архитектура](./docs/ARCHITECTURE.ru.md)
- [Architecture](./docs/ARCHITECTURE.md)
- [Заметки по платформе](./docs/PLATFORM_NOTES.ru.md)
- [Platform Notes](./docs/PLATFORM_NOTES.md)
- [Упаковка для TrimUI SPS](./device/trimui_sps/README.ru.md)
- [TrimUI SPS Packaging](./device/trimui_sps/README.md)
- [План работ](./TODO.md)
