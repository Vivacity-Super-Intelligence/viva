#include "viva_hal.h"
#include "viva_osal.h"
#include "driver/uart.h"
viva_error_t viva_hal_init(void) {
    uart_config_t cfg = { .baud_rate = 115200, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, .source_clk = UART_SCLK_DEFAULT };
    if (uart_param_config(UART_NUM_0, &cfg) != ESP_OK || uart_driver_install(UART_NUM_0, 512, 0, 0, NULL, 0) != ESP_OK) { return VIVA_E_IO; }
    return VIVA_OK;
}
const char *viva_hal_platform(void) { return "esp32s3"; }
viva_error_t viva_hal_write(const char *data, size_t len) {
    return data && uart_write_bytes(UART_NUM_0, data, len) == (int)len ? VIVA_OK : VIVA_E_IO;
}
viva_error_t viva_hal_read(char *byte) {
    if (!byte) { return VIVA_E_ARGUMENT; }
    int n = uart_read_bytes(UART_NUM_0, byte, 1, 0);
    return n == 1 ? VIVA_OK : (n == 0 ? VIVA_E_TIMEOUT : VIVA_E_IO);
}
bool viva_hal_recovery_check(void) { return viva_os_watchdog_feed() == VIVA_OK; }
