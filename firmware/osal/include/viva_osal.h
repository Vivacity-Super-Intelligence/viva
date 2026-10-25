#ifndef VIVA_OSAL_H
#define VIVA_OSAL_H
#include "viva_core.h"
/* These are task-context APIs. Never call lock, queue, sleep, or join in an ISR.
 * Object storage is static and platform-sized; handles must not be copied. */
#if defined(VIVA_OSAL_FREERTOS)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
typedef struct { StaticSemaphore_t storage; SemaphoreHandle_t handle; } viva_mutex_t;
typedef struct { StaticQueue_t storage; QueueHandle_t handle; uint32_t data[8]; } viva_queue_t;
typedef struct { portMUX_TYPE lock; } viva_critical_t;
typedef struct { StaticTask_t storage; StackType_t stack[1024]; TaskHandle_t handle; StaticSemaphore_t done_storage; SemaphoreHandle_t done; void (*entry)(void *); void *argument; } viva_task_t;
#elif defined(VIVA_OSAL_ZEPHYR)
#include <zephyr/kernel.h>
typedef struct { struct k_mutex native; } viva_mutex_t;
typedef struct { struct k_msgq native; uint32_t data[8]; } viva_queue_t;
typedef struct { struct k_spinlock lock; k_spinlock_key_t key; } viva_critical_t;
typedef struct { struct k_thread native; K_KERNEL_STACK_MEMBER(stack, 2048); void (*entry)(void *); void *argument; } viva_task_t;
#else
#error "Select VIVA_OSAL_FREERTOS or VIVA_OSAL_ZEPHYR"
#endif
uint64_t viva_os_now_ms(void);
void viva_os_sleep_ms(uint32_t milliseconds);
viva_error_t viva_os_mutex_init(viva_mutex_t *mutex);
viva_error_t viva_os_mutex_lock(viva_mutex_t *mutex, uint32_t timeout_ms);
viva_error_t viva_os_mutex_unlock(viva_mutex_t *mutex);
viva_error_t viva_os_queue_init(viva_queue_t *queue);
viva_error_t viva_os_queue_send(viva_queue_t *queue, uint32_t value, uint32_t timeout_ms);
viva_error_t viva_os_queue_receive(viva_queue_t *queue, uint32_t *value, uint32_t timeout_ms);
viva_error_t viva_os_task_start(viva_task_t *task, void (*entry)(void *), void *argument);
viva_error_t viva_os_task_join(viva_task_t *task, uint32_t timeout_ms);
void viva_os_critical_init(viva_critical_t *section);
void viva_os_critical_enter(viva_critical_t *section);
void viva_os_critical_exit(viva_critical_t *section);
viva_error_t viva_os_watchdog_start(uint32_t timeout_ms);
viva_error_t viva_os_watchdog_feed(void);
bool viva_os_selftest(void);
#endif
