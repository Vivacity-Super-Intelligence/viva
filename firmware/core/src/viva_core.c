#include "viva_core.h"
#include <string.h>
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "Viva requires octets");
_Static_assert(sizeof(uint32_t) == 4, "Viva requires 32-bit uint32_t");
static bool valid(const viva_core_t *c) { return c != NULL && c->initialized; }
static void increment(uint32_t *n) { if (*n != UINT32_MAX) { ++*n; } }
viva_config_t viva_default_config(void) {
    const viva_config_t c = { VIVA_CONFIG_VERSION, 1000u, 10000u, VIVA_CAP_BASE };
    return c;
}
viva_error_t viva_config_validate(const viva_config_t *c, uint32_t implemented) {
    if (c == NULL) { return VIVA_E_ARGUMENT; }
    if (c->schema_version != VIVA_CONFIG_VERSION || c->telemetry_period_ms < 100u ||
        c->telemetry_period_ms > 5000u || c->watchdog_timeout_ms < 2u * c->telemetry_period_ms ||
        c->watchdog_timeout_ms > 60000u) { return VIVA_E_CONFIG; }
    if ((implemented & ~VIVA_CAP_ALL) != 0u || (c->capabilities & ~implemented) != 0u ||
        (c->capabilities & VIVA_CAP_BASE) != VIVA_CAP_BASE) { return VIVA_E_CAPABILITY; }
    return VIVA_OK;
}
viva_error_t viva_init(viva_core_t *c, const viva_config_t *cfg, const char *platform, uint32_t caps, uint64_t now) {
    if (c == NULL || platform == NULL || platform[0] == '\0') { return VIVA_E_ARGUMENT; }
    size_t length = 0;
    for (; platform[length] != '\0'; ++length) {
        char ch = platform[length];
        if (length >= 32u || !((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-')) { return VIVA_E_ARGUMENT; }
    }
    viva_error_t e = viva_config_validate(cfg, caps);
    if (e != VIVA_OK) { return e; }
    memset(c, 0, sizeof(*c));
    c->config = *cfg; c->platform = platform; c->implemented = caps;
    c->boot_ms = now; c->now_ms = now; c->state = VIVA_BOOT; c->initialized = true;
    return VIVA_OK;
}
viva_error_t viva_begin_selftest(viva_core_t *c) {
    if (!valid(c)) { return VIVA_E_ARGUMENT; }
    if (c->state != VIVA_BOOT) { return VIVA_E_STATE; }
    c->state = VIVA_SELFTEST; return VIVA_OK;
}
viva_error_t viva_finish_selftest(viva_core_t *c, bool passed) {
    if (!valid(c)) { return VIVA_E_ARGUMENT; }
    if (c->state != VIVA_SELFTEST) { return VIVA_E_STATE; }
    if (!passed) { return viva_fault(c, VIVA_E_SELFTEST); }
    c->state = VIVA_READY; return VIVA_OK;
}
viva_error_t viva_activate(viva_core_t *c) {
    if (!valid(c)) { return VIVA_E_ARGUMENT; }
    if (c->state != VIVA_READY) { return VIVA_E_STATE; }
    c->state = VIVA_ACTIVE; return VIVA_OK;
}
viva_error_t viva_fault(viva_core_t *c, viva_error_t e) {
    if (!valid(c) || e <= VIVA_OK || e > VIVA_E_UNSUPPORTED) { return VIVA_E_ARGUMENT; }
    c->state = VIVA_FAULT; c->diagnostics.last_fault = e;
    increment(&c->diagnostics.faults); return e;
}
viva_error_t viva_recover(viva_core_t *c, bool passed) {
    if (!valid(c)) { return VIVA_E_ARGUMENT; }
    if (c->state != VIVA_FAULT) { return VIVA_E_STATE; }
    if (!passed || viva_config_validate(&c->config, c->implemented) != VIVA_OK) { return VIVA_E_SELFTEST; }
    c->state = VIVA_SELFTEST;
    increment(&c->diagnostics.recoveries);
    return viva_finish_selftest(c, true);
}
viva_error_t viva_tick(viva_core_t *c, uint64_t now) {
    if (!valid(c)) { return VIVA_E_ARGUMENT; }
    if (now < c->now_ms) { return viva_fault(c, VIVA_E_CLOCK); }
    c->now_ms = now; return VIVA_OK;
}
viva_error_t viva_configure(viva_core_t *c, const viva_config_t *cfg) {
    if (!valid(c)) { return VIVA_E_ARGUMENT; }
    if (c->state != VIVA_READY) { return VIVA_E_STATE; }
    viva_error_t e = viva_config_validate(cfg, c->implemented);
    if (e != VIVA_OK) { return e; }
    /* The watchdog is installed once by the platform; changing it needs a reboot. */
    if (cfg->watchdog_timeout_ms != c->config.watchdog_timeout_ms || cfg->capabilities != c->config.capabilities) { return VIVA_E_UNSUPPORTED; }
    c->config = *cfg; return VIVA_OK;
}
uint64_t viva_uptime(const viva_core_t *c) { return valid(c) ? c->now_ms - c->boot_ms : 0u; }
viva_health_t viva_health(const viva_core_t *c) {
    if (!valid(c) || c->state == VIVA_FAULT) { return VIVA_HEALTH_FAULT; }
    if (c->state == VIVA_READY) { return VIVA_HEALTH_READY; }
    if (c->state == VIVA_ACTIVE) { return VIVA_HEALTH_OK; }
    return VIVA_HEALTH_BOOTING;
}
const char *viva_state_name(viva_state_t s) {
    static const char *const names[] = { "BOOT", "SELFTEST", "READY", "ACTIVE", "FAULT" };
    return (unsigned)s < sizeof(names)/sizeof(names[0]) ? names[s] : "UNKNOWN";
}
const char *viva_health_name(viva_health_t s) {
    static const char *const names[] = { "BOOTING", "READY", "OK", "FAULT" };
    return (unsigned)s < sizeof(names)/sizeof(names[0]) ? names[s] : "UNKNOWN";
}
const char *viva_platform(const viva_core_t *c) { return valid(c) ? c->platform : "uninitialized"; }
const char *viva_version(void) { return VIVA_VERSION; }
