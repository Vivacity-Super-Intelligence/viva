# Hardware profiles and compatibility

Profiles in `hardware/profiles/` are JSON Schema 2020-12 documents with additional
cross-field checks. Validate using:

```powershell
python -m pip install -r requirements-dev.txt
python tools/validation/validate_profiles.py
python tools/validation/check_compatibility.py --profile hardware/profiles/esp32s3.json
python tools/validation/check_compatibility.py --profile hardware/profiles/nucleo_g491re.json --require-verified
```

Each logical capability separately records hardware support, firmware
implementation, configuration, physical testing, physical verification and an
evidence path. Implementation and configuration do not prove execution on silicon.
`tested`/`verified` can become true only after a corresponding physical run passes.
The checker rejects mismatched masks, unknown bits, invalid watchdog timing,
incorrect RTOS/framework mapping, missing evidence, and unsupported PASS claims.

| Target | Framework / RTOS | Console | Programming |
| --- | --- | --- | --- |
| ESP32-S3 development board | ESP-IDF v5.5.4 / FreeRTOS | UART0, 115200 8N1 | Espressif esptool via identified USB serial bridge |
| STM32 NUCLEO-G491RE | Zephyr commit bc5d59d1b6657cfeef30124b818c8fbd69da68ac | LPUART1, 115200 8N1 | Onboard ST-LINK/V3E, SWD, OpenOCD |

Exact Zephyr board identifier: `nucleo_g491re`. No substitute target is used.
The pinned upstream Zephyr commit reports version 4.5.0-rc3. Stable v4.4.2
lacks this board; see ADR 008 and the retained failed build log.
COM port numbers are discovery results and can change; never hardcode them as
universal board properties. ESP32-S3 board variants differ in USB bridge, flash,
PSRAM, LED pinout, and USB connector routing. This milestone does not configure
board-specific LEDs or assume a particular USB bridge.

The profiles describe only the seven implemented foundation capabilities, not all
MCU peripherals. ESP32-S3 wireless, robotics IO, CAN, PWM, sensors, and external
actuators are neither enabled nor claimed verified. HAL deliberately exposes only
the console. Watchdog verification scope is start/feed and sustained liveness
unless a separate reset-on-starvation test is recorded.

Sources: [Espressif ESP32-S3 guide](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/index.html),
[Zephyr Nucleo G491RE board documentation](https://docs.zephyrproject.org/latest/boards/st/nucleo_g491re/doc/index.html),
[ST UM2505 board manual](https://www.st.com/resource/en/user_manual/um2505-getting-started-with-stm32g4-series-nucleo64-board-stmicroelectronics.pdf).
The Zephyr board DTS is authoritative for the pinned firmware's pin selection.

For USB power on MB1367, ST's factory jumper table specifies JP5 pins 1–2
(5V_USB_STLK), JP6 IDD fitted, JP3 target reset fitted, JP1 ST-LINK reset open,
and JP7 BOOT0 open. Remove USB before changing jumpers. JP6 removed without an
ammeter leaves the target MCU unpowered even if the ST-LINK enumerates.

`hardware/compatibility/` holds the comparison of actual target results. Consult
the physical report for identities, timestamps, failed attempts and verification
status. This is an engineering baseline with no functional safety certification.
