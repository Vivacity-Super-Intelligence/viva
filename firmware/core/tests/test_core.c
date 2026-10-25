#include "viva_selftest.h"
#include <stdio.h>
#define REQUIRE(x) do { ++total; if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return 1; } } while (0)
int main(void) {
    unsigned total = 0; viva_core_t c = {0}; viva_config_t cfg = viva_default_config();
    viva_test_result_t r = viva_core_selftest();
    REQUIRE(r.failed == 0u && r.passed == 18u);
    REQUIRE(viva_init(NULL, &cfg, "native", VIVA_CAP_BASE, 0) == VIVA_E_ARGUMENT);
    REQUIRE(viva_init(&c, NULL, "native", VIVA_CAP_BASE, 0) == VIVA_E_ARGUMENT);
    REQUIRE(viva_init(&c, &cfg, "", VIVA_CAP_BASE, 0) == VIVA_E_ARGUMENT);
    REQUIRE(viva_init(&c, &cfg, "unsafe|identity", VIVA_CAP_BASE, 0) == VIVA_E_ARGUMENT);
    REQUIRE(viva_init(&c, &cfg, "abcdefghijklmnopqrstuvwxyz0123456789", VIVA_CAP_BASE, 0) == VIVA_E_ARGUMENT);
    REQUIRE(viva_config_validate(NULL, VIVA_CAP_ALL) == VIVA_E_ARGUMENT);
    REQUIRE(viva_activate(&c) == VIVA_E_ARGUMENT);
    REQUIRE(viva_health(&c) == VIVA_HEALTH_FAULT);
    REQUIRE(viva_init(&c, &cfg, "native", VIVA_CAP_BASE, UINT64_C(0xffffffff)) == VIVA_OK);
    REQUIRE(viva_tick(&c, UINT64_C(0x100000000)) == VIVA_OK && viva_uptime(&c) == 1u);
    REQUIRE(viva_finish_selftest(&c, true) == VIVA_E_STATE);
    REQUIRE(viva_begin_selftest(&c) == VIVA_OK);
    REQUIRE(viva_begin_selftest(&c) == VIVA_E_STATE);
    REQUIRE(viva_finish_selftest(&c, false) == VIVA_E_SELFTEST);
    REQUIRE(c.state == VIVA_FAULT);
    REQUIRE(viva_fault(&c, VIVA_OK) == VIVA_E_ARGUMENT);
    REQUIRE(viva_fault(&c, (viva_error_t)99) == VIVA_E_ARGUMENT);
    REQUIRE(viva_recover(&c, false) == VIVA_E_SELFTEST);
    REQUIRE(viva_recover(&c, true) == VIVA_OK);
    REQUIRE(viva_recover(&c, true) == VIVA_E_STATE);
    cfg.schema_version = 2;
    REQUIRE(viva_config_validate(&cfg, VIVA_CAP_ALL) == VIVA_E_CONFIG);
    cfg = viva_default_config(); cfg.telemetry_period_ms = 5001;
    REQUIRE(viva_config_validate(&cfg, VIVA_CAP_ALL) == VIVA_E_CONFIG);
    cfg = viva_default_config(); cfg.watchdog_timeout_ms = 1999;
    REQUIRE(viva_config_validate(&cfg, VIVA_CAP_ALL) == VIVA_E_CONFIG);
    cfg.watchdog_timeout_ms = 60001;
    REQUIRE(viva_config_validate(&cfg, VIVA_CAP_ALL) == VIVA_E_CONFIG);
    cfg = viva_default_config(); cfg.capabilities = 0;
    REQUIRE(viva_config_validate(&cfg, VIVA_CAP_ALL) == VIVA_E_CAPABILITY);
    cfg = viva_default_config(); cfg.watchdog_timeout_ms = 20000;
    REQUIRE(viva_configure(&c, &cfg) == VIVA_E_UNSUPPORTED);
    cfg = c.config; cfg.telemetry_period_ms = 500;
    REQUIRE(viva_configure(&c, &cfg) == VIVA_OK);
    c.diagnostics.faults = UINT32_MAX;
    REQUIRE(viva_fault(&c, VIVA_E_IO) == VIVA_E_IO && c.diagnostics.faults == UINT32_MAX);
    printf("Viva core: %u native assertions + %u shared assertions passed\n", total, r.passed);
    return 0;
}
