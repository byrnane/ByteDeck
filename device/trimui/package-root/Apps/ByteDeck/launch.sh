#!/bin/sh

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
APP_ROOT="$SCRIPT_DIR"
APPS_ROOT=$(CDPATH= cd -- "$APP_ROOT/.." && pwd)
SDCARD_ROOT=$(CDPATH= cd -- "$APPS_ROOT/.." && pwd)

first_existing_path() {
    default_path=$1
    shift

    for candidate in "$@"; do
        if [ -d "$candidate" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    printf '%s\n' "$default_path"
}

export BYTEDECK_ROOT="$APP_ROOT"
export BYTEDECK_ROMS_ROOT="${BYTEDECK_ROMS_ROOT:-$(first_existing_path "$SDCARD_ROOT/Roms" "$SDCARD_ROOT/Roms" "$SDCARD_ROOT/ROMS" "$SDCARD_ROOT/roms")}"
export BYTEDECK_BIOS_ROOT="${BYTEDECK_BIOS_ROOT:-$(first_existing_path "$SDCARD_ROOT/BIOS" "$SDCARD_ROOT/BIOS" "$SDCARD_ROOT/Bios" "$SDCARD_ROOT/bios")}"
export BYTEDECK_APPS_ROOT="${BYTEDECK_APPS_ROOT:-$(first_existing_path "$SDCARD_ROOT/Apps" "$SDCARD_ROOT/Apps" "$SDCARD_ROOT/App" "$SDCARD_ROOT/apps")}"
export BYTEDECK_COLLECTIONS_ROOT="${BYTEDECK_COLLECTIONS_ROOT:-$(first_existing_path "$SDCARD_ROOT/collections" "$SDCARD_ROOT/collections" "$SDCARD_ROOT/Collections")}"
export BYTEDECK_CACHE_ROOT="${BYTEDECK_CACHE_ROOT:-$APP_ROOT/cache}"
export BYTEDECK_SCRIPTS_ROOT="${BYTEDECK_SCRIPTS_ROOT:-$APP_ROOT/scripts}"
export BYTEDECK_CONFIG_ROOT="${BYTEDECK_CONFIG_ROOT:-$APP_ROOT/config}"
export BYTEDECK_EMUS_ROOT="${BYTEDECK_EMUS_ROOT:-$(first_existing_path "$SDCARD_ROOT/Emus" "$SDCARD_ROOT/Emus" "$SDCARD_ROOT/Emu")}"
export BYTEDECK_LAUNCH_MODE="${BYTEDECK_LAUNCH_MODE:-execute}"
export BYTEDECK_LAUNCH_SCRIPT_MODE="${BYTEDECK_LAUNCH_SCRIPT_MODE:-execute}"
export BYTEDECK_INPUT_BACKEND="${BYTEDECK_INPUT_BACKEND:-joystick}"

export LD_LIBRARY_PATH="$APP_ROOT/lib:/usr/trimui/lib:/usr/lib:/lib:$LD_LIBRARY_PATH"

mkdir -p "$BYTEDECK_CACHE_ROOT/logs"
cd "$APP_ROOT" || exit 1

exec "$APP_ROOT/bin/bytedeck" "$@" >"$BYTEDECK_CACHE_ROOT/logs/bytedeck-stdout.log" 2>&1
