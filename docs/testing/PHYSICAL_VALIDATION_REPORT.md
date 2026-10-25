# Physical validation report

Generated UTC: 2026-10-25T18:22:53.576922+00:00
Firmware: 0.1.0; source fingerprint: `src-f022ba759df586ee`.

This report summarizes actual captured results. Missing tests remain unverified.

| Target | Build | Physical acceptance |
| --- | --- | --- |
| esp32s3 | PASS | PASS |
| nucleo_g491re | PASS | PASS |

## esp32s3

Status: **PASS**. Started UTC: 20261025T181949Z; completed: 2026-10-25T18:20:51.460380+00:00.
Serial interface: COM10; USB identity: `{"vid": 6790, "pid": 21971, "serial": "5B5E114053", "description": "USB-Enhanced-SERIAL CH343 (COM10)"}`.
Device response: `{"product": "Viva", "platform": "esp32s3"}`.
Binary SHA256: `8d2407e34720b2a2fe8431a7ef57569fcbe83e1ac1e61ae9bb2b2b457cc26e0c`.
Observation: 60 seconds; uptime 1294 to 61374 ms; 60 telemetry samples.
Raw serial: [tests/hardware/artifacts/20261025T181949Z-esp32s3/serial.log](../../tests/hardware/artifacts/20261025T181949Z-esp32s3/serial.log).
Structured evidence: [result.json](../../tests/hardware/artifacts/20261025T181949Z-esp32s3/result.json).

| Check | Outcome |
| --- | --- |
| serial_device_present | PASS |
| usb_serial_selected | PASS |
| boot_identity | PASS |
| boot_build | PASS |
| selftest | PASS |
| no_boot_failure | PASS |
| platform_identity | PASS |
| firmware_version | PASS |
| ready_health | PASS |
| capabilities | PASS |
| configuration_read | PASS |
| configuration_invalid | PASS |
| configuration_unchanged | PASS |
| configuration_write | PASS |
| configuration_restore | PASS |
| protocol_version_reject | PASS |
| protocol_unknown_reject | PASS |
| fault_transition | PASS |
| fault_health | PASS |
| fault_blocks_active | PASS |
| recovery | PASS |
| fault_diagnostics | PASS |
| active_transition | PASS |
| active_blocks_configuration | PASS |
| no_unexpected_reboot | PASS |
| no_runtime_panic | PASS |
| observation_health | PASS |
| uptime_progression | PASS |
| watchdog_fed_observation | PASS |

Driver start/feed and liveness beyond timeout; reset-on-starvation not tested by this run.

## nucleo_g491re

Status: **PASS**. Started UTC: 20261025T182109Z; completed: 2026-10-25T18:22:11.055961+00:00.
Serial interface: COM9; USB identity: `{"vid": 1155, "pid": 14158, "serial": "002500473331511734333834", "description": "STMicroelectronics STLink Virtual COM Port (COM9)"}`.
Device response: `{"product": "Viva", "platform": "nucleo_g491re"}`.
Binary SHA256: `88b284e5503370499bcbd590ad25dd20c88e04a76996b2e564ee481cb7fe085c`.
Observation: 60 seconds; uptime 1339 to 61382 ms; 60 telemetry samples.
Raw serial: [tests/hardware/artifacts/20261025T182109Z-nucleo_g491re/serial.log](../../tests/hardware/artifacts/20261025T182109Z-nucleo_g491re/serial.log).
Structured evidence: [result.json](../../tests/hardware/artifacts/20261025T182109Z-nucleo_g491re/result.json).

| Check | Outcome |
| --- | --- |
| serial_device_present | PASS |
| stlink_selected | PASS |
| reset_success | PASS |
| boot_identity | PASS |
| boot_build | PASS |
| selftest | PASS |
| no_boot_failure | PASS |
| platform_identity | PASS |
| firmware_version | PASS |
| ready_health | PASS |
| capabilities | PASS |
| configuration_read | PASS |
| configuration_invalid | PASS |
| configuration_unchanged | PASS |
| configuration_write | PASS |
| configuration_restore | PASS |
| protocol_version_reject | PASS |
| protocol_unknown_reject | PASS |
| fault_transition | PASS |
| fault_health | PASS |
| fault_blocks_active | PASS |
| recovery | PASS |
| fault_diagnostics | PASS |
| active_transition | PASS |
| active_blocks_configuration | PASS |
| no_unexpected_reboot | PASS |
| no_runtime_panic | PASS |
| observation_health | PASS |
| uptime_progression | PASS |
| watchdog_fed_observation | PASS |

Driver start/feed and liveness beyond timeout; reset-on-starvation not tested by this run.

## Interpretation and limits

No motors or external actuators are configured or used. ACTIVE is a logical state.
Passing watchdog evidence covers installation, feeding and liveness beyond its configured timeout; it does not prove reset-on-starvation, electrical safety, brownout recovery, or long endurance.
Early failed detection/build/acceptance logs are retained and do not count as successful validation.
The NUCLEO initially enumerated ST-LINK but failed MCU attachment. A subsequent probe measured near-zero target voltage. After the owner restored board jumpers, SWD attached successfully at about 3.32 V. Logs retain both failure and recovery.
Publication and release status must also be checked against GitHub and the final engineering report.
