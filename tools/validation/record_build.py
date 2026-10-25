"""Run a build command and retain its actual exit code, source ID, and output."""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import subprocess
import time
from source_identity import source_identity, ROOT

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--target", choices=["native", "sanitize", "esp32s3", "nucleo_g491re"], required=True)
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.command
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        parser.error("A real build command is required")
    folder = ROOT / "tests/integration/artifacts" / (datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "-" + args.target)
    folder.mkdir(parents=True)
    start = time.monotonic()
    source = source_identity()
    with (folder / "build.log").open("w", encoding="utf-8") as log:
        # Pipe across the WSL boundary: direct inherited file handles can give
        # stdout/stderr independent offsets and overwrite diagnostic text.
        process = subprocess.Popen(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding="utf-8", errors="replace")
        for line in process.stdout:
            log.write(line)
            log.flush()
        process.wait()
    result = {"target": args.target, "status": "PASS" if process.returncode == 0 else "FAIL",
              "exit_code": process.returncode, "command": command, "build_id": source,
              "completed_utc": datetime.now(timezone.utc).isoformat(),
              "duration_seconds": round(time.monotonic() - start, 2),
              "source_unchanged": source == source_identity(),
              "log": (folder / "build.log").relative_to(ROOT).as_posix()}
    if not result["source_unchanged"]:
        result["status"] = "FAIL"
    (folder / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result, indent=2))
    print((folder / "build.log").read_text(encoding="utf-8", errors="replace")[-6000:])
    return 0 if result["status"] == "PASS" else 1

if __name__ == "__main__":
    raise SystemExit(main())
