# ByteDeck Theming Guide

- [Русская версия](./THEMING.ru.md)
- [Project README](../README.md)
- [Architecture](./ARCHITECTURE.md)

## Overview

ByteDeck UI is split into two parts:

- `config/ui/screens/*.json` describes the screen structure
- `config/themes/<theme-id>/theme.json` describes how that structure looks

Layouts define panels, lists, text and images.
Themes define colors, spacing, typography, backgrounds and icons.

## Theme Root

Every theme lives in its own folder:

```text
config/themes/<theme-id>/
```

The entry file is:

```text
config/themes/<theme-id>/theme.json
```

Any asset path inside `theme.json` is resolved relative to that theme folder.

Example:

```json
"background_image": "images/main-menu.png"
```

This means:

```text
config/themes/<theme-id>/images/main-menu.png
```

There is no forced internal folder structure. You can organize `fonts/`, `icons/`, `images/` and `backgrounds/` however you want.

## Theme File Structure

The main sections are:

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

## Colors

### Recommended formats

Preferred color formats are:

- `#RRGGBB`
- `#RRGGBBAA`

Examples:

```json
"primary": "#F5F1E6"
"accent_overlay": "#E4B756CC"
```

### Separate alpha

If you want to keep alpha separate, use:

```json
{
  "color": "#F5F1E6",
  "alpha": 75
}
```

Rules:

- `alpha` is in percent from `0` to `100`
- `#RRGGBBAA` has priority over `alpha`
- legacy arrays like `[245, 241, 230, 255]` still work for compatibility, but they are no longer the main format

## Tokens

`tokens` are reusable values.

Typical token groups:

- `colors`
- `spacing`

Example:

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

Use a token from styles with `$`:

```json
"background_color": "$colors.background.panel_primary"
```

## Typography And Fonts

### Current text backend

ByteDeck currently supports two text paths:

1. Theme fonts loaded from TTF or OTF files
2. Built-in bitmap font fallback

If a theme font cannot be loaded, ByteDeck falls back to the built-in bitmap font and keeps the UI usable.

### `fonts`

The `fonts` section declares font families.

Example:

```json
"fonts": {
  "ui": {
    "path": "fonts/Inter-Medium.ttf"
  },
  "brand": "fonts/Display.otf"
}
```

Both object and string forms are accepted.

### `typography`

The `typography` section defines named text roles.

Example:

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

Fields:

- `family`: font family from `fonts`
- `size`: font size for TTF/OTF rendering
- `line_height`: optional fixed line height
- `bitmap_scale`: fallback size for the built-in bitmap font

### How text size is chosen

In the current UI runtime:

- `font_role` is the main text style control
- `font_size` can override the size from the role
- `scale` still works as a bitmap-font fallback

Example style:

```json
"menu-card-title": {
  "font_role": "title",
  "text_color": "$colors.text.primary"
}
```

## Theme Assets

Theme assets are regular files inside the theme folder.

Currently supported asset use cases:

- `background_image`
- `system_icons`
- image paths referenced by style rules
- font files from `fonts`

### Background images

Example:

```json
"panel-primary": {
  "background_color": "#1A1F2A",
  "background_image": "images/panel-noise.png"
}
```

### System icons

Example:

```json
"system_icons": {
  "nes": "icons/nes.png",
  "megadrive": "icons/megadrive.png"
}
```

An image node can request an icon by system id through bindings.

## Styles

The `styles` section is split into:

- `types`: defaults by node type
- `classes`: reusable named styles
- `ids`: one-off overrides for a specific layout node

Style priority is:

1. runtime defaults
2. `styles.types`
3. `styles.classes`
4. `styles.ids`
5. inline values from the layout file

### Common style properties

Text:

- `text_color`
- `font_role`
- `font_size`
- `wrap`
- `truncate`

Containers:

- `background_color`
- `background_image`
- `border_color`
- `border_width`
- `padding`

Images:

- `path`
- `placeholder_text`
- `system_icon_bind`

## Screen Variants

Themes can choose a predefined layout variant for a screen:

```json
"screen_variants": {
  "main_menu": "hero"
}
```

This changes the selected layout variant. It does not change screen logic.

## Safe Editing Order

If you are creating a new theme, the safest order is:

1. edit `tokens.colors`
2. edit `tokens.spacing`
3. edit `typography`
4. edit `styles.classes`
5. add images, icons and fonts

## Practical Notes

- Use relative asset paths inside the theme folder
- Prefer `#RRGGBB` and `#RRGGBBAA`
- Keep `bitmap_scale` set even if you use real fonts, so the fallback stays readable
- Test the theme on desktop first, then on device

## Related Docs

- [Project README](../README.md)
- [Architecture](./ARCHITECTURE.md)
- [Platform Notes](./PLATFORM_NOTES.md)
- [TrimUI SPS Packaging](../device/trimui_sps/README.md)
