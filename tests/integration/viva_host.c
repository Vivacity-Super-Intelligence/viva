#include "viva_protocol.h"
#include "viva_selftest.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
int main(void) {
    viva_core_t c = {0}; viva_config_t cfg = viva_default_config();
    char input[256], output[VIVA_RESPONSE_MAX];
    if (viva_init(&c, &cfg, "native", VIVA_CAP_BASE, 0) != VIVA_OK) { return 1; }
    (void)viva_begin_selftest(&c);
    (void)viva_finish_selftest(&c, viva_core_selftest().failed == 0u);
    uint64_t ticks = 0;
    while (fgets(input, sizeof(input), stdin) != NULL) {
        (void)viva_tick(&c, ++ticks * 100u);
        (void)viva_protocol_handle(&c, input, strlen(input), viva_core_selftest().failed == 0u, output, sizeof(output));
        fputs(output, stdout); fflush(stdout);
    }
    return 0;
}
