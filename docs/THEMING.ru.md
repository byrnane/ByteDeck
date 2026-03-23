# Гайд по темам ByteDeck

- [English version](./THEMING.md)
- [README проекта](../README.md#ru)
- [Архитектура](./ARCHITECTURE.ru.md)
- [HTML-референс интерфейса](../ui_reference/index.html)

## Общая модель

Темы ByteDeck больше не описывают структуру экранов.

Активная модель интерфейса такая:

- структура экранов зашита в коде
- темы управляют оформлением
- HTML-референс фиксирует целевую композицию и themeable-части

То есть тема может менять внешний вид лаунчера, но не может перестраивать саму структуру экранов.

## Корень темы

Каждая тема живёт в своей папке:

```text
themes/<theme-id>/
```

Точка входа:

```text
themes/<theme-id>/theme.json
```

Любой путь к asset внутри `theme.json` считается относительным к этой папке темы.

Пример:

```json
"background_image": "assets/backgrounds/main-menu.png"
```

Это значит:

```text
themes/<theme-id>/assets/backgrounds/main-menu.png
```

Жёсткой внутренней структуры нет. Обязателен только `theme.json` как entrypoint.

## Актуальная схема `theme.json`

Текущий fixed-template renderer ожидает такую верхнеуровневую структуру:

```json
{
  "tokens": {},
  "fonts": {},
  "typography": {},
  "system_icons": {},
  "shell": {},
  "components": {},
  "screens": {}
}
```

### `tokens`

Переиспользуемые значения, например:

- `colors`
- `spacing`

### `fonts`

Семейства шрифтов, которые ссылаются на относительные пути к файлам.

Пример:

```json
"fonts": {
  "noto_sans": {
    "path": "assets/fonts/NotoSans-Regular.ttf"
  },
  "superstar": {
    "path": "assets/fonts/superstar.ttf"
  }
}
```

### `typography`

Именованные текстовые роли, которые использует renderer.

Пример:

```json
"typography": {
  "body": {
    "family": "noto_sans",
    "size": 24,
    "line_height": 30,
    "bitmap_scale": 3
  },
  "hero": {
    "family": "superstar",
    "size": 42,
    "line_height": 48,
    "bitmap_scale": 5
  }
}
```

Поля:

- `family`
- `size`
- `line_height`
- `bitmap_scale`

### `system_icons`

Связывает system id ByteDeck с файлами иконок:

```json
"system_icons": {
  "nes": "assets/icons/systems/fc.png",
  "megadrive": "assets/icons/systems/md.png"
}
```

### `shell`

Описывает общие метрики и стили shell:

- `metrics`
- `root`
- `header`
- `header_brand`
- `header_context`
- `header_meta`
- `footer`
- `footer_action`
- `footer_hint`

### `components`

Общие небольшие presentation-блоки, например:

- placeholder для изображений

### `screens`

Секции со стилями конкретных fixed templates:

- `main_menu`
- `games`
- `browser`
- `apps`
- `settings`
- `placeholder`

Внутри секции экрана могут быть:

- `metrics`
- стили блоков
- стили текста
- стили изображений

## Какие свойства поддерживаются в Theme v1

Вот что fixed renderer рассчитан поддерживать прямо сейчас:

- `background_color`
- `text_color`
- `border_color`
- `border_width`
- `background_image`
- `font_role`
- `font_size`
- `line_height` через typography roles
- `padding`
- `gap`
- `width`
- `height`
- `opacity` через alpha-канал цвета или `alpha` в объекте
- `wrap`
- `truncate`

Для изображений:

- обычные пути к asset
- system icons
- placeholder text

## Форматы цветов

Рекомендуемые форматы:

- `#RRGGBB`
- `#RRGGBBAA`

Примеры:

```json
"text_primary": "#F5F1E6"
"overlay": "#E4B756CC"
```

Можно задавать прозрачность и отдельно:

```json
{
  "color": "#F5F1E6",
  "alpha": 75
}
```

Правила:

- `alpha` задаётся в процентах от `0` до `100`
- `#RRGGBBAA` важнее отдельного `alpha`
- legacy-массивы вроде `[245, 241, 230, 255]` ещё поддерживаются для совместимости, но больше не считаются основным форматом

## Шрифты и рендер текста

У ByteDeck два пути рендера текста:

1. theme fonts из TTF/OTF
2. встроенный bitmap fallback

Если шрифт темы не загрузился, ByteDeck откатывается на встроенный bitmap-font и интерфейс остаётся рабочим.

Размер текста задаётся через:

- `font_role`
- `font_size`, если нужен override
- `bitmap_scale` как fallback-размер для bitmap-font

## Assets темы

Assets темы — это обычные файлы внутри папки темы.

Сейчас поддерживаются:

- шрифты
- фоновые изображения
- system icons
- другие UI-картинки, на которые ссылаются style-секции

Все asset paths считаются относительными к корню активной темы, если явно не указан абсолютный путь.

## Что темы не контролируют в v1

Темы не управляют:

- иерархией экранов
- произвольной layout-композицией
- absolute positioning
- CSS-подобными grid/flex layout
- скруглениями
- тенями
- clipping
- transform-эффектами
- градиентами
- анимациями

Это осознанные ограничения первой версии fixed-template renderer.

## HTML-референс и схема классов

Референсный файл лежит здесь:

```text
ui_reference/index.html
ui_reference/screens/*.html
```

Это не runtime-код, а набор визуальных и поведенческих reference-файлов.

Классы в нём разделены на два namespace:

- `bd-builtin-*` для структурных частей, которые принадлежат renderer'у
- `bd-theme-*` для частей, чьё оформление должно выражаться через `theme.json`

Это основной контракт между макетами и SDL-реализацией.

## Legacy layout JSON-файлы

Файлы в:

```text
config/ui/screens/
```

больше не являются активным source of truth для runtime layout.

Они остаются в репозитории только как legacy reference на переходный период.

## Безопасный порядок правок

Если ты создаёшь или редактируешь тему, безопаснее идти так:

1. править `tokens.colors`
2. править `tokens.spacing`
3. править `fonts`
4. править `typography`
5. править `shell`
6. править `screens`
7. добавлять или заменять изображения и иконки

## Практические заметки

- используй относительные пути внутри папки темы
- предпочитай `#RRGGBB` и `#RRGGBBAA`
- не убирай `bitmap_scale`, даже если используешь реальные шрифты
- сначала проверяй тему на desktop, потом на устройстве

## Связанные документы

- [README проекта](../README.md#ru)
- [Архитектура](./ARCHITECTURE.ru.md)
- [Заметки по платформе](./PLATFORM_NOTES.ru.md)
- [Упаковка для TrimUI SPS](../device/trimui_sps/README.ru.md)
