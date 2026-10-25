"""Validate JSON schemas, cross-field invariants, and physical evidence claims."""
import argparse
import json
from pathlib import Path
from jsonschema import Draft202012Validator, FormatChecker
from referencing import Registry, Resource

ROOT = Path(__file__).resolve().parents[2]
NAMES = {"core": 1, "diagnostics": 2, "telemetry": 4, "configuration": 8,
         "protocol": 16, "watchdog": 32, "osal": 64}

def registry():
    result = Registry()
    for file in (ROOT / "schemas").glob("*.json"):
        schema = json.loads(file.read_text())
        Draft202012Validator.check_schema(schema)
        result = result.with_resource(schema["$id"], Resource.from_contents(schema))
    return result

def validate(profile, evidence_root=ROOT):
    schema = json.loads((ROOT / "schemas/controller-profile.schema.json").read_text())
    Draft202012Validator(schema, registry=registry(), format_checker=FormatChecker()).validate(profile)
    caps = profile["capabilities"]
    if {c["name"]: c["bit"] for c in caps} != NAMES or len({c["name"] for c in caps}) != 7:
        raise ValueError("Capability names and bits must match protocol v1")
    implemented = sum(c["bit"] for c in caps if c["firmware_implemented"])
    configured = sum(c["bit"] for c in caps if c["configured"])
    cfg = profile["config"]
    if cfg["capabilities"] != configured or configured & 31 != 31 or configured & ~implemented:
        raise ValueError("Capability masks disagree")
    if cfg["watchdog_timeout_ms"] < 2 * cfg["telemetry_period_ms"]:
        raise ValueError("Watchdog must allow two telemetry periods")
    expected = {"esp32s3": ("FreeRTOS", "ESP-IDF v5.5.4"), "nucleo_g491re": ("Zephyr", "Zephyr bc5d59d1b6657cfeef30124b818c8fbd69da68ac")}
    if (profile["rtos"], profile["framework"]) != expected[profile["platform"]]:
        raise ValueError("Platform/framework mismatch")
    for cap in caps:
        if cap["tested"] or cap["verified"]:
            if not cap["evidence"]:
                raise ValueError("Tested capability requires evidence")
            path = (evidence_root / cap["evidence"]).resolve()
            if not path.is_relative_to(evidence_root.resolve()):
                raise ValueError("Evidence must remain in repository")
            evidence = json.loads(path.read_text())
            if (not evidence.get("physical") or evidence.get("platform") != profile["platform"]
                    or evidence.get("status") != "PASS" or evidence.get("version") != "0.1.0"
                    or cap["name"] not in evidence.get("passed_capabilities", [])):
                raise ValueError("Physical evidence does not substantiate capability")
    return True

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("profiles", nargs="*", type=Path)
    args = parser.parse_args()
    files = args.profiles or sorted((ROOT / "hardware/profiles").glob("*.json"))
    if not files:
        raise SystemExit("No profiles found")
    for file in files:
        validate(json.loads(file.read_text()))
        print(f"PASS {file.name}")

if __name__ == "__main__":
    main()
