# ByteDeck Theming Guide

- [Русская версия](./THEMING.ru.md)
- [Project README](../README.md#en)
- [Architecture](./ARCHITECTURE.md)
- [UI Reference HTML](./ui_reference/index.html)

## Overview

ByteDeck themes no longer define screen structure.

The active UI model is:

- screen structure is fixed in code
- themes control presentation
- the HTML reference file documents the intended composition and themeable parts

That means a theme can change how the launcher looks, but not how screens are structurally composed.

## Theme Root

Every theme lives in its own folder:

```text
themes/<theme-id>/
```

The entry file is:

```text
themes/<theme-id>/theme.json
```

Any asset path inside `theme.json` is resolved relative to that theme folder.

Example:

```json
"background_image": "assets/backgrounds/main-menu.png"
```

This means:

```text
themes/<theme-id>/assets/backgrounds/main-menu.png
```

There is no forced internal folder structure beyond `theme.json` being the entrypoint.

## Current Theme Schema

The active fixed-template renderer expects this high-level structure:

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

Reusable values such as:

- `colors`
- `spacing`

### `fonts`

Font families mapped to relative asset paths.

Example:

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

Named text roles used by the renderer.

Example:

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

Fields:

- `family`
- `size`
- `line_height`
- `bitmap_scale`

### `system_icons`

Maps ByteDeck system ids to image files:

```json
"system_icons": {
  "nes": "assets/icons/systems/fc.png",
  "megadrive": "assets/icons/systems/md.png"
}
```

### `shell`

Defines shared shell metrics and styles:

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

Shared reusable presentation fragments, for example:

- image placeholders

### `screens`

Per-screen style sections for fixed templates such as:

- `main_menu`
- `games`
- `browser`
- `apps`
- `settings`
- `placeholder`

Each screen section may contain:

- `metrics`
- block styles
- text styles
- image styles

## Supported Style Properties In Theme v1

These are the properties the fixed renderer is designed to support right now:

- `background_color`
- `text_color`
- `border_color`
- `border_width`
- `background_image`
- `font_role`
- `font_size`
- `line_height` through typography roles
- `padding`
- `gap`
- `width`
- `height`
- `opacity` through color alpha or an `alpha` object field
- `wrap`
- `truncate`

For images:

- regular image asset paths
- system icons
- placeholder text

## Color Formats

Recommended formats:

- `#RRGGBB`
- `#RRGGBBAA`

Examples:

```json
"text_primary": "#F5F1E6"
"overlay": "#E4B756CC"
```

Separate alpha is also supported:

```json
{
  "color": "#F5F1E6",
  "alpha": 75
}
```

Rules:

- `alpha` is in percent from `0` to `100`
- `#RRGGBBAA` has priority over `alpha`
- legacy arrays like `[245, 241, 230, 255]` still work for compatibility, but they are not the preferred format anymore

## Fonts And Text Rendering

ByteDeck supports two text paths:

1. theme fonts loaded from TTF or OTF files
2. built-in bitmap font fallback

If a theme font cannot be loaded, ByteDeck falls back to the built-in bitmap font and keeps the UI usable.

Text sizing is controlled through:

- `font_role`
- `font_size` overrides when needed
- `bitmap_scale` as the fallback size for the bitmap font

## Theme Assets

Theme assets are regular files inside the theme folder.

Current supported asset types:

- fonts
- background images
- system icons
- other UI images referenced by style sections

All asset paths are relative to the active theme root unless an absolute path is used explicitly.

## What Themes Do Not Control In v1

Themes do not control:

- screen hierarchy
- free-form layout composition
- absolute positioning
- CSS-like grid or flex layout
- rounded corners
- shadows
- clipping
- transforms
- gradients
- animations

Those are intentionally outside the first fixed-template renderer.

## HTML Reference And Class Naming

The reference file is:

```text
docs/ui_reference/index.html
docs/ui_reference/screens/*.html
```

They are not runtime code. They are a visual and behavioral reference set.

Class naming in that file follows two namespaces:

- `bd-builtin-*` for parts that are structural and should stay inside the renderer
- `bd-theme-*` for parts whose presentation should be expressible in `theme.json`

This is the main contract between mockup work and the SDL runtime implementation.

## Legacy Layout JSON Files

Files under:

```text
config/ui/screens/
```

are no longer the active source of truth for runtime layout.

They remain in the repository only as legacy reference during the transition.

## Safe Editing Order

If you are creating or editing a theme, the safest order is:

1. edit `tokens.colors`
2. edit `tokens.spacing`
3. edit `fonts`
4. edit `typography`
5. edit `shell`
6. edit `screens`
7. add or replace images and icons

## Practical Notes

- use relative asset paths inside the theme folder
- prefer `#RRGGBB` and `#RRGGBBAA`
- keep `bitmap_scale` set even if you use real fonts
- test the theme on desktop first, then on device

## Related Docs

- [Project README](../README.md#en)
- [Architecture](./ARCHITECTURE.md)
- [Platform Notes](./PLATFORM_NOTES.md)
- [TrimUI SPS Packaging](../device/trimui_sps/README.md)
