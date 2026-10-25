"""Generate honest current-source physical and cross-platform summaries."""
from datetime import datetime, timezone
import json
from pathlib import Path
import argparse
from source_identity import ROOT, source_identity

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--update-profiles", action="store_true")
    args = parser.parse_args()
    source = source_identity()
    physical = {}
    for path in sorted((ROOT / "tests/hardware/artifacts").glob("*/result.json")):
        result = json.loads(path.read_text())
        if result.get("build_id") == source:
            physical[result["platform"]] = (path, result)
    builds = {}
    for path in sorted((ROOT / "tests/integration/artifacts").glob("*/result.json")):
        result = json.loads(path.read_text())
        if result.get("build_id") == source:
            builds[result["target"]] = (path, result)
    lines = ["# Physical validation report", "", f"Generated UTC: {datetime.now(timezone.utc).isoformat()}",
             f"Firmware: 0.1.0; source fingerprint: `{source}`.", "",
             "This report summarizes actual captured results. Missing tests remain unverified.", "",
             "| Target | Build | Physical acceptance |", "| --- | --- | --- |"]
    for platform in ["esp32s3", "nucleo_g491re"]:
        build = builds.get(platform)
        run = physical.get(platform)
        lines.append(f"| {platform} | {build[1]['status'] if build else 'NOT RECORDED'} | {run[1]['status'] if run else 'NOT RUN'} |")
    for platform in ["esp32s3", "nucleo_g491re"]:
        lines += ["", f"## {platform}", ""]
        run = physical.get(platform)
        if not run:
            lines += ["No passing physical acceptance record for the current firmware source.", "Consult detection logs under `tests/hardware/artifacts/` for actual connection attempts."]
            continue
        path, result = run
        lines += [f"Status: **{result['status']}**. Started UTC: {result['timestamp_utc']}; completed: {result['completed_utc']}.",
                  f"Serial interface: {result['port']}; USB identity: `{json.dumps(result.get('usb_identity', {}))}`.",
                  f"Device response: `{json.dumps(result.get('device_identity', {}))}`.",
                  f"Binary SHA256: `{result['binary_sha256']}`.",
                  f"Observation: {result['observation_seconds']} seconds; uptime {result.get('uptime_start_ms', 'unknown')} to {result.get('uptime_end_ms', 'unknown')} ms; {result.get('telemetry_samples', 0)} telemetry samples.",
                  f"Raw serial: [{result['log']}](../../{result['log']}).",
                  f"Structured evidence: [{path.name}](../../{path.relative_to(ROOT).as_posix()}).", "",
                  "| Check | Outcome |", "| --- | --- |"]
        lines += [f"| {name} | {'PASS' if passed else 'FAIL'} |" for name, passed in result.get("checks", {}).items()]
        if result.get("error"):
            lines += ["", "Failure: " + result["error"]]
        lines += ["", result.get("watchdog_scope", "Watchdog behavior not verified by a completed run.")]
        if args.update_profiles and result["status"] == "PASS":
            profile_path = ROOT / f"hardware/profiles/{platform}.json"
            profile = json.loads(profile_path.read_text())
            for cap in profile["capabilities"]:
                if cap["name"] in result["passed_capabilities"]:
                    cap.update(tested=True, verified=True, evidence=path.relative_to(ROOT).as_posix())
            profile_path.write_text(json.dumps(profile, indent=2) + "\n")
    lines += ["", "## Interpretation and limits", "",
              "No motors or external actuators are configured or used. ACTIVE is a logical state.",
              "Passing watchdog evidence covers installation, feeding and liveness beyond its configured timeout; it does not prove reset-on-starvation, electrical safety, brownout recovery, or long endurance.",
              "Early failed detection/build/acceptance logs are retained and do not count as successful validation.",
              "The NUCLEO initially enumerated ST-LINK but failed MCU attachment. A subsequent probe measured near-zero target voltage. After the owner restored board jumpers, SWD attached successfully at about 3.32 V. Logs retain both failure and recovery.",
              "Publication and release status must also be checked against GitHub and the final engineering report.", ""]
    (ROOT / "docs/testing/PHYSICAL_VALIDATION_REPORT.md").write_text("\n".join(lines))
    comparison = ["# Cross-platform comparison", "", f"Source: `{source}`; version 0.1.0.", "",
                  "| Behavior | ESP32-S3 | NUCLEO-G491RE |", "| --- | --- | --- |"]
    for check in ["boot_identity", "selftest", "platform_identity", "firmware_version", "ready_health", "capabilities", "configuration_invalid", "configuration_write", "fault_transition", "fault_blocks_active", "recovery", "fault_diagnostics", "active_transition", "uptime_progression", "watchdog_fed_observation"]:
        values = []
        for platform in ["esp32s3", "nucleo_g491re"]:
            run = physical.get(platform)
            value = run[1].get("checks", {}).get(check) if run else None
            values.append("PASS" if value is True else "FAIL" if value is False else "NOT RUN")
        comparison.append(f"| {check} | {' | '.join(values)} |")
    comparison += ["", "## Expected platform differences", "",
                   "FreeRTOS timing uses the ESP microsecond timer; Zephyr uses its 64-bit millisecond uptime API. Both expose monotonic milliseconds.",
                   "ESP32-S3 uses task watchdog subscription and UART0 via its detected USB bridge. STM32 uses the independent IWDG and the default LPUART1 via ST-LINK VCP.",
                   "Static task storage and critical section implementations use native RTOS types. Scheduler tick rounding and boot duration may differ; exact timing equality is not required.",
                   "Both compile the same portable core, protocol, isolated 18-assertion self-test and common runtime. Native test processes are not a substitute for either physical column.", ""]
    (ROOT / "hardware/compatibility").mkdir(parents=True, exist_ok=True)
    (ROOT / "hardware/compatibility/CROSS_PLATFORM_REPORT.md").write_text("\n".join(comparison))
    print(f"Reports generated for {source}")

if __name__ == "__main__":
    main()
