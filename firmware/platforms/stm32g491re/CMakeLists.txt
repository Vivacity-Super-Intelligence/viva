cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(viva VERSION 0.1.0 LANGUAGES C)
set(VIVA_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../..")
target_sources(app PRIVATE src/main.c src/viva_hal_stm32.c
  "${VIVA_ROOT}/firmware/core/src/viva_core.c"
  "${VIVA_ROOT}/firmware/core/src/viva_protocol.c"
  "${VIVA_ROOT}/firmware/tests/viva_selftest.c"
  "${VIVA_ROOT}/firmware/tests/viva_osal_selftest.c"
  "${VIVA_ROOT}/firmware/hal/src/viva_runtime.c"
  "${VIVA_ROOT}/firmware/osal/zephyr/viva_osal.c")
target_include_directories(app PRIVATE "${VIVA_ROOT}/firmware/core/include" "${VIVA_ROOT}/firmware/hal/include" "${VIVA_ROOT}/firmware/osal/include" "${VIVA_ROOT}/firmware/tests")
target_compile_definitions(app PRIVATE VIVA_OSAL_ZEPHYR=1)
target_compile_features(app PRIVATE c_std_11)
if(DEFINED VIVA_BUILD_ID)
  target_compile_definitions(app PRIVATE VIVA_BUILD_ID="${VIVA_BUILD_ID}")
endif()
