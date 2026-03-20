# Упаковка для TrimUI SPS

- [English version](./README.md)
- [README проекта](../../README.ru.md)
- [Заметки по платформе](../../docs/PLATFORM_NOTES.ru.md)

## Назначение

В этой папке лежит tracked-шаблон упаковки для **TrimUI Smart Pro S** со стоковой прошивкой.

ByteDeck упаковывается как обычное приложение стоковой системы:

```text
Apps/ByteDeck/
```

## Отслеживаемые файлы пакета

- `device/trimui_sps/package-root/Apps/ByteDeck/config.json`
- `device/trimui_sps/package-root/Apps/ByteDeck/launch.sh`

## Результат сборки

Основная команда сборки:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build-trimui_sps.ps1 -Clean
```

Выходные каталоги:

- промежуточная ARM-сборка: `out/trimui_sps/Release/`
- готовый SD overlay: `dist/trimui_sps/`

Копирование на карту:

```text
dist/trimui_sps/Apps/ByteDeck -> SDCARD/Apps/ByteDeck
```

## Что делает `launch.sh`

Упакованный `launch.sh`:

- вычисляет корни SD-карты
- выставляет `BYTEDECK_*` path overrides
- включает device execute mode
- принудительно выбирает joystick backend
- настраивает `LD_LIBRARY_PATH`
- запускает `bin/bytedeck`

## Предположения runtime

- стоковая прошивка
- SD-карта смонтирована как `/mnt/SDCARD`
- штатные emulator scripts доступны в `Emus/`
- корень с ROM-файлами передаётся через `BYTEDECK_ROMS_ROOT`
