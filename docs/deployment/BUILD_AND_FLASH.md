# Windows / WSL2 build, flash and monitor

Authoritative directory: `P:\VIVACITY\viva-foundation`. Use Windows for USB;
use Ubuntu 24.04 WSL2 for official cross-toolchains. No second Viva checkout is
needed. All commands below are run from the repository root unless stated.

## Inspect and set up

```powershell
Set-Location 'P:\VIVACITY\viva-foundation'
Get-ChildItem -Force
wsl --list --verbose
wsl --install -d Ubuntu-24.04 --no-launch --web-download --location 'P:\VIVACITY\viva-foundation\.tools\wsl\Ubuntu-24.04'
wsl -d Ubuntu-24.04 -u root -- wslpath -u 'P:\VIVACITY\viva-foundation'
wsl -d Ubuntu-24.04 -u root -- bash -lc 'findmnt -t 9p,drvfs; ls -la /mnt/p/VIVACITY/viva-foundation'
```

Do not assume `/mnt/p/viva-foundation`. If P: is absent, inspect `findmnt` and
`/etc/wsl.conf`. A supported explicit mount of this fixed NTFS drive, after
confirming it is not already mounted, is `mkdir -p /mnt/p` followed by
`mount -t drvfs P: /mnt/p` as root. Do not copy or move the repository.

Allow at least 20 GB for Linux SDKs and caches on the host volume that stores the
WSL virtual disk. Linux `df` reports virtual capacity and does not prove that the
Windows host has room to grow the disk. Check `Get-Volume` on Windows. During
this session the new distro's default C: placement exhausted host working space;
its storage was relocated to the above ignored P: directory using
`wsl --manage Ubuntu-24.04 --move PATH`. The repository was never moved.

Install Linux host packages (inside WSL):

```bash
apt-get update
apt-get install -y git wget curl flex bison gperf python3-pip python3-venv \
  cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0-dev \
  build-essential device-tree-compiler gdb-multiarch openocd unzip xz-utils file
cd /mnt/p/VIVACITY/viva-foundation
bash tools/build/bootstrap-wsl.sh esp32s3
bash tools/build/bootstrap-wsl.sh stm32
```

These scripts install pinned ESP-IDF v5.5.4, Zephyr commit bc5d59d1b6657cfeef30124b818c8fbd69da68ac, west 1.5.0 and the
Zephyr ARM SDK selected by that pinned Zephyr version. Dependency SDKs use
`/opt/viva-toolchains`; Python environments isolate host and ESP-IDF dependencies.
The scripts do not modify shell profiles or change the default WSL distribution.
The Zephyr commit reports 4.5.0-rc3; it is pinned because stable v4.4.2 lacks
`nucleo_g491re`. Its matching SDK is 1.0.1. See ADR 008 for the dependency risk.

Windows serial tools:

```powershell
python -m pip install -r requirements-dev.txt
python -m pip install esptool==5.6.0
python -m serial.tools.list_ports -v
Get-PnpDevice -PresentOnly | Where-Object InstanceId -Match '^USB'
```

For ST-LINK on Windows, install xPack OpenOCD 0.12.0-7 (upstream OpenOCD
distribution) and retain its scripts beside its executable. The validation session
uses the package under ignored `.tools/xpack-openocd-0.12.0-7/`:

```powershell
gh release download v0.12.0-7 --repo xpack-dev-tools/openocd-xpack --pattern '*win32-x64.zip' --dir .tools
Expand-Archive .tools/xpack-openocd-0.12.0-7-win32-x64.zip .tools
$openOcd = (Resolve-Path '.tools/xpack-openocd-0.12.0-7/bin/openocd.exe').Path
```

## Build and test

```powershell
wsl -d Ubuntu-24.04 -u root -- bash /mnt/p/VIVACITY/viva-foundation/tools/build/build-wsl.sh native
wsl -d Ubuntu-24.04 -u root -- bash /mnt/p/VIVACITY/viva-foundation/tools/build/build-wsl.sh sanitize
wsl -d Ubuntu-24.04 -u root -- bash /mnt/p/VIVACITY/viva-foundation/tools/build/build-wsl.sh esp32s3
wsl -d Ubuntu-24.04 -u root -- bash /mnt/p/VIVACITY/viva-foundation/tools/build/build-wsl.sh stm32
```

Equivalent inside WSL: `bash tools/build/build-wsl.sh native` (or `sanitize`,
`esp32s3`, `stm32`). Build output remains under the single root `build/`, ignored
by Git. The root native project can also build on Windows with a supported C11
compiler, CMake and Ninja; this session's tested host configuration is WSL GCC.

ESP output: `build/esp32s3/viva.bin`, bootloader, partition table and
`flasher_args.json`. STM output: `build/stm32g491re/zephyr/zephyr.elf`, `.bin`,
and `.hex`. Never flash an ELF at a raw address using a binary-only command.

## Identify, flash, reset, validate

Use discovered ports and ST-LINK serial, not a guessed port. The following values
are examples from this session; recheck on another connection:

```powershell
python -m esptool --chip esp32s3 --port COM10 chip-id
& $openOcd -f interface/stlink.cfg -c 'adapter serial 002500473331511734333834' -f target/stm32g4x.cfg -c 'init; targets; shutdown'
.\tools\flash\flash-esp32.ps1 -Port COM10
.\tools\flash\flash-stm32.ps1 -OpenOcd $openOcd -ProbeSerial 002500473331511734333834
$buildId = python tools/validation/source_identity.py
python tools/validation/hardware_validate.py --platform esp32s3 --port COM10 --build-id $buildId --binary build/esp32s3/viva.bin
python tools/validation/hardware_validate.py --platform nucleo_g491re --port COM9 --openocd $openOcd --probe-serial 002500473331511734333834 --build-id $buildId --binary build/stm32g491re/zephyr/zephyr.bin
```

Flashing overwrites only the selected development board's application/required
boot image as authorized. No mass erase, option-byte changes, security unprotect,
or ST-LINK firmware update is required. If target identity is ambiguous, stop.

The hardware runner resets the board with the serial interface already open and
captures startup. Do not run another serial monitor concurrently. Manual monitor:

```powershell
python -m serial.tools.miniterm COM10 115200 --raw
python -m serial.tools.miniterm COM9 115200 --raw
```

Exit miniterm with Ctrl+]. Press the board RESET button for fresh logs. Protocol
requests require LF; use the reference client when exact framing matters.

Troubleshooting: a visible ST-LINK does not prove target power or SWD connectivity.
Check factory jumpers and voltage first. `reset_config srst_only srst_nogate
connect_assert_srst` with 1000 kHz can attach under reset. Do not erase a board to
diagnose a power fault. ESP connection failures need a data cable and the actual
UART bridge; native USB may enumerate differently and is not this build's console.

Releases: run native/sanitizer and both board builds, flash and accept both boards,
validate profiles and release evidence, scan/review staging, push main and wait
for all CI jobs. Only then create v0.1.0. No license is selected in this project.
