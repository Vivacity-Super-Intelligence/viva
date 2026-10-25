#include "viva_selftest.h"
#include <string.h>
#define CHECK(x) do { if (x) { ++r.passed; } else { ++r.failed; } } while (0)
viva_test_result_t viva_core_selftest(void) {
    viva_test_result_t r = {0}; viva_core_t c = {0}; viva_config_t cfg = viva_default_config();
    CHECK(viva_init(&c, &cfg, "selftest", VIVA_CAP_BASE, 100u) == VIVA_OK);
    CHECK(c.state == VIVA_BOOT && viva_health(&c) == VIVA_HEALTH_BOOTING);
    CHECK(viva_activate(&c) == VIVA_E_STATE);
    CHECK(viva_begin_selftest(&c) == VIVA_OK && c.state == VIVA_SELFTEST);
    CHECK(viva_finish_selftest(&c, true) == VIVA_OK && c.state == VIVA_READY);
    CHECK(viva_health(&c) == VIVA_HEALTH_READY);
    CHECK(strcmp(viva_platform(&c), "selftest") == 0);
    CHECK(strcmp(viva_version(), "0.1.0") == 0);
    CHECK(viva_tick(&c, 200u) == VIVA_OK && viva_uptime(&c) == 100u);
    CHECK(c.implemented == VIVA_CAP_BASE && c.config.capabilities == VIVA_CAP_BASE);
    cfg.telemetry_period_ms = 99u;
    CHECK(viva_configure(&c, &cfg) == VIVA_E_CONFIG && c.config.telemetry_period_ms == 1000u);
    cfg = c.config; cfg.capabilities |= VIVA_CAP_WATCHDOG;
    CHECK(viva_config_validate(&cfg, VIVA_CAP_BASE) == VIVA_E_CAPABILITY);
    CHECK(viva_activate(&c) == VIVA_OK && viva_health(&c) == VIVA_HEALTH_OK);
    CHECK(viva_fault(&c, VIVA_E_IO) == VIVA_E_IO && c.state == VIVA_FAULT);
    CHECK(viva_recover(&c, false) == VIVA_E_SELFTEST && c.state == VIVA_FAULT);
    CHECK(viva_recover(&c, true) == VIVA_OK && c.state == VIVA_READY && c.diagnostics.last_fault == VIVA_E_IO);
    CHECK(c.diagnostics.faults == 1u && c.diagnostics.recoveries == 1u);
    CHECK(viva_tick(&c, 199u) == VIVA_E_CLOCK && c.state == VIVA_FAULT);
    return r;
}
