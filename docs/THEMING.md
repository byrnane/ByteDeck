# ByteDeck Theming Guide

This guide explains how ByteDeck themes work in plain language.

If you open `config/themes/default/theme.json` and see "a pile of magic numbers", this is the document that turns it back into something understandable.

## Mental Model

ByteDeck UI is split into two parts:

- `config/ui/screens/*.json` describes **what is on the screen**
- `config/themes/<theme-id>/theme.json` describes **how it should look**

In other words:

- screen layouts are the structure
- themes are the paint, spacing and typography

Themes do **not** contain gameplay logic, navigation logic or launch behavior.

## What A Theme Can Change

A theme can currently change:

- colors
- spacing
- text scale
- panel fills and borders
- style presets attached to layout classes
- screen variant choice for a given screen

A theme cannot currently:

- invent new screen behavior
- replace the whole app logic
- run expressions or code

## Theme File Structure

Each theme lives here:

```text
config/themes/<theme-id>/theme.json
```

Current shipped theme:

```text
config/themes/default/theme.json
```

The top-level structure is:

```json
{
  "tokens": {},
  "styles": {
    "types": {},
    "classes": {},
    "ids": {}
  },
  "screen_variants": {}
}
```

## 1. Tokens

`tokens` are reusable named values.

Think of them as variables.

Example:

```json
"spacing": {
  "xs": 4,
  "sm": 8,
  "md": 16,
  "lg": 24
}
```

That means:

- `4`, `8`, `16`, `24` are not random numbers
- they are the standard spacing steps used by the theme

Another example:

```json
"text": {
  "primary": [245, 241, 230, 255],
  "muted": [147, 157, 176, 255]
}
```

These are colors in:

```text
[R, G, B, A]
```

Where:

- `R` = red
- `G` = green
- `B` = blue
- `A` = alpha / opacity

So:

```json
[245, 241, 230, 255]
```

means:

- very light warm text
- fully opaque because alpha is `255`

## 2. Styles

`styles` are reusable style rules.

They are split into three buckets:

- `types`: default style for a node type like `screen`, `text`, `panel`
- `classes`: reusable named style presets
- `ids`: one-off overrides for a specific layout node id

Priority is:

1. runtime defaults
2. style by node type
3. styles by class
4. style by id
5. inline values in the layout JSON

So if something looks "wrong", check in that order.

## 3. Screen Variants

`screen_variants` tells the renderer which layout variant to use for a given screen.

Example:

```json
"screen_variants": {
  "main_menu": "hero"
}
```

This means:

- the `main_menu` screen exists in `config/ui/screens/main_menu.json`
- that file contains multiple layout variants
- the active theme chooses `hero`

If no variant is specified, ByteDeck falls back to `default`.

## Token References

Inside styles, values can reference tokens with `$`.

Example:

```json
"fill_color": "$colors.background.panel_primary"
```

This means:

- go to `tokens.colors.background.panel_primary`
- use that value here

It is similar to CSS variables, but simpler.

## Common Value Types

### Colors

Usually written as:

```json
[r, g, b, a]
```

Example:

```json
[26, 31, 42, 255]
```

### Numbers

Used for spacing, padding, border width, height, width, etc.

Example:

```json
"padding": 24
```

### Percentages

Some sizes can be strings like:

```json
"width": "50%"
```

That means 50% of the parent area.

### Fill

Some dimensions can use:

```json
"height": "fill"
```

That means "take the remaining available space".

## The Most Important Classes In The Default Theme

These are the main building blocks:

- `screen-brand`: big top logo text
- `screen-title`: main page title
- `screen-subtitle`: secondary heading text
- `panel-primary`: left/main panel background
- `panel-secondary`: right/details panel background
- `menu-card-*`: main menu cards
- `system-tile-*`: system grid cards
- `browser-list-item-*`: rows in game/app lists
- `preview-frame`: image preview container
- `status-success`: positive launch status text
- `status-error`: error launch status text

## Why There Are So Many Similar Colors

This is intentional. The theme separates:

- outer frame color
- inner fill color
- selected frame color
- selected inner fill color
- muted text color
- highlighted text color

Without that separation, the whole UI quickly turns flat and hard to tune.

## Quick Start: Change The Theme Safely

Start with these edits first:

1. Change background colors in `tokens.colors.background`
2. Change main text colors in `tokens.colors.text`
3. Change spacing scale in `tokens.spacing`
4. Change title sizes in `tokens.typography`

That gives visible changes without risking layout breakage.

## Example: Make The Theme More Contrasty

```json
"text": {
  "primary": [255, 255, 255, 255],
  "muted": [180, 190, 205, 255],
  "highlight": [255, 196, 92, 255],
  "error": [235, 120, 120, 255]
}
```

## Example: Make Spacing Tighter

```json
"spacing": {
  "xs": 2,
  "sm": 6,
  "md": 12,
  "lg": 18,
  "xl": 28,
  "xxl": 40
}
```

## Example: Switch Main Menu Layout Variant

```json
"screen_variants": {
  "main_menu": "default"
}
```

That changes only the layout variant used by the theme.

It does **not** affect navigation logic.

## Relationship Between Layouts And Themes

The layout file says things like:

- there is a `panel`
- inside it there is a `list`
- each item has text and a frame

The theme says things like:

- panels use this fill color
- selected list items use this highlight color
- title text uses this scale

So:

- layout = structure
- theme = presentation

## Troubleshooting

### "I changed a token but nothing happened"

Possible reasons:

- the token is not referenced anywhere
- the value is overridden by a class
- the value is overridden inline in the screen layout

### "I changed a class but only one widget changed"

That usually means:

- only that layout node uses the class
- or a more specific `id` / inline override wins

### "The screen looks broken"

Most likely causes:

- too small spacing values
- invalid width/height combination
- color alpha accidentally set to `0`
- bad JSON syntax

When a layout file fails, ByteDeck should fall back to an error screen instead of crashing.

## Recommended Workflow

For theme development:

1. Start from `config/themes/default/theme.json`
2. Change one small group at a time
3. Rebuild and run desktop first
4. Only then test on device

Start with:

- `tokens.colors`
- `tokens.spacing`
- `tokens.typography`

Touch `styles.classes` only after the basics make sense.

## Current Limitations

Right now the theming system is intentionally simple:

- one shipped theme
- no live reload
- no visual theme editor
- no CSS selector engine
- no code inside theme files

That is deliberate. The goal is to keep the system editable and understandable on a handheld project, not to build a browser.
