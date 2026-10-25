#include "viva_osal.h"
static viva_mutex_t mutex;
static viva_queue_t queue;
static viva_task_t task;
static viva_critical_t critical;
static void producer(void *arg) { (void)arg; (void)viva_os_queue_send(&queue, UINT32_C(0x56495641), 100); }
bool viva_os_selftest(void) {
    uint32_t value = 0;
    if (viva_os_mutex_init(&mutex) != VIVA_OK || viva_os_mutex_lock(&mutex, 100) != VIVA_OK || viva_os_mutex_unlock(&mutex) != VIVA_OK || viva_os_queue_init(&queue) != VIVA_OK) { return false; }
    if (viva_os_queue_receive(&queue, &value, 0) != VIVA_E_TIMEOUT) { return false; }
    if (viva_os_task_start(&task, producer, NULL) != VIVA_OK) { return false; }
    bool received = viva_os_queue_receive(&queue, &value, 500) == VIVA_OK && value == UINT32_C(0x56495641);
    bool joined = viva_os_task_join(&task, 500) == VIVA_OK;
    viva_os_critical_init(&critical); viva_os_critical_enter(&critical); value = 42; viva_os_critical_exit(&critical);
    uint64_t before = viva_os_now_ms(); viva_os_sleep_ms(20);
    return received && joined && value == 42u && viva_os_now_ms() > before;
}
