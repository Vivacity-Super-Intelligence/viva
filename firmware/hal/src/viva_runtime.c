#include "viva_hal.h"
#include "viva_osal.h"
#include "viva_protocol.h"
#include "viva_selftest.h"
#include <stdio.h>
#include <string.h>
static viva_core_t core;
static char output[VIVA_RESPONSE_MAX];
static bool send_text(const char *text) {
    if (viva_hal_write(text, strlen(text)) != VIVA_OK) { (void)viva_fault(&core, VIVA_E_IO); return false; }
    return true;
}
void viva_runtime_run(void) {
    viva_config_t cfg = viva_default_config(); cfg.capabilities = VIVA_CAP_ALL;
    if (viva_hal_init() != VIVA_OK || viva_init(&core, &cfg, viva_hal_platform(), VIVA_CAP_ALL, viva_os_now_ms()) != VIVA_OK) { return; }
    (void)send_text("VIVA_BOOT product=Viva version=" VIVA_VERSION " build=" VIVA_BUILD_ID " state=BOOT\n");
    (void)viva_begin_selftest(&core);
    (void)send_text("VIVA_STATE SELFTEST\n");
    viva_test_result_t result = viva_core_selftest(); bool os_ok = viva_os_selftest();
    bool watchdog_ok = viva_os_watchdog_start(cfg.watchdog_timeout_ms) == VIVA_OK;
    (void)snprintf(output, sizeof(output), "VIVA_SELFTEST passed=%lu failed=%lu osal=%s watchdog=%s\n", (unsigned long)result.passed, (unsigned long)result.failed, os_ok ? "PASS" : "FAIL", watchdog_ok ? "STARTED" : "FAIL");
    (void)send_text(output);
    (void)viva_finish_selftest(&core, result.failed == 0u && os_ok && watchdog_ok);
    char input[VIVA_REQUEST_MAX]; size_t used = 0; bool discard = false; uint64_t previous = 0;
    for (;;) {
        (void)viva_tick(&core, viva_os_now_ms());
        for (unsigned read_count = 0; read_count < 64u; ++read_count) {
            char b; viva_error_t e = viva_hal_read(&b);
            if (e == VIVA_E_TIMEOUT) { break; }
            if (e != VIVA_OK) { (void)viva_fault(&core, VIVA_E_IO); used = 0; discard = true; break; }
            if (b == '\n') {
                if (discard) { (void)send_text("VIVA|1|0|ERROR|{\"error\":5}\n"); if (core.diagnostics.invalid_requests != UINT32_MAX) { ++core.diagnostics.invalid_requests; } }
                else if (used != 0u) {
                    bool recovery = os_ok && watchdog_ok && viva_core_selftest().failed == 0u && viva_hal_recovery_check();
                    (void)viva_protocol_handle(&core, input, used, recovery, output, sizeof(output));
                    (void)send_text(output);
                }
                used = 0; discard = false;
            } else if (!discard) {
                if (used == sizeof(input)) { discard = true; }
                else { input[used++] = b; }
            }
        }
        if (core.now_ms - previous >= core.config.telemetry_period_ms) {
            previous = core.now_ms;
            if (viva_telemetry(&core, 0, output, sizeof(output)) == VIVA_OK) { (void)send_text(output); }
        }
        if (watchdog_ok && viva_os_watchdog_feed() != VIVA_OK) { (void)viva_fault(&core, VIVA_E_IO); }
        viva_os_sleep_ms(10);
    }
}
