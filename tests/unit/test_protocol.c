#include "viva_protocol.h"
#include <stdio.h>
#include <string.h>
#define REQUIRE(x) do { ++total; if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); return 1; } } while (0)
static viva_error_t handle(viva_core_t *c, const char *s, char *out) { return viva_protocol_handle(c, s, strlen(s), true, out, VIVA_RESPONSE_MAX); }
int main(void) {
    unsigned total = 0; viva_message_t m; char out[VIVA_RESPONSE_MAX]; viva_core_t c = {0};
    viva_config_t cfg = viva_default_config();
    REQUIRE(viva_init(&c, &cfg, "native", VIVA_CAP_BASE, 0) == VIVA_OK);
    REQUIRE(viva_begin_selftest(&c) == VIVA_OK && viva_finish_selftest(&c, true) == VIVA_OK);
    REQUIRE(viva_message_parse("VIVA|1|42|HEALTH|\r\n", 19, &m) == VIVA_OK && m.sequence == 42u);
    REQUIRE(handle(&c, "VIVA|1|1|IDENTIFY|", out) == VIVA_OK && strstr(out, "native") != NULL);
    REQUIRE(handle(&c, "VIVA|1|2|VERSION|", out) == VIVA_OK && strstr(out, VIVA_VERSION) != NULL);
    REQUIRE(handle(&c, "VIVA|1|3|HEALTH|", out) == VIVA_OK && strstr(out, "READY") != NULL);
    REQUIRE(handle(&c, "VIVA|1|4|CAPABILITIES|", out) == VIVA_OK && strstr(out, "31") != NULL);
    REQUIRE(handle(&c, "VIVA|1|5|CONFIG|200", out) == VIVA_OK && c.config.telemetry_period_ms == 200u);
    REQUIRE(handle(&c, "VIVA|1|6|CONFIG|0", out) == VIVA_E_CONFIG && c.config.telemetry_period_ms == 200u);
    REQUIRE(handle(&c, "VIVA|1|7|FAULT|10", out) == VIVA_OK && c.state == VIVA_FAULT);
    REQUIRE(viva_protocol_handle(&c, "VIVA|1|8|RECOVER|", 17, false, out, sizeof(out)) == VIVA_E_SELFTEST);
    REQUIRE(c.state == VIVA_FAULT);
    REQUIRE(handle(&c, "VIVA|1|9|RECOVER|", out) == VIVA_OK && c.state == VIVA_READY);
    REQUIRE(handle(&c, "VIVA|2|10|HEALTH|", out) == VIVA_E_VERSION && strstr(out, "|10|ERROR|") != NULL);
    REQUIRE(handle(&c, "VIVA|1|4294967296|HEALTH|", out) == VIVA_E_PROTOCOL);
    REQUIRE(handle(&c, "VIVA|1|0|HEALTH|", out) == VIVA_E_PROTOCOL);
    REQUIRE(handle(&c, "VIVA|1|1|HEALTH|x|y", out) == VIVA_E_PROTOCOL);
    REQUIRE(handle(&c, "VIVA|1|1|health|", out) == VIVA_E_PROTOCOL);
    REQUIRE(handle(&c, "VIVA|1|1|HEALTH|x", out) == VIVA_E_PROTOCOL);
    REQUIRE(handle(&c, "VIVA|1|1|CONFIG|-1", out) == VIVA_E_CONFIG);
    REQUIRE(handle(&c, "VIVA|1|1|UNKNOWN|", out) == VIVA_E_UNSUPPORTED);
    REQUIRE(viva_message_parse("VIVA|1|1|HEALTH|\0x", 18, &m) == VIVA_E_PROTOCOL);
    REQUIRE(viva_telemetry(&c, 0, out, 2) == VIVA_E_OVERFLOW && out[0] == '\0');
    REQUIRE(handle(&c, "VIVA|1|1|START|", out) == VIVA_OK && c.state == VIVA_ACTIVE);
    REQUIRE(handle(&c, "VIVA|1|1|CONFIG|500", out) == VIVA_E_STATE);
    /* Deterministic malformed-byte sweep: parser must reject controls without reading beyond input. */
    for (unsigned i = 0; i < 32u; ++i) { char bad[] = "VIVA|1|1|HEALTH|"; bad[7] = (char)i; REQUIRE(viva_message_parse(bad, sizeof(bad)-1u, &m) != VIVA_OK); }
    printf("Viva protocol: %u assertions passed\n", total); return 0;
}
