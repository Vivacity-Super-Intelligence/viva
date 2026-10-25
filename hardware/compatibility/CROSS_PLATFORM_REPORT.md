# Cross-platform comparison

Source: `src-0e64cfb7530d06c8`; version 0.1.0.

| Behavior | ESP32-S3 | NUCLEO-G491RE |
| --- | --- | --- |
| boot_identity | PASS | PASS |
| selftest | PASS | PASS |
| platform_identity | PASS | PASS |
| firmware_version | PASS | PASS |
| ready_health | PASS | PASS |
| capabilities | PASS | PASS |
| configuration_invalid | PASS | PASS |
| configuration_write | PASS | PASS |
| fault_transition | PASS | PASS |
| fault_blocks_active | PASS | PASS |
| recovery | PASS | PASS |
| fault_diagnostics | PASS | PASS |
| active_transition | PASS | PASS |
| uptime_progression | PASS | PASS |
| watchdog_fed_observation | PASS | PASS |

## Expected platform differences

FreeRTOS timing uses the ESP microsecond timer; Zephyr uses its 64-bit millisecond uptime API. Both expose monotonic milliseconds.
ESP32-S3 uses task watchdog subscription and UART0 via its detected USB bridge. STM32 uses the independent IWDG and the default LPUART1 via ST-LINK VCP.
Static task storage and critical section implementations use native RTOS types. Scheduler tick rounding and boot duration may differ; exact timing equality is not required.
Both compile the same portable core, protocol, isolated 18-assertion self-test and common runtime. Native test processes are not a substitute for either physical column.
