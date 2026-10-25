#ifndef VIVA_CORE_H
#define VIVA_CORE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define VIVA_VERSION "0.1.0"
#define VIVA_CONFIG_VERSION 1u
#ifndef VIVA_BUILD_ID
#define VIVA_BUILD_ID "local"
#endif
#define VIVA_CAP_CORE (UINT32_C(1) << 0)
#define VIVA_CAP_DIAGNOSTICS (UINT32_C(1) << 1)
#define VIVA_CAP_TELEMETRY (UINT32_C(1) << 2)
#define VIVA_CAP_CONFIG (UINT32_C(1) << 3)
#define VIVA_CAP_PROTOCOL (UINT32_C(1) << 4)
#define VIVA_CAP_WATCHDOG (UINT32_C(1) << 5)
#define VIVA_CAP_OSAL (UINT32_C(1) << 6)
#define VIVA_CAP_BASE UINT32_C(31)
#define VIVA_CAP_ALL UINT32_C(127)
typedef enum { VIVA_OK = 0, VIVA_E_ARGUMENT = 1, VIVA_E_CONFIG = 2,
    VIVA_E_STATE = 3, VIVA_E_CAPABILITY = 4, VIVA_E_OVERFLOW = 5,
    VIVA_E_PROTOCOL = 6, VIVA_E_VERSION = 7, VIVA_E_CLOCK = 8,
    VIVA_E_SELFTEST = 9, VIVA_E_IO = 10, VIVA_E_TIMEOUT = 11,
    VIVA_E_UNSUPPORTED = 12 } viva_error_t;
typedef enum { VIVA_BOOT, VIVA_SELFTEST, VIVA_READY, VIVA_ACTIVE, VIVA_FAULT } viva_state_t;
typedef enum { VIVA_HEALTH_BOOTING, VIVA_HEALTH_READY, VIVA_HEALTH_OK, VIVA_HEALTH_FAULT } viva_health_t;
typedef struct { uint32_t schema_version, telemetry_period_ms, watchdog_timeout_ms, capabilities; } viva_config_t;
typedef struct { uint32_t faults, recoveries, invalid_requests; viva_error_t last_fault; } viva_diagnostics_t;
typedef struct {
    viva_config_t config;
    viva_diagnostics_t diagnostics;
    const char *platform;
    uint64_t boot_ms, now_ms;
    uint32_t implemented;
    viva_state_t state;
    bool initialized;
} viva_core_t;
viva_config_t viva_default_config(void);
viva_error_t viva_config_validate(const viva_config_t *config, uint32_t implemented);
viva_error_t viva_init(viva_core_t *core, const viva_config_t *config, const char *platform, uint32_t implemented, uint64_t now_ms);
viva_error_t viva_begin_selftest(viva_core_t *core);
viva_error_t viva_finish_selftest(viva_core_t *core, bool passed);
viva_error_t viva_activate(viva_core_t *core);
viva_error_t viva_fault(viva_core_t *core, viva_error_t fault);
viva_error_t viva_recover(viva_core_t *core, bool checks_passed);
viva_error_t viva_tick(viva_core_t *core, uint64_t now_ms);
viva_error_t viva_configure(viva_core_t *core, const viva_config_t *config);
uint64_t viva_uptime(const viva_core_t *core);
viva_health_t viva_health(const viva_core_t *core);
const char *viva_state_name(viva_state_t state);
const char *viva_health_name(viva_health_t health);
const char *viva_platform(const viva_core_t *core);
const char *viva_version(void);
#endif
