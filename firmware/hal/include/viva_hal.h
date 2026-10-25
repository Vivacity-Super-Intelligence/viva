#ifndef VIVA_HAL_H
#define VIVA_HAL_H
#include "viva_core.h"
/* Foundation exposes only console IO. No actuator drivers are linked. */
viva_error_t viva_hal_init(void);
const char *viva_hal_platform(void);
viva_error_t viva_hal_write(const char *data, size_t length);
/* Nonblocking read: OK means one byte, TIMEOUT means no input. */
viva_error_t viva_hal_read(char *byte);
bool viva_hal_recovery_check(void);
void viva_runtime_run(void);
#endif
