# Contributing

## Local Workflow

1. Build and run the desktop version first.
2. Keep changes aligned with `trimui_launcher_spec.md`.
3. Test scanner changes against local `roms/` data.
4. Do not commit local ROM, BIOS, cache or generated binaries.

## Windows

- Fast loop: `scripts/dev-windows.bat`
- Clean rebuild: `scripts/dev-windows-clean.bat`

## Linux

```bash
cmake -S . -B build
cmake --build build
./build/bytedeck
```

## Commit Guidelines

- Keep commits focused and small.
- Use short Russian commit messages.
- Avoid mixing refactors with behavior changes unless necessary.

## Pull Requests

- Explain what changed and why.
- Mention how the change was tested.
- Include screenshots for visible UI changes when relevant.
