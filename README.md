# Viva Foundation v0.1

Viva is the robotics firmware, development and deployment platform within
**Vivacity**. This repository implements the **0.1.0 engineering foundation**:
a portable C controller core, ESP32-S3 / FreeRTOS firmware, and Zephyr firmware
for the exact STM32 NUCLEO-G491RE board.

This is an initial engineering foundation, not production-ready robot firmware.
No motors or external actuators are controlled. No license has been selected.

Physical acceptance on 2026-10-25, source `src-f022ba759df586ee`:

| Target | Official toolchain build | Physical acceptance |
| --- | --- | --- |
| ESP32-S3 / FreeRTOS | PASS | 18/18 shared checks, OSAL, serial journey and 60-second observation: PASS |
| NUCLEO-G491RE / Zephyr | PASS | 18/18 shared checks, OSAL, serial journey and 60-second observation: PASS |

Native and ASan/UBSan CTest suites pass. These are engineering observations;
watchdog reset-on-starvation and long endurance are outside the recorded scope.

## What works

- Portable lifecycle: BOOT → SELFTEST → READY → ACTIVE, explicit FAULT and checked recovery.
- Stable version/platform identity, monotonic uptime, health, capabilities and fault diagnostics.
- Validated configuration with bounded memory and explicit error codes.
- Separate native FreeRTOS and Zephyr OS adapters for tasks, synchronization,
  queues, timing, critical sections and watchdog start/feed.
- Versioned serial development protocol and reference Python client.
- Native and sanitized C tests, host integration/schema tests, official target
  build workflows, fail-closed physical acceptance tools and source-bound evidence.

Physical verification is reported in the [actual validation report](docs/testing/PHYSICAL_VALIDATION_REPORT.md).
Build/test evidence lives in `tests/integration/artifacts/`; actual serial runs
live in `tests/hardware/artifacts/`. A compile success alone never means a board
passed. The [cross-platform report](hardware/compatibility/CROSS_PLATFORM_REPORT.md)
shows equivalent checks and RTOS differences.

## Build and validate

From `P:\VIVACITY\viva-foundation` on Windows, with the documented WSL toolchains:

```powershell
wsl -d Ubuntu-24.04 -u root -- bash /mnt/p/VIVACITY/viva-foundation/tools/build/build-wsl.sh native
wsl -d Ubuntu-24.04 -u root -- bash /mnt/p/VIVACITY/viva-foundation/tools/build/build-wsl.sh sanitize
wsl -d Ubuntu-24.04 -u root -- bash /mnt/p/VIVACITY/viva-foundation/tools/build/build-wsl.sh esp32s3
wsl -d Ubuntu-24.04 -u root -- bash /mnt/p/VIVACITY/viva-foundation/tools/build/build-wsl.sh stm32
```

See [setup, flashing and serial commands](docs/deployment/BUILD_AND_FLASH.md)
before connecting or programming a board. Discover USB identity and serial ports;
do not assume the same COM numbers or USB bridge on another ESP32-S3 board.

```powershell
python -m pip install -r requirements-dev.txt
python tools/validation/validate_profiles.py
python tools/validation/release_gate.py
```

The release evidence gate deliberately fails while a required build or physical
test is missing or stale. Hardware tests require attached identified devices;
GitHub's hosted CI only runs host checks and firmware builds.

## Repository map

```text
.
├── CMakeLists.txt / CMakePresets.json / VERSION
├── firmware/
│   ├── core/{include,src,tests}    Portable C API, lifecycle and protocol
│   ├── osal/{include,freertos,zephyr}
│   ├── hal/{include,src}          Console contract and common runtime
│   ├── platforms/
│   │   ├── esp32s3/              ESP-IDF application, UART0
│   │   └── stm32g491re/          Zephyr application, LPUART1 / IWDG
│   └── tests/                    Shared on-device core and OSAL self-tests
├── hardware/{profiles,compatibility}
├── schemas/                      Config, profile and capability schemas
├── tools/{build,flash,serial,validation}
├── tests/{unit,integration,hardware}
├── docs/{architecture,firmware,hardware,testing,deployment,decisions}
└── .github/workflows/ci.yml
```

## Documentation

- [System architecture](docs/architecture/SYSTEM_ARCHITECTURE.md)
- [Firmware API / state model](docs/firmware/FIRMWARE_ARCHITECTURE.md)
- [Serial protocol v1](docs/firmware/SERIAL_PROTOCOL.md)
- [Hardware profiles and compatibility](docs/hardware/HARDWARE_PROFILES.md)
- [Test strategy](docs/testing/TEST_STRATEGY.md) and [shared acceptance specification](tests/hardware/FUNCTIONAL_SPEC.md)
- [Physical validation report](docs/testing/PHYSICAL_VALIDATION_REPORT.md)
- [Build and flash](docs/deployment/BUILD_AND_FLASH.md)
- [Architecture decisions](docs/decisions/ARCHITECTURE_DECISIONS.md)
- [Roadmap](docs/ROADMAP.md)

Known boundaries: volatile configuration/fault history, unauthenticated local
development console, no wireless/OTA/sensors/actuation, no safety certification,
and no production endurance qualification. Watchdog evidence scope is documented
per physical run. Future Runtime, SDK, Studio, Intelligence, Simulation and
Deployment services remain roadmap items.
