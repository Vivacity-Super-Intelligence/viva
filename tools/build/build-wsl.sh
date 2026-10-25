#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
toolroot="${VIVA_TOOLCHAIN_ROOT:-/opt/viva-toolchains}"
export VIVA_BUILD_ID="$(python3 "$root/tools/validation/source_identity.py")"
case "${1:-native}" in
  native)
    export PATH="$toolroot/venv/bin:$PATH"
    cd "$root"
    cmake --preset native
    cmake --build --preset native
    ctest --preset native
    ;;
  sanitize)
    export PATH="$toolroot/venv/bin:$PATH"
    cd "$root"
    cmake --preset sanitize
    cmake --build --preset sanitize
    ctest --preset sanitize
    ;;
  esp32s3)
    source "$toolroot/esp-idf/export.sh"
    cd "$root/firmware/platforms/esp32s3"
    idf.py -D VIVA_BUILD_ID="$VIVA_BUILD_ID" -B "$root/build/esp32s3" build
    ;;
  stm32)
    source "$toolroot/venv/bin/activate"
    export ZEPHYR_BASE="$toolroot/zephyrproject/zephyr"
    west build -b nucleo_g491re -d "$root/build/stm32g491re" "$root/firmware/platforms/stm32g491re" -- -DVIVA_BUILD_ID="$VIVA_BUILD_ID"
    ;;
  *) printf 'Use native, sanitize, esp32s3, or stm32\n' >&2; exit 2 ;;
esac
