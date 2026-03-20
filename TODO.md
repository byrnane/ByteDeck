# ByteDeck 0.1.0 TODO

## Scope

Version `0.1.0` should be a stable first public preview for `TrimUI Smart Pro S` with:

- working launcher boot from `Apps/ByteDeck`
- reliable ROM library scan from ByteDeck folder layout
- solid navigation on device
- working emulator handoff for supported systems
- predictable packaging and rebuild workflow

## Core

- [ ] Review system definitions and finalize the first supported set of platforms.
- [ ] Expand launch adapter coverage beyond the current baseline systems.
- [ ] Improve launch error handling and surface user-facing messages instead of silent black screens.
- [ ] Normalize metadata handling for incomplete or broken `gamelist.xml` files.
- [ ] Decide how `favorites` and `last played` should be updated and persisted in v0.1.0.
- [ ] Add a small validation pass for generated `cache/library.json`.

## UI

- [ ] Replace placeholder main menu presentation with a cleaner handheld-first layout.
- [ ] Improve game browser readability for long titles and descriptions.
- [ ] Add clear empty states for systems with no games and for empty app lists.
- [ ] Add visible launch/loading feedback when starting a game or app.
- [ ] Review focus and back-navigation behavior on every screen.
- [ ] Add a basic settings screen that is useful, not just a stub.

## Media And Metadata

- [ ] Confirm thumbnail path rules on both Windows and TrimUI.
- [ ] Improve preview fallback behavior when images are missing or broken.
- [ ] Decide whether screenshots, boxart and video should stay out of `0.1.0` scope or get partial support.
- [ ] Add a few representative test datasets with mixed metadata quality.

## Localization

- [ ] Replace remaining hardcoded UI strings with translation lookups.
- [ ] Finalize CSV loading behavior and fallback language rules.
- [ ] Verify Cyrillic rendering across the full UI, not only descriptions.

## Device And Packaging

- [ ] Decide which runtime libraries must be bundled in `dist/trimui_sps`.
- [ ] Recheck app packaging against a clean stock SD base.
- [ ] Add a short smoke-test checklist for every device build.
- [ ] Verify resume path after exiting RetroArch and standalone emulators.
- [ ] Review logs on device and trim any remaining low-signal noise.

## Desktop Workflow

- [ ] Keep desktop simulation aligned with device behavior where it matters.
- [ ] Review the root launcher UX and decide whether more actions belong there.
- [ ] Make sure all public scripts have consistent argument names and help output.

## Release Prep

- [ ] Write a `0.1.0` release checklist.
- [ ] Define the expected SD-card folder layout in one clear document.
- [ ] Add version stamping for builds and packaged artifacts.
- [ ] Prepare a minimal changelog format.
- [ ] Test a clean install path from archive to device.
