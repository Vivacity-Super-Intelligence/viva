#ifndef VIVA_PROTOCOL_H
#define VIVA_PROTOCOL_H
#include "viva_core.h"
#define VIVA_PROTOCOL_VERSION 1u
#define VIVA_REQUEST_MAX 128u
#define VIVA_RESPONSE_MAX 512u
typedef struct { uint32_t sequence; char type[17]; char payload[65]; } viva_message_t;
viva_error_t viva_message_parse(const char *data, size_t length, viva_message_t *message);
viva_error_t viva_protocol_handle(viva_core_t *core, const char *data, size_t length, bool recovery_checks, char *output, size_t capacity);
viva_error_t viva_telemetry(const viva_core_t *core, uint32_t sequence, char *output, size_t capacity);
#endif
