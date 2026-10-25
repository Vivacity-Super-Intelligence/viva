# Architecture decisions

## ADR 001 — One portable C11 core
Accepted, 2026-10-25. Both firmware projects and the native tests compile the same
source files directly. Context ownership and explicit injected monotonic time
avoid RTOS dependencies, hidden global clocks and dynamic allocation. A library
package manager is unnecessary for this initial monorepo.

## ADR 002 — RTOS adapters retain native storage
Accepted. FreeRTOS and Zephyr have different stack, scheduling, synchronization,
critical-section and watchdog semantics. The common task-context API uses separate
backend-specific object layouts; it is source portable, not a binary ABI or ISR
compatibility claim. Static storage is predictable but consumes fixed RAM.

## ADR 003 — Single runtime owner, isolated self-tests
Accepted. The runtime task owns state and parser data. Boot tests use a different
core context so fault/recovery tests cannot corrupt the live context. OSAL tests
run once at boot. Recovery requires fresh portable checks and watchdog/console
availability; there is no automatic recovery from arbitrary faults.

## ADR 004 — Simple serial development protocol
Accepted. Bounded ASCII lines with strict fields and JSON responses make human
diagnostics and host testing straightforward. No unauthenticated network bridge,
CRC, binary serialization framework, command queue, or persistent settings are
introduced. Authentication and robust transport are prerequisites for broader
deployment. Runtime config is limited to telemetry period while READY.

## ADR 005 — Evidence belongs to capabilities
Accepted. Hardware support, implementation, configuration, testing and verification
are separate fields. Checked-in actual logs back verification claims. Native/CI
results cannot substitute for physical acceptance. Source fingerprint plus binary
hash prevent a successful old board run silently validating modified firmware.

## ADR 006 — WSL compilers, Windows USB, one checkout
Accepted. Windows P: is verified mounted by WSL at `/mnt/p`; all Viva sources and
build output remain in the requested workspace. Dependency SDKs live on the Linux
filesystem for toolchain compatibility/performance. Windows esptool/OpenOCD and
pyserial avoid USB forwarding and unnecessary driver changes.

## ADR 007 — No physical outputs or production claims
Accepted. ACTIVE is a lifecycle state, not motion. HAL exposes console only. The
engineering release does not claim production readiness, certified safety,
endurance qualification, or untested peripheral support. No license is chosen
without owner approval.

## ADR 008 — Pin upstream Zephyr board support
Accepted after an actual build failure. Stable Zephyr v4.4.2 does not contain
`boards/st/nucleo_g491re`; the first target build failed with “No board named
nucleo_g491re”. Use official upstream commit
`bc5d59d1b6657cfeef30124b818c8fbd69da68ac` (4.5.0-rc3 version metadata),
which contains the requested board, with its matching west modules and SDK 1.0.1.
This development snapshot is an explicit engineering dependency risk. No other
board is substituted, and CI pins the same commit. Re-evaluate against a stable
release containing this board before production work.

## ADR 009 — Interrupt-buffered STM32 serial reception
Accepted after physical validation. The first NUCLEO image booted and passed
self-tests but did not answer a burst IDENTIFY request. Polling the hardware
receive register every 10 ms cannot preserve a 115200-baud command. The STM32 HAL
now uses a static 256-byte single-producer/single-consumer ring buffer filled by
the UART IRQ; the controller task consumes it in bounded batches. Buffer overflow
reports an IO fault and discards through the next newline before resynchronizing.
ESP-IDF already provides a buffered UART driver. Failed test logs are retained.
