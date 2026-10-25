"""Physical-only serial acceptance run. Writes FAIL evidence on any missing check.

No simulation fallback. Reset uses ESP RTS or an explicitly selected ST-LINK.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import time

import serial
from serial.tools import list_ports

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/serial"))
from viva_client import Client, decode

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--platform", choices=["esp32s3", "nucleo_g491re"], required=True)
    parser.add_argument("--port", required=True)
    parser.add_argument("--openocd", type=Path)
    parser.add_argument("--probe-serial")
    parser.add_argument("--observe", type=int, default=60)
    parser.add_argument("--build-id", required=True)
    parser.add_argument("--binary", type=Path, required=True)
    args = parser.parse_args()
    if args.observe < 30:
        parser.error("At least 30 seconds observation required")
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    folder = ROOT / "tests/hardware/artifacts" / f"{stamp}-{args.platform}"
    folder.mkdir(parents=True, exist_ok=False)
    result = {"physical": True, "status": "FAIL", "timestamp_utc": stamp,
              "platform": args.platform, "version": "0.1.0", "build_id": args.build_id,
              "binary_sha256": hashlib.sha256(args.binary.read_bytes()).hexdigest(),
              "port": args.port, "checks": {}, "passed_capabilities": [], "observation_seconds": args.observe}
    checks = result["checks"]

    def check(name, condition):
        checks[name] = bool(condition)
        if not condition:
            raise AssertionError(name)

    try:
        ports = [p for p in list_ports.comports() if p.device == args.port]
        check("serial_device_present", len(ports) == 1)
        device = ports[0]
        result["usb_identity"] = {"vid": device.vid, "pid": device.pid, "serial": device.serial_number, "description": device.description}
        if args.platform == "nucleo_g491re":
            check("stlink_selected", device.vid == 0x0483 and device.serial_number == args.probe_serial and args.openocd is not None)
        else:
            check("usb_serial_selected", device.vid is not None and device.vid != 0x0483)
        with (folder / "serial.log").open("w", encoding="utf-8") as log:
            with serial.Serial(args.port, 115200, timeout=0.2, write_timeout=2) as port:
                port.dtr = args.platform == "nucleo_g491re"
                port.reset_input_buffer()
                client = Client(port, log)
                if args.platform == "esp32s3":
                    port.rts = True
                    time.sleep(0.1)
                    port.rts = False
                else:
                    reset = subprocess.run([str(args.openocd), "-f", "interface/stlink.cfg", "-c", f"adapter serial {args.probe_serial}", "-f", "target/stm32g4x.cfg", "-c", "init; reset run; shutdown"], capture_output=True, text=True, timeout=20)
                    (folder / "reset.log").write_text(reset.stdout + reset.stderr)
                    check("reset_success", reset.returncode == 0)
                boot = []
                seen_boot = False
                deadline = time.monotonic() + 12
                while time.monotonic() < deadline:
                    line = client.line()
                    boot.append(line)
                    seen_boot = seen_boot or "VIVA_BOOT product=Viva" in line
                    message = decode(line)
                    if seen_boot and message and message["type"] == "TELEMETRY" and message["payload"]["state"] in ("READY", "FAULT"):
                        break
                text = "".join(boot)
                check("boot_identity", "VIVA_BOOT product=Viva version=0.1.0" in text)
                check("boot_build", f"build={args.build_id} " in text)
                check("selftest", "VIVA_SELFTEST passed=18 failed=0 osal=PASS watchdog=STARTED" in text)
                check("no_boot_failure", all(s not in text for s in ["FAIL", "panic", "Guru Meditation", "assert failed"]))
                identity = client.request("IDENTIFY")
                result["device_identity"] = identity
                check("platform_identity", identity == {"product": "Viva", "platform": args.platform})
                version = client.request("VERSION")
                check("firmware_version", version["version"] == "0.1.0" and version["build"] == args.build_id and version["config_version"] == 1 and version["protocol_version"] == 1)
                check("ready_health", client.request("HEALTH")["health"] == "READY")
                caps = client.request("CAPABILITIES")
                check("capabilities", caps == {"implemented": 127, "configured": 127})
                check("configuration_read", client.request("CONFIG")["telemetry_period_ms"] == 1000)
                check("configuration_invalid", client.request("CONFIG", "99", expect_error=True)["error"] == 2)
                check("configuration_unchanged", client.request("CONFIG")["telemetry_period_ms"] == 1000)
                check("configuration_write", client.request("CONFIG", "500")["telemetry_period_ms"] == 500)
                check("configuration_restore", client.request("CONFIG", "1000")["telemetry_period_ms"] == 1000)
                check("protocol_version_reject", client.request("HEALTH", version=2, expect_error=True)["error"] == 7)
                check("protocol_unknown_reject", client.request("UNKNOWN", expect_error=True)["error"] == 12)
                check("fault_transition", client.request("FAULT", "10")["state"] == "FAULT")
                check("fault_health", client.request("HEALTH")["health"] == "FAULT")
                check("fault_blocks_active", client.request("START", expect_error=True)["error"] == 3)
                check("recovery", client.request("RECOVER")["state"] == "READY")
                diag = client.request("DIAGNOSTICS")
                check("fault_diagnostics", diag["faults"] == 1 and diag["recoveries"] == 1 and diag["last_fault"] == 10)
                check("active_transition", client.request("START")["state"] == "ACTIVE")
                check("active_blocks_configuration", client.request("CONFIG", "500", expect_error=True)["error"] == 3)
                # Restore READY through the same checked recovery path, without controlling hardware.
                client.request("FAULT", "10")
                client.request("RECOVER")
                before = client.request("TELEMETRY")["uptime_ms"]
                observed = []
                deadline = time.monotonic() + args.observe
                while time.monotonic() < deadline:
                    line = client.line()
                    check("no_unexpected_reboot", "VIVA_BOOT" not in line)
                    check("no_runtime_panic", all(s not in line for s in ["panic", "Guru Meditation", "assert failed"]))
                    message = decode(line)
                    if message and message["type"] == "TELEMETRY":
                        check("observation_health", message["payload"]["health"] == "READY")
                        observed.append(message["payload"]["uptime_ms"])
                after = client.request("HEALTH")["uptime_ms"]
                check("uptime_progression", after > before + (args.observe - 1) * 1000 and len(observed) >= args.observe // 2 and all(b > a for a, b in zip(observed, observed[1:])))
                check("watchdog_fed_observation", after - before > 10000)
                result["uptime_start_ms"], result["uptime_end_ms"] = before, after
                result["telemetry_samples"] = len(observed)
                result["passed_capabilities"] = ["core", "diagnostics", "telemetry", "configuration", "protocol", "watchdog", "osal"]
                result["watchdog_scope"] = "Driver start/feed and liveness beyond timeout; reset-on-starvation not tested by this run."
                result["status"] = "PASS"
    except Exception as error:
        result["error"] = f"{type(error).__name__}: {error}"
    finally:
        result["completed_utc"] = datetime.now(timezone.utc).isoformat()
        result["log"] = str((folder / "serial.log").relative_to(ROOT)).replace("\\", "/")
        if (folder / "serial.log").is_file():
            result["log_sha256"] = hashlib.sha256((folder / "serial.log").read_bytes()).hexdigest()
        (folder / "result.json").write_text(json.dumps(result, indent=2) + "\n")
        print(json.dumps(result, indent=2))
        print(f"Evidence: {folder}")
    return 0 if result["status"] == "PASS" else 1

if __name__ == "__main__":
    raise SystemExit(main())
