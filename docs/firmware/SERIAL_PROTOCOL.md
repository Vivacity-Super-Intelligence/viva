# Viva serial protocol v1

Development console: 115200 baud, 8 data bits, no parity, 1 stop bit, no flow
control. The ESP build uses UART0 through the detected board's USB serial bridge;
the NUCLEO build uses Zephyr's chosen LPUART1 console through ST-LINK VCP.

Request framing (ASCII, LF terminated; CRLF also accepted):

```text
VIVA|1|42|IDENTIFY|
VIVA|1|43|CONFIG|500
```

Exactly five pipe-separated fields: magic, version, sequence, uppercase message
type, payload. Host sequence is decimal 1–4294967295. Sequence 0 is reserved for
unsolicited telemetry or unparseable errors. Responses echo sequence/type and use
a compact JSON object payload. `ERROR` responses contain a numeric `error`.
Boot/self-test log lines do not start `VIVA|` and must be treated as diagnostic text.

Requests contain at most 128 bytes before newline in the runtime; payload at most
64 bytes, type at most 16 characters, no control bytes or embedded pipes. At the
portable parser boundary the total supplied buffer is at most 128 bytes (strip
line terminators first when consuming a maximal transport frame). Overlong
transport lines are discarded through LF, report overflow, then resynchronize.
There is no partial-frame timeout; partial data uses bounded storage and does not
block telemetry/watchdog servicing. Responses fit in 512 bytes including NUL.

| Type | Request payload | Response / effect |
| --- | --- | --- |
| IDENTIFY | empty | product and platform |
| VERSION | empty | version, build ID, config and protocol compatibility |
| HEALTH | empty | lifecycle, health, uptime_ms |
| CAPABILITIES | empty | implemented/configured bitmasks |
| DIAGNOSTICS | empty | fault/recovery/invalid counts and historical last fault |
| TELEMETRY | empty | same state data as periodic sequence-0 telemetry |
| CONFIG | empty or decimal ms | Read config or change telemetry period while READY |
| FAULT | known nonzero error number | Diagnostic fault injection, enters FAULT |
| RECOVER | empty | Core and platform checks, then READY if healthy |
| START | empty | READY to ACTIVE, without actuator output |

Capability bits: core 1, diagnostics 2, telemetry 4, configuration 8, protocol 16,
watchdog 32, OSAL 64. The first five are mandatory in v1; firmware config and
profiles must agree. These bits state implemented/configured behavior, not a
physical verification claim. Verified status belongs to test-backed profiles.

Configuration writes affect RAM only and are lost on reset. The watchdog timeout
and capability mask are build-time defaults; changing them live is unsupported.
Writes fail without partial mutation. Unknown types, bad version, malformed
numbers, invalid state, overflow and invalid config produce explicit error codes.

Limitations: one local client, no authentication, encryption, CRC, replay
protection, binary payload, subscriptions, persistent configuration, or reliable
delivery guarantees. A sequence is correlation, not a security counter. Serial
logs can be interrupted at reset; a host must resynchronize on a newline and
retry after detecting a fresh boot. Do not use this protocol for safety-critical
control. `tools/serial/viva_client.py` is the reference host decoder/client.
