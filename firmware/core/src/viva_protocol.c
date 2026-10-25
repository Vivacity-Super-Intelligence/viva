#include "viva_protocol.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
static bool number(const char *s, uint32_t *value) {
    uint32_t n = 0;
    if (*s == '\0') { return false; }
    for (; *s; ++s) {
        if (*s < '0' || *s > '9' || n > (UINT32_MAX - (uint32_t)(*s - '0'))/10u) { return false; }
        n = n * 10u + (uint32_t)(*s - '0');
    }
    *value = n; return true;
}
viva_error_t viva_message_parse(const char *data, size_t len, viva_message_t *m) {
    char local[VIVA_REQUEST_MAX + 1u]; char *parts[5]; size_t count = 1;
    if (data == NULL || m == NULL) { return VIVA_E_ARGUMENT; }
    memset(m, 0, sizeof(*m));
    if (len == 0u || len > VIVA_REQUEST_MAX) { return VIVA_E_OVERFLOW; }
    if (data[len-1u] == '\n') { --len; }
    if (len != 0u && data[len-1u] == '\r') { --len; }
    memcpy(local, data, len); local[len] = '\0'; parts[0] = local;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)local[i];
        if (c < 32u || c > 126u) { return VIVA_E_PROTOCOL; }
        if (c == '|') {
            if (count >= 5u) { return VIVA_E_PROTOCOL; }
            local[i] = '\0'; parts[count++] = &local[i+1u];
        }
    }
    if (count != 5u || strcmp(parts[0], "VIVA") != 0) { return VIVA_E_PROTOCOL; }
    if (!number(parts[2], &m->sequence) || m->sequence == 0u || strlen(parts[3]) == 0u || strlen(parts[3]) >= sizeof(m->type) || strlen(parts[4]) >= sizeof(m->payload)) { return VIVA_E_PROTOCOL; }
    if (strcmp(parts[1], "1") != 0) { return VIVA_E_VERSION; }
    for (const char *p = parts[3]; *p; ++p) { if (*p < 'A' || *p > 'Z') { return VIVA_E_PROTOCOL; } }
    strcpy(m->type, parts[3]); strcpy(m->payload, parts[4]); return VIVA_OK;
}
static viva_error_t frame(uint32_t seq, const char *type, const char *payload, char *out, size_t cap) {
    if (out == NULL || cap == 0u) { return VIVA_E_ARGUMENT; }
    int n = snprintf(out, cap, "VIVA|1|%" PRIu32 "|%s|%s\n", seq, type, payload);
    if (n < 0 || (size_t)n >= cap) { out[0] = '\0'; return VIVA_E_OVERFLOW; }
    return VIVA_OK;
}
static void status(const viva_core_t *c, char *p, size_t cap) {
    (void)snprintf(p, cap, "{\"state\":\"%s\",\"health\":\"%s\",\"uptime_ms\":%" PRIu64 "}", viva_state_name(c->state), viva_health_name(viva_health(c)), viva_uptime(c));
}
viva_error_t viva_telemetry(const viva_core_t *c, uint32_t seq, char *out, size_t cap) {
    char payload[256];
    if (c == NULL || !c->initialized) { return VIVA_E_ARGUMENT; }
    status(c, payload, sizeof(payload)); return frame(seq, "TELEMETRY", payload, out, cap);
}
viva_error_t viva_protocol_handle(viva_core_t *c, const char *data, size_t length, bool recovery_checks, char *out, size_t cap) {
    viva_message_t m = {0}; char p[384] = {0};
    if (c == NULL || !c->initialized) { return VIVA_E_ARGUMENT; }
    viva_error_t e = viva_message_parse(data, length, &m);
    if (e == VIVA_OK) {
        bool is_config = strcmp(m.type, "CONFIG") == 0;
        bool is_fault = strcmp(m.type, "FAULT") == 0;
        if (!is_config && !is_fault && m.payload[0] != '\0') { e = VIVA_E_PROTOCOL; }
        else if (strcmp(m.type, "IDENTIFY") == 0) {
            (void)snprintf(p, sizeof(p), "{\"product\":\"Viva\",\"platform\":\"%s\"}", c->platform);
        } else if (strcmp(m.type, "VERSION") == 0) {
            (void)snprintf(p, sizeof(p), "{\"version\":\"%s\",\"build\":\"%s\",\"config_version\":1,\"protocol_version\":1}", VIVA_VERSION, VIVA_BUILD_ID);
        } else if (strcmp(m.type, "HEALTH") == 0 || strcmp(m.type, "TELEMETRY") == 0) { status(c, p, sizeof(p)); }
        else if (strcmp(m.type, "CAPABILITIES") == 0) {
            (void)snprintf(p, sizeof(p), "{\"implemented\":%" PRIu32 ",\"configured\":%" PRIu32 "}", c->implemented, c->config.capabilities);
        } else if (strcmp(m.type, "DIAGNOSTICS") == 0) {
            (void)snprintf(p, sizeof(p), "{\"faults\":%" PRIu32 ",\"recoveries\":%" PRIu32 ",\"invalid_requests\":%" PRIu32 ",\"last_fault\":%u}", c->diagnostics.faults, c->diagnostics.recoveries, c->diagnostics.invalid_requests, (unsigned)c->diagnostics.last_fault);
        } else if (is_config) {
            if (m.payload[0] != '\0') {
                viva_config_t cfg = c->config;
                if (!number(m.payload, &cfg.telemetry_period_ms)) { e = VIVA_E_CONFIG; }
                else { e = viva_configure(c, &cfg); }
            }
            (void)snprintf(p, sizeof(p), "{\"schema_version\":1,\"telemetry_period_ms\":%" PRIu32 ",\"watchdog_timeout_ms\":%" PRIu32 ",\"capabilities\":%" PRIu32 "}", c->config.telemetry_period_ms, c->config.watchdog_timeout_ms, c->config.capabilities);
        } else if (is_fault) {
            uint32_t code = 0;
            if (!number(m.payload, &code) || code == 0u || code > VIVA_E_UNSUPPORTED) { e = VIVA_E_ARGUMENT; }
            else { (void)viva_fault(c, (viva_error_t)code); status(c, p, sizeof(p)); }
        } else if (strcmp(m.type, "RECOVER") == 0) { e = viva_recover(c, recovery_checks); status(c, p, sizeof(p)); }
        else if (strcmp(m.type, "START") == 0) { e = viva_activate(c); status(c, p, sizeof(p)); }
        else { e = VIVA_E_UNSUPPORTED; }
    }
    if (e != VIVA_OK) {
        if (c->diagnostics.invalid_requests != UINT32_MAX) { ++c->diagnostics.invalid_requests; }
        (void)snprintf(p, sizeof(p), "{\"error\":%u}", (unsigned)e);
        viva_error_t framed = frame(m.sequence, "ERROR", p, out, cap);
        return framed == VIVA_OK ? e : framed;
    }
    return frame(m.sequence, m.type, p, out, cap);
}
