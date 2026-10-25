#ifndef VIVA_SELFTEST_H
#define VIVA_SELFTEST_H
#include "viva_core.h"
typedef struct { uint32_t passed, failed; } viva_test_result_t;
/* Isolated context: never injects faults into the running controller. */
viva_test_result_t viva_core_selftest(void);
#endif
