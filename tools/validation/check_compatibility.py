"""Check a deployment config against one board's implemented/verified capability set."""
import argparse
import json
from pathlib import Path
from jsonschema import Draft202012Validator
from validate_profiles import ROOT, registry, validate

def check(profile, config, require_verified=False):
    validate(profile)
    schema = json.loads((ROOT / "schemas/firmware-config.schema.json").read_text())
    Draft202012Validator(schema, registry=registry()).validate(config)
    implemented = sum(c["bit"] for c in profile["capabilities"] if c["firmware_implemented"] and c["hardware_supported"])
    verified = sum(c["bit"] for c in profile["capabilities"] if c["verified"])
    requested = config["capabilities"]
    if requested & 31 != 31 or requested & ~implemented:
        raise ValueError("Requested capabilities are not implemented by target")
    if config["watchdog_timeout_ms"] < 2 * config["telemetry_period_ms"]:
        raise ValueError("Watchdog must allow two telemetry periods")
    if require_verified and requested & ~verified:
        raise ValueError("Requested capabilities lack physical verification")
    return {"platform": profile["platform"], "compatible": True, "requires_verified": require_verified, "capabilities": requested}

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", type=Path, required=True)
    parser.add_argument("--config", type=Path)
    parser.add_argument("--require-verified", action="store_true")
    args = parser.parse_args()
    profile = json.loads(args.profile.read_text())
    config = json.loads(args.config.read_text()) if args.config else profile["config"]
    print(json.dumps(check(profile, config, args.require_verified), indent=2))

if __name__ == "__main__":
    main()
