set(VIVA_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../..")
idf_component_register(SRCS "main.c" "viva_hal_esp32.c"
  "${VIVA_ROOT}/firmware/core/src/viva_core.c"
  "${VIVA_ROOT}/firmware/core/src/viva_protocol.c"
  "${VIVA_ROOT}/firmware/tests/viva_selftest.c"
  "${VIVA_ROOT}/firmware/tests/viva_osal_selftest.c"
  "${VIVA_ROOT}/firmware/hal/src/viva_runtime.c"
  "${VIVA_ROOT}/firmware/osal/freertos/viva_osal.c"
  INCLUDE_DIRS "${VIVA_ROOT}/firmware/core/include" "${VIVA_ROOT}/firmware/hal/include"
  "${VIVA_ROOT}/firmware/osal/include" "${VIVA_ROOT}/firmware/tests"
  REQUIRES esp_driver_uart esp_timer esp_system)
target_compile_definitions(${COMPONENT_LIB} PUBLIC VIVA_OSAL_FREERTOS=1)
target_compile_features(${COMPONENT_LIB} PRIVATE c_std_11)
if(DEFINED VIVA_BUILD_ID)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE VIVA_BUILD_ID="${VIVA_BUILD_ID}")
endif()
