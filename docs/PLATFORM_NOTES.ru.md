# Заметки по платформе TrimUI SPS

- [English version](./PLATFORM_NOTES.md)
- [README проекта](../README.ru.md)
- [Архитектура](./ARCHITECTURE.ru.md)
- [Упаковка для TrimUI SPS](../device/trimui_sps/README.ru.md)

## Целевая платформа

Сейчас ByteDeck нацелен на **TrimUI Smart Pro S** со стоковой прошивкой.

Идентификатор устройства в репозитории:

```text
trimui_sps
```

Платформенный id из официальных TrimUI assets и SDK:

```text
tg5050
```

Архитектура:

```text
aarch64
```

## Модель работы с SD-картой

Подтверждённый путь до SD-карты на устройстве:

```text
/mnt/SDCARD
```

Штатный формат приложения:

```text
Apps/<AppName>/config.json
Apps/<AppName>/launch.sh
```

ByteDeck разворачивается как:

```text
/mnt/SDCARD/Apps/ByteDeck
```

## SDK и сборка

Основной путь до SDK в этом репозитории:

```text
local/sdk/trimui_sps
```

Точки входа для сборки:

- `scripts/build-trimui_sps.ps1`
- `scripts/_build-trimui_sps-wsl.sh`
- `cmake/toolchains/trimui_sps-aarch64-linux-gnu.cmake`

## Ввод

На железе для ByteDeck надёжнее всего работает SDL joystick backend.

Текущая device-обёртка выставляет:

```text
BYTEDECK_INPUT_BACKEND=joystick
```

## Передача управления эмулятору

ByteDeck не запускает эмуляторы напрямую.

Он передаёт управление штатным скриптам из:

```text
/mnt/SDCARD/Emus/<System>/launch.sh
```

Подтверждённые соответствия:

- `nes -> Emus/FC/launch.sh`
- `snes -> Emus/SFC/launch.sh`
- `megadrive -> Emus/MD/launch.sh`
- `psp -> Emus/PPSSPP/launch.sh`

Важный момент:

- перед handoff ByteDeck должен полностью завершить SDL
- иначе RetroArch-обёртки могут упасть на инициализации видео

## Структура ROM-корня

Стоковая прошивка даёт такие корни:

- `Apps/`
- `Emus/`
- `RetroArch/`
- `Roms/`

ByteDeck использует стоковые названия только для интеграции с прошивкой.

Внутри корня с ROM-файлами ByteDeck сохраняет свои названия систем:

- `Roms/nes`
- `Roms/snes`
- `Roms/megadrive`
- `Roms/psp`
