#include "viva_osal.h"
#include <zephyr/drivers/watchdog.h>
#include <string.h>
uint64_t viva_os_now_ms(void) { return (uint64_t)k_uptime_get(); }
void viva_os_sleep_ms(uint32_t ms) { k_msleep((int32_t)ms); }
viva_error_t viva_os_mutex_init(viva_mutex_t *m) { return m && k_mutex_init(&m->native) == 0 ? VIVA_OK : VIVA_E_ARGUMENT; }
viva_error_t viva_os_mutex_lock(viva_mutex_t *m, uint32_t ms) { return !m ? VIVA_E_ARGUMENT : (k_mutex_lock(&m->native, K_MSEC(ms)) == 0 ? VIVA_OK : VIVA_E_TIMEOUT); }
viva_error_t viva_os_mutex_unlock(viva_mutex_t *m) { return !m ? VIVA_E_ARGUMENT : (k_mutex_unlock(&m->native) == 0 ? VIVA_OK : VIVA_E_STATE); }
viva_error_t viva_os_queue_init(viva_queue_t *q) { if (!q) { return VIVA_E_ARGUMENT; } k_msgq_init(&q->native, (char *)q->data, sizeof(uint32_t), 8); return VIVA_OK; }
viva_error_t viva_os_queue_send(viva_queue_t *q, uint32_t v, uint32_t ms) { return !q ? VIVA_E_ARGUMENT : (k_msgq_put(&q->native, &v, K_MSEC(ms)) == 0 ? VIVA_OK : VIVA_E_TIMEOUT); }
viva_error_t viva_os_queue_receive(viva_queue_t *q, uint32_t *v, uint32_t ms) { return !q || !v ? VIVA_E_ARGUMENT : (k_msgq_get(&q->native, v, K_MSEC(ms)) == 0 ? VIVA_OK : VIVA_E_TIMEOUT); }
static void task_entry(void *arg, void *unused1, void *unused2) { (void)unused1; (void)unused2; viva_task_t *t = arg; t->entry(t->argument); }
viva_error_t viva_os_task_start(viva_task_t *t, void (*entry)(void *), void *arg) {
    if (!t || !entry) { return VIVA_E_ARGUMENT; }
    t->entry = entry; t->argument = arg;
    k_thread_create(&t->native, t->stack, K_KERNEL_STACK_SIZEOF(t->stack), task_entry, t, NULL, NULL, 3, 0, K_NO_WAIT);
    return VIVA_OK;
}
viva_error_t viva_os_task_join(viva_task_t *t, uint32_t ms) { return !t ? VIVA_E_ARGUMENT : (k_thread_join(&t->native, K_MSEC(ms)) == 0 ? VIVA_OK : VIVA_E_TIMEOUT); }
void viva_os_critical_init(viva_critical_t *s) { memset(&s->lock, 0, sizeof(s->lock)); }
void viva_os_critical_enter(viva_critical_t *s) { s->key = k_spin_lock(&s->lock); }
void viva_os_critical_exit(viva_critical_t *s) { k_spin_unlock(&s->lock, s->key); }
static const struct device *const wdt = DEVICE_DT_GET(DT_NODELABEL(iwdg));
static int channel = -1;
viva_error_t viva_os_watchdog_start(uint32_t ms) {
    if (!device_is_ready(wdt)) { return VIVA_E_IO; }
    struct wdt_timeout_cfg cfg = { .window = { .min = 0, .max = ms }, .callback = NULL, .flags = WDT_FLAG_RESET_SOC };
    channel = wdt_install_timeout(wdt, &cfg);
    if (channel < 0 || wdt_setup(wdt, WDT_OPT_PAUSE_HALTED_BY_DBG) != 0) { return VIVA_E_IO; }
    return VIVA_OK;
}
viva_error_t viva_os_watchdog_feed(void) { return channel >= 0 && wdt_feed(wdt, channel) == 0 ? VIVA_OK : VIVA_E_IO; }
