#!/usr/bin/env bash
set -euo pipefail
# Dependency checkouts are SDKs, never a second checkout of Viva.
toolroot="${VIVA_TOOLCHAIN_ROOT:-/opt/viva-toolchains}"
zephyr_ref=bc5d59d1b6657cfeef30124b818c8fbd69da68ac
mkdir -p "$toolroot"
python3 -m venv "$toolroot/venv"
"$toolroot/venv/bin/pip" install 'west==1.5.0' 'jsonschema==4.26.0' 'pyserial==3.5'
case "${1:-all}" in
  esp32s3|all)
    if [ ! -d "$toolroot/esp-idf/.git" ]; then
      git clone --depth 1 --branch v5.5.4 --recursive --shallow-submodules https://github.com/espressif/esp-idf.git "$toolroot/esp-idf"
    fi
    test "$(git -C "$toolroot/esp-idf" describe --tags --exact-match)" = v5.5.4
    (cd "$toolroot/esp-idf"; ./install.sh esp32s3)
    ;;
esac
case "${1:-all}" in
  stm32|all)
    export PATH="$toolroot/venv/bin:$PATH"
    if [ ! -d "$toolroot/zephyrproject/.west" ]; then
      west init --clone-opt=--depth=1 -m https://github.com/zephyrproject-rtos/zephyr --mr "$zephyr_ref" "$toolroot/zephyrproject"
    fi
    cd "$toolroot/zephyrproject"
    git -C zephyr fetch --depth 1 origin "$zephyr_ref"
    git -C zephyr checkout --detach "$zephyr_ref"
    west update -o=--depth=1 cmsis cmsis_6 hal_stm32
    pip install -r zephyr/scripts/requirements-base.txt
    west zephyr-export
    west sdk install --gnu-toolchains arm-zephyr-eabi
    ;;
esac
