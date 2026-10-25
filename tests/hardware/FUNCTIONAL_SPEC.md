# Shared board acceptance specification v1

Targets: ESP32-S3 / ESP-IDF and NUCLEO-G491RE / Zephyr. Use the same source
fingerprint and firmware version. Preconditions: no actuators, identified USB
devices, connected boards, correct toolchain, successful build/flash verification.

1. Record USB VID/PID, adapter serial, target identity, source ID, binary hash and UTC time.
2. Reset the selected target; capture fresh BOOT and SELFTEST lines and first telemetry.
3. Require version 0.1.0, 18/18 portable assertions, OSAL PASS, watchdog STARTED.
4. IDENTIFY and VERSION must return correct target, source build ID, config 1, protocol 1.
5. HEALTH must report READY. Uptime must be nonnegative and monotonic.
6. CAPABILITIES must report implemented/configured masks matching the profile (127).
7. CONFIG read must return 1000 ms; 99 ms must fail unchanged; 500 ms must succeed; restore 1000 ms.
8. Wrong protocol version and unknown message must return explicit errors without a fault/reboot.
9. Inject IO fault 10. Require FAULT health and reject START. Recover through fresh checks to READY.
10. DIAGNOSTICS must retain fault 10, increment fault/recovery counts.
11. START must enter ACTIVE. Configuration changes in ACTIVE must fail.
12. Restore READY using diagnostic fault plus checked recovery. No outputs are activated.
13. Observe for 60 seconds. Require periodic READY telemetry, strictly increasing uptime,
    no unexpected boot/panic, and a successful final HEALTH request.
14. Save raw serial, reset output, test booleans, metadata and failure explanation.

Equivalent implementation: `tools/validation/hardware_validate.py`.
The core's 18-assertion on-device test also checks denied recovery, invalid
capability config, failed lifecycle transitions, historical faults and clock
regression in an isolated context. CI cannot establish these tests ran on boards.

Watchdog scope: driver installation and feeding throughout observation beyond its
timeout. Reset-on-starvation requires a separately recorded test. Transport
CRC/reliability, wireless, sensors and actuators are not covered or claimed.
