#include "viva_hal.h"
#include "viva_osal.h"
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/sys/atomic.h>
static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
RING_BUF_DECLARE(viva_rx, 256);
static atomic_t rx_overflow;
/* Single IRQ producer and single runtime consumer: ring_buf's SPSC contract. */
static void receive_irq(const struct device *dev, void *unused) {
    (void)unused;
    uart_irq_update(dev);
    while (uart_irq_rx_ready(dev)) {
        uint8_t bytes[16];
        int count = uart_fifo_read(dev, bytes, sizeof(bytes));
        if (count <= 0) { break; }
        if (ring_buf_put(&viva_rx, bytes, (uint32_t)count) != (uint32_t)count) { atomic_set(&rx_overflow, 1); }
    }
}
viva_error_t viva_hal_init(void) {
    if (!device_is_ready(console) || uart_irq_callback_user_data_set(console, receive_irq, NULL) != 0) { return VIVA_E_IO; }
    uart_irq_rx_enable(console); return VIVA_OK;
}
const char *viva_hal_platform(void) { return "nucleo_g491re"; }
viva_error_t viva_hal_write(const char *data, size_t len) {
    if (!data || !device_is_ready(console)) { return VIVA_E_IO; }
    for (size_t i = 0; i < len; ++i) { uart_poll_out(console, (unsigned char)data[i]); }
    return VIVA_OK;
}
viva_error_t viva_hal_read(char *byte) {
    if (!byte) { return VIVA_E_ARGUMENT; }
    if (atomic_set(&rx_overflow, 0)) { return VIVA_E_IO; }
    return ring_buf_get(&viva_rx, (uint8_t *)byte, 1) == 1u ? VIVA_OK : VIVA_E_TIMEOUT;
}
bool viva_hal_recovery_check(void) { return device_is_ready(console) && viva_os_watchdog_feed() == VIVA_OK; }
