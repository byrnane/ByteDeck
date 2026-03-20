# Гайд по темам ByteDeck

- [English version](./THEMING.md)
- [README проекта](../README.ru.md)
- [Архитектура](./ARCHITECTURE.ru.md)

## Общая модель

Интерфейс ByteDeck разделён на две части:

- `config/ui/screens/*.json` описывает структуру экрана
- `config/themes/<theme-id>/theme.json` описывает внешний вид этой структуры

Layout отвечает за панели, списки, текст и изображения.
Theme отвечает за цвета, отступы, типографику, фоны и иконки.

## Корень темы

Каждая тема живёт в своей папке:

```text
config/themes/<theme-id>/
```

Точка входа:

```text
config/themes/<theme-id>/theme.json
```

Любой путь к ассету внутри `theme.json` считается относительным к этой папке темы.

Пример:

```json
"background_image": "images/main-menu.png"
```

Это значит:

```text
config/themes/<theme-id>/images/main-menu.png
```

Жёсткой внутренней структуры нет. Ты сам решаешь, как разложить `fonts/`, `icons/`, `images/` и `backgrounds/`.

## Структура `theme.json`

Основные разделы:

```json
{
  "tokens": {},
  "fonts": {},
  "typography": {},
  "system_icons": {},
  "styles": {
    "types": {},
    "classes": {},
    "ids": {}
  },
  "screen_variants": {}
}
```

## Цвета

### Основные форматы

Рекомендуемые форматы цветов:

- `#RRGGBB`
- `#RRGGBBAA`

Примеры:

```json
"primary": "#F5F1E6"
"accent_overlay": "#E4B756CC"
```

### Отдельная прозрачность

Если удобнее задавать прозрачность отдельно, используй объект:

```json
{
  "color": "#F5F1E6",
  "alpha": 75
}
```

Правила:

- `alpha` задаётся в процентах от `0` до `100`
- если указан `#RRGGBBAA`, он важнее отдельного `alpha`
- старый формат `[245, 241, 230, 255]` пока ещё поддерживается для совместимости, но больше не считается основным

## Tokens

`tokens` — это переиспользуемые значения.

Обычно там лежат:

- `colors`
- `spacing`

Пример:

```json
"tokens": {
  "colors": {
    "background": {
      "app": "#0F1218",
      "panel_primary": "#1A1F2A"
    },
    "text": {
      "primary": "#F5F1E6",
      "muted": "#939DB0"
    }
  },
  "spacing": {
    "sm": 8,
    "md": 16,
    "lg": 24
  }
}
```

Использование токена в стилях:

```json
"background_color": "$colors.background.panel_primary"
```

## Типографика и шрифты

### Как сейчас работает текст

Сейчас у ByteDeck есть два пути рендера текста:

1. реальные шрифты из TTF или OTF
2. встроенный bitmap-font как fallback

Если шрифт темы не загрузился, интерфейс не падает и откатывается на встроенный bitmap-font.

### Раздел `fonts`

Раздел `fonts` объявляет семейства шрифтов.

Пример:

```json
"fonts": {
  "ui": {
    "path": "fonts/Inter-Medium.ttf"
  },
  "brand": "fonts/Display.otf"
}
```

Поддерживаются и объект, и строка.

### Раздел `typography`

Раздел `typography` задаёт именованные текстовые роли.

Пример:

```json
"typography": {
  "body": {
    "family": "ui",
    "size": 16,
    "line_height": 20,
    "bitmap_scale": 2
  },
  "title": {
    "family": "brand",
    "size": 28,
    "bitmap_scale": 4
  }
}
```

Поля:

- `family`: семейство шрифта из `fonts`
- `size`: размер для TTF/OTF
- `line_height`: необязательная фиксированная высота строки
- `bitmap_scale`: размер fallback для встроенного bitmap-font

### Как задаётся размер текста

В текущем runtime:

- `font_role` — основной способ задать стиль текста
- `font_size` — необязательный override размера из роли
- `scale` — fallback для bitmap-font

Пример:

```json
"menu-card-title": {
  "font_role": "title",
  "text_color": "$colors.text.primary"
}
```

## Ассеты темы

Ассеты темы — это обычные файлы внутри папки темы.

Сейчас поддерживаются:

- `background_image`
- `system_icons`
- пути к изображениям из style rules
- файлы шрифтов из `fonts`

### Фоновые изображения

Пример:

```json
"panel-primary": {
  "background_color": "#1A1F2A",
  "background_image": "images/panel-noise.png"
}
```

### Иконки систем

Пример:

```json
"system_icons": {
  "nes": "icons/nes.png",
  "megadrive": "icons/megadrive.png"
}
```

Узел `image` может запросить такую иконку по `system_id` через bindings.

## Styles

Раздел `styles` делится на:

- `types`: значения по умолчанию для типа узла
- `classes`: переиспользуемые стили
- `ids`: точечные переопределения для конкретного узла

Приоритет применения:

1. runtime defaults
2. `styles.types`
3. `styles.classes`
4. `styles.ids`
5. inline-значения из layout-файла

### Часто используемые свойства

Текст:

- `text_color`
- `font_role`
- `font_size`
- `wrap`
- `truncate`

Контейнеры:

- `background_color`
- `background_image`
- `border_color`
- `border_width`
- `padding`

Изображения:

- `path`
- `placeholder_text`
- `system_icon_bind`

## Варианты экранов

Тема может выбрать заранее подготовленный вариант layout’а:

```json
"screen_variants": {
  "main_menu": "hero"
}
```

Это меняет только вариант layout’а. Логика экрана при этом не меняется.

## Безопасный порядок редактирования

Если делаешь новую тему, удобнее идти так:

1. править `tokens.colors`
2. править `tokens.spacing`
3. править `typography`
4. править `styles.classes`
5. добавлять картинки, иконки и шрифты

## Практические советы

- используй относительные пути внутри папки темы
- предпочитай `#RRGGBB` и `#RRGGBBAA`
- даже если используешь реальные шрифты, оставляй `bitmap_scale`, чтобы fallback оставался читаемым
- сначала проверяй тему на desktop-сборке, потом на устройстве

## Связанные документы

- [README проекта](../README.ru.md)
- [Архитектура](./ARCHITECTURE.ru.md)
- [Заметки по платформе](./PLATFORM_NOTES.ru.md)
- [Упаковка для TrimUI SPS](../device/trimui_sps/README.ru.md)
