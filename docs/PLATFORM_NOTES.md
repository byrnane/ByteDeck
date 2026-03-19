# Platform Notes

Status on March 19, 2026:

- Device-side platform discovery is not complete yet.
- Current work targets desktop simulation only.

## Pending Discovery Items

- SD card mount path on TrimUI Smart Pro S
- Stock launcher integration points
- External app launch mechanism on stock Linux
- RetroArch binary path
- RetroArch core paths
- PPSSPP standalone binary path
- BIOS path expected by RetroArch
- BIOS path expected by PPSSPP
- Return-to-launcher behavior after emulator exit
- SDL2 availability on the stock OS

## Desktop Simulation Assumption

Until device access is available, the launcher resolves paths relative to the repository root:

- `roms/`
- `bios/`
- `Apps/`
- `collections/`
- `cache/`
- `scripts/`

This assumption is temporary and must be replaced by confirmed platform data before deployment.
