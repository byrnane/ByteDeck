#!/bin/sh

SYSTEM_ID=$1
TARGET_PATH=$2
MODE=${BYTEDECK_LAUNCH_SCRIPT_MODE:-mock}

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
APP_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
SDCARD_ROOT=$(CDPATH= cd -- "$APP_ROOT/../.." && pwd)

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

EMUS_ROOT=${BYTEDECK_EMUS_ROOT:-$(first_existing_path "$SDCARD_ROOT/Emus" "$SDCARD_ROOT/Emus" "$SDCARD_ROOT/Emu")}

resolve_emulator_launch() {
    case "$1" in
        nes) printf '%s\n' "$EMUS_ROOT/FC/launch.sh" ;;
        snes) printf '%s\n' "$EMUS_ROOT/SFC/launch.sh" ;;
        megadrive) printf '%s\n' "$EMUS_ROOT/MD/launch.sh" ;;
        psp) printf '%s\n' "$EMUS_ROOT/PPSSPP/launch.sh" ;;
        *) return 1 ;;
    esac
}

if [ "$MODE" != "execute" ]; then
    echo "ByteDeck mock launch: system=$SYSTEM_ID path=$TARGET_PATH"
    exit 0
fi

if [ "$SYSTEM_ID" = "apps" ]; then
    if [ ! -f "$TARGET_PATH" ]; then
        echo "ByteDeck launch error: app target missing: $TARGET_PATH" >&2
        exit 1
    fi

    case "$TARGET_PATH" in
        *.sh) exec /bin/sh "$TARGET_PATH" ;;
        *) exec "$TARGET_PATH" ;;
    esac
fi

EMU_LAUNCH=$(resolve_emulator_launch "$SYSTEM_ID") || {
    echo "ByteDeck launch error: unsupported system id: $SYSTEM_ID" >&2
    exit 1
}

if [ ! -f "$EMU_LAUNCH" ]; then
    echo "ByteDeck launch error: emulator launch script missing: $EMU_LAUNCH" >&2
    exit 1
fi

exec /bin/sh "$EMU_LAUNCH" "$TARGET_PATH"
