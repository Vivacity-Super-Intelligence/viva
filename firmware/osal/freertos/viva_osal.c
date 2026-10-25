#include "viva_osal.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
static TickType_t ticks(uint32_t ms) { return ms == 0u ? 0 : (TickType_t)(((uint64_t)ms * configTICK_RATE_HZ + 999u)/1000u); }
uint64_t viva_os_now_ms(void) { return (uint64_t)esp_timer_get_time()/1000u; }
void viva_os_sleep_ms(uint32_t ms) { vTaskDelay(ticks(ms)); }
viva_error_t viva_os_mutex_init(viva_mutex_t *m) { if (!m) { return VIVA_E_ARGUMENT; } m->handle = xSemaphoreCreateMutexStatic(&m->storage); return m->handle ? VIVA_OK : VIVA_E_IO; }
viva_error_t viva_os_mutex_lock(viva_mutex_t *m, uint32_t ms) { return m && m->handle ? (xSemaphoreTake(m->handle, ticks(ms)) == pdTRUE ? VIVA_OK : VIVA_E_TIMEOUT) : VIVA_E_ARGUMENT; }
viva_error_t viva_os_mutex_unlock(viva_mutex_t *m) { return m && m->handle ? (xSemaphoreGive(m->handle) == pdTRUE ? VIVA_OK : VIVA_E_STATE) : VIVA_E_ARGUMENT; }
viva_error_t viva_os_queue_init(viva_queue_t *q) { if (!q) { return VIVA_E_ARGUMENT; } q->handle = xQueueCreateStatic(8, sizeof(uint32_t), (uint8_t *)q->data, &q->storage); return q->handle ? VIVA_OK : VIVA_E_IO; }
viva_error_t viva_os_queue_send(viva_queue_t *q, uint32_t v, uint32_t ms) { return q && q->handle ? (xQueueSend(q->handle, &v, ticks(ms)) == pdTRUE ? VIVA_OK : VIVA_E_TIMEOUT) : VIVA_E_ARGUMENT; }
viva_error_t viva_os_queue_receive(viva_queue_t *q, uint32_t *v, uint32_t ms) { return q && q->handle && v ? (xQueueReceive(q->handle, v, ticks(ms)) == pdTRUE ? VIVA_OK : VIVA_E_TIMEOUT) : VIVA_E_ARGUMENT; }
static void task_entry(void *arg) {
    viva_task_t *t = arg; t->entry(t->argument);
    (void)xSemaphoreGive(t->done);
    /* Static task storage remains valid until its creator joins and deletes it. */
    vTaskSuspend(NULL);
}
viva_error_t viva_os_task_start(viva_task_t *t, void (*entry)(void *), void *arg) {
    if (!t || !entry) { return VIVA_E_ARGUMENT; }
    t->entry = entry; t->argument = arg; t->done = xSemaphoreCreateBinaryStatic(&t->done_storage);
    t->handle = xTaskCreateStatic(task_entry, "viva_test", sizeof(t->stack), t, 3, t->stack, &t->storage);
    return t->handle ? VIVA_OK : VIVA_E_IO;
}
viva_error_t viva_os_task_join(viva_task_t *t, uint32_t ms) {
    if (!t || !t->handle) { return VIVA_E_ARGUMENT; }
    if (xSemaphoreTake(t->done, ticks(ms)) != pdTRUE) { return VIVA_E_TIMEOUT; }
    vTaskDelete(t->handle); t->handle = NULL; return VIVA_OK;
}
void viva_os_critical_init(viva_critical_t *s) { s->lock = (portMUX_TYPE)portMUX_INITIALIZER_UNLOCKED; }
void viva_os_critical_enter(viva_critical_t *s) { portENTER_CRITICAL(&s->lock); }
void viva_os_critical_exit(viva_critical_t *s) { portEXIT_CRITICAL(&s->lock); }
viva_error_t viva_os_watchdog_start(uint32_t ms) {
    esp_task_wdt_config_t cfg = { .timeout_ms = ms, .idle_core_mask = 0, .trigger_panic = true };
    esp_err_t e = esp_task_wdt_init(&cfg);
    if (e == ESP_ERR_INVALID_STATE) { e = esp_task_wdt_reconfigure(&cfg); }
    if (e != ESP_OK) { return VIVA_E_IO; }
    return esp_task_wdt_add(NULL) == ESP_OK ? VIVA_OK : VIVA_E_IO;
}
viva_error_t viva_os_watchdog_feed(void) { return esp_task_wdt_reset() == ESP_OK ? VIVA_OK : VIVA_E_IO; }
