"""Fail closed unless current-source builds and both physical acceptances passed."""
import json
import hashlib
from pathlib import Path
from source_identity import ROOT, source_identity
from validate_profiles import validate, NAMES

def main():
    source = source_identity()
    missing = []
    builds = [json.loads(p.read_text()) for p in (ROOT / "tests/integration/artifacts").glob("*/result.json")]
    for target in ["native", "sanitize", "esp32s3", "nucleo_g491re"]:
        if not any(r.get("target") == target and r.get("build_id") == source and r.get("status") == "PASS" and r.get("source_unchanged") for r in builds):
            missing.append(f"current-source {target} build/tests")
    profile_files = sorted((ROOT / "hardware/profiles").glob("*.json"))
    if {p.stem for p in profile_files} != {"esp32s3", "nucleo_g491re"}:
        missing.append("both exact hardware profiles")
    for file in profile_files:
        p = json.loads(file.read_text())
        validate(p)
        for cap in p["capabilities"]:
            if not cap["verified"]:
                missing.append(f"{p['platform']} {cap['name']} physical evidence")
                continue
            record = json.loads((ROOT / cap["evidence"]).read_text())
            if record.get("build_id") != source or not record.get("checks") or not all(record["checks"].values()) or set(record.get("passed_capabilities", [])) != set(NAMES):
                missing.append(f"{p['platform']} stale/incomplete acceptance")
            log = ROOT / record["log"]
            if not log.is_file() or hashlib.sha256(log.read_bytes()).hexdigest() != record.get("log_sha256"):
                missing.append(f"{p['platform']} serial log integrity")
    if missing:
        raise SystemExit("VIVA FOUNDATION V0.1 - PARTIALLY COMPLETE\n" + "\n".join(missing))
    print(f"PASS local release evidence for {source}. CI and GitHub publishing must also pass before tagging.")

if __name__ == "__main__":
    main()
