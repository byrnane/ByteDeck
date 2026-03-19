#!/usr/bin/env bash

set -euo pipefail

config="Release"
build_dir="build-trimui"
clean=0

usage() {
  cat <<'EOF'
Usage: ./scripts/build-trimui-wsl.sh [--config Debug|Release] [--build-dir DIR] [--clean]
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --config)
      config="${2:-}"
      shift 2
      ;;
    --build-dir)
      build_dir="${2:-}"
      shift 2
      ;;
    --clean)
      clean=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if [[ "$config" != "Debug" && "$config" != "Release" ]]; then
  echo "Invalid config: $config" >&2
  exit 1
fi

require_command() {
  local command_name=$1
  local package_name=$2

  if ! command -v "$command_name" >/dev/null 2>&1; then
    echo "Missing '$command_name'. Install it in Ubuntu, for example:" >&2
    echo "  sudo apt update && sudo apt install -y $package_name" >&2
    exit 1
  fi
}

resolve_official_sdk_root() {
  local repo_root=$1
  local candidates=(
    "$repo_root/sdk_tg5050_linux_v1.0.0"
    "$repo_root/toolchains/sdk_tg5050_linux_v1.0.0"
    "$repo_root/toolchains/sdk_tg5050_linux_v1.0.0/sdk_tg5050_linux_v1.0.0"
  )

  local candidate
  for candidate in "${candidates[@]}"; do
    if [[ -x "$candidate/host/opt/ext-toolchain/bin/aarch64-none-linux-gnu-gcc" ]]; then
      printf '%s\n' "$candidate"
      return 0
    fi
  done

  return 1
}

require_command cmake "cmake"

if command -v ninja >/dev/null 2>&1; then
  generator="Ninja"
elif command -v make >/dev/null 2>&1; then
  generator="Unix Makefiles"
else
  echo "Missing build backend. Install one of these in Ubuntu:" >&2
  echo "  sudo apt update && sudo apt install -y ninja-build" >&2
  echo "  sudo apt update && sudo apt install -y make" >&2
  exit 1
fi

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
build_dir_abs="$repo_root/$build_dir"
toolchain_file="$repo_root/cmake/toolchains/trimui-aarch64-linux-gnu.cmake"

sdk_root=${BYTEDECK_TRIMUI_SDK_ROOT:-}
if [[ -z "$sdk_root" ]]; then
  sdk_root=$(resolve_official_sdk_root "$repo_root" || true)
fi

if [[ -z "$sdk_root" ]]; then
  echo "Official TrimUI SDK not found. Expected one of:" >&2
  echo "  $repo_root/sdk_tg5050_linux_v1.0.0" >&2
  echo "  $repo_root/toolchains/sdk_tg5050_linux_v1.0.0" >&2
  exit 1
fi

toolchain_prefix=${BYTEDECK_TRIMUI_TOOLCHAIN_PREFIX:-"$sdk_root/host/opt/ext-toolchain/bin/aarch64-none-linux-gnu-"}
sysroot=${BYTEDECK_TRIMUI_SYSROOT:-"$sdk_root/host/aarch64-buildroot-linux-gnu/sysroot"}
sdl2_root=${BYTEDECK_SDL2_ROOT:-"$sysroot/usr"}

if [[ ! -x "${toolchain_prefix}gcc" ]]; then
  echo "Cross compiler not found: ${toolchain_prefix}gcc" >&2
  exit 1
fi

if [[ ! -d "$sysroot" ]]; then
  echo "Sysroot not found: $sysroot" >&2
  exit 1
fi

if [[ ! -f "$sdl2_root/lib/cmake/SDL2/sdl2-config.cmake" ]]; then
  echo "SDL2 CMake package not found: $sdl2_root/lib/cmake/SDL2/sdl2-config.cmake" >&2
  exit 1
fi

if [[ $clean -eq 1 ]]; then
  rm -rf "$build_dir_abs"
fi

echo "Configuring ByteDeck for TrimUI tg5050 ($config)..."
echo "Generator: $generator"
echo "Toolchain prefix: $toolchain_prefix"
echo "Sysroot: $sysroot"
echo "SDL2 root: $sdl2_root"

cmake \
  -G "$generator" \
  -S "$repo_root" \
  -B "$build_dir_abs" \
  -DCMAKE_TOOLCHAIN_FILE="$toolchain_file" \
  -DCMAKE_BUILD_TYPE="$config" \
  -DBYTEDECK_TRIMUI_TOOLCHAIN_PREFIX="$toolchain_prefix" \
  -DBYTEDECK_TRIMUI_SYSROOT="$sysroot" \
  -DBYTEDECK_SDL2_ROOT="$sdl2_root"

echo "Building ByteDeck for TrimUI tg5050 ($config)..."
cmake --build "$build_dir_abs" --parallel

echo "TrimUI WSL build completed."
