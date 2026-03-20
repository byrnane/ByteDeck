# Архитектура ByteDeck

- [English version](./ARCHITECTURE.md)
- [README проекта](../README.ru.md)
- [Гайд по темам](./THEMING.ru.md)
- [Заметки по платформе](./PLATFORM_NOTES.ru.md)

## Основные слои

### `src/platform`

Отвечает за:

- вычисление рабочих путей
- загрузку пользовательских настроек
- запись логов

Именно этот слой знает, где лежат конфиги, ROM'ы, приложения, скрипты и кеш в desktop-режиме и на устройстве.

### `src/data`

Отвечает за:

- нормализованные модели библиотеки
- сериализацию кеша библиотеки

Основные сущности:

- `GameItem`
- `AppItem`
- `Collection`
- `SystemEntry`
- `LibraryData`

### `src/core`

Отвечает за:

- сканирование ROM-каталогов
- парсинг `gamelist.xml`
- объединение метаданных и реальных файлов
- создание fallback-элементов
- сканирование приложений и коллекций
- сохранение нормализованного кеша

### `src/ui`

Отвечает за:

- перевод SDL-ввода в действия навигации
- локальную логику каждого экрана
- загрузку декларативных layout-файлов
- загрузку тем
- разрешение bindings, ассетов и стилей
- финальный рендер через SDL

Ключевые части:

- `LayoutRegistry`
- `ThemeManager`
- `ThemeFontRenderer`
- `UiRenderer`

### `src/launch`

Отвечает за:

- единый интерфейс запуска игр и приложений
- соответствие систем ByteDeck и штатных TrimUI-скриптов
- отложенный handoff после полного завершения SDL на устройстве

### `src/app`

Отвечает за:

- инициализацию подсистем
- загрузку библиотеки
- создание SDL-окна и renderer’а
- управление стеком экранов и UI runtime
- координацию запуска эмуляторов

## Декларативный UI

У ByteDeck code-first логика экранов и декларативный слой рендера.

Экран по-прежнему отвечает за:

- навигацию
- состояние выбора
- callbacks запуска
- подготовку bindings

Но экран больше не рисует интерфейс вручную. Вместо этого он отдаёт:

- `screen_id()`
- `build_bindings()`
- `window_title()`

Пайплайн рендера:

```text
layout + bindings + theme -> SDL draw calls
```

## Layout-файлы

Layout’ы лежат здесь:

```text
config/ui/screens/
```

Сейчас поддерживаются узлы:

- `screen`
- `panel`
- `stack`
- `text`
- `image`
- `list`
- `rect`
- `spacer`

## Темы

Темы лежат здесь:

```text
config/themes/<theme-id>/
```

Точка входа:

```text
config/themes/<theme-id>/theme.json
```

Сейчас тема управляет:

- цветами
- отступами
- типографическими ролями
- реальными файлами шрифтов с bitmap fallback
- фоновыми изображениями
- иконками систем
- style rules по type, class и id
- заранее подготовленными вариантами экранов

Ассеты темы всегда резолвятся относительно папки темы.

## Рендер текста

Сейчас есть два пути:

1. шрифты темы из TTF или OTF
2. встроенный bitmap-font как fallback

За счёт этого тема может использовать кастомные шрифты, но приложение остаётся работоспособным даже без них.

## Зоны репозитория

Отслеживаемые файлы проекта:

- `src/`
- `scripts/`
- `config/`
- `device/trimui_sps/`
- `docs/`
- `cmake/`

Локальные данные:

- `local/sdk/trimui_sps/`
- `local/references/`

Генерируемые артефакты:

- `out/` для внутренней сборки и runtime cache
- `dist/` для готовых пакетов

## Почему `cmake/toolchains` и `local/sdk` разделены

`cmake/toolchains/` содержит tracked-описания для CMake.

`local/sdk/trimui_sps/` содержит реальный внешний SDK и sysroot от TrimUI.

Это связанные вещи, но разного типа:

- `cmake/toolchains` — исходники сборочной системы
- `local/sdk` — внешняя зависимость

## Выходные каталоги сборки

Windows:

- build tree: `out/host/<Config>/`
- готовый пакет: `dist/windows/`

TrimUI Smart Pro S:

- build tree: `out/trimui_sps/<Config>/`
- готовый SD overlay: `dist/trimui_sps/`

Desktop runtime cache и логи:

- `out/runtime/`

## Runtime flow

Запуск:

1. `main.cpp` создаёт `Application`
2. `Application::initialize()` находит пути и запускает логирование
3. из пользовательских настроек выбирается активная тема
4. `LayoutRegistry` загружает screen JSON
5. `ThemeManager` загружает тему
6. `LibraryScanner` строит `LibraryData`
7. кеш записывается в cache root
8. инициализируются SDL-окно, renderer и ввод
9. `UiRenderer` рисует активный экран

Запуск игры:

1. экран запрашивает запуск
2. `LaunchService` находит штатный launch script
3. на устройстве `Application` сначала завершает SDL loop
4. только после shutdown ByteDeck передаёт управление штатному скрипту

Такой отложенный handoff нужен, чтобы RetroArch-обёртки на устройстве корректно поднимали видеоинициализацию.
