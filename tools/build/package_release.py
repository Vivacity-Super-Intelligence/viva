"""Package existing, accepted binaries; never stages generated binaries in Git."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import zipfile
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/validation"))
from source_identity import source_identity

def main():
    subprocess.run([sys.executable, str(ROOT / "tools/validation/release_gate.py")], check=True)
    out = ROOT / "build/release"
    out.mkdir(parents=True, exist_ok=True)
    source = source_identity()
    groups = {
        "esp32s3": [("viva.bin", ROOT / "build/esp32s3/viva.bin"),
                     ("bootloader/bootloader.bin", ROOT / "build/esp32s3/bootloader/bootloader.bin"),
                     ("partition_table/partition-table.bin", ROOT / "build/esp32s3/partition_table/partition-table.bin"),
                     ("flasher_args.json", ROOT / "build/esp32s3/flasher_args.json")],
        "nucleo_g491re": [("zephyr." + ext, ROOT / ("build/stm32g491re/zephyr/zephyr." + ext)) for ext in ["bin", "hex", "elf"]],
    }
    for platform, files in groups.items():
        binary = files[0][1]
        digest = hashlib.sha256(binary.read_bytes()).hexdigest()
        records = [json.loads(p.read_text()) for p in (ROOT / "tests/hardware/artifacts").glob("*/result.json")]
        if not any(r.get("platform") == platform and r.get("status") == "PASS" and r.get("build_id") == source and r.get("binary_sha256") == digest for r in records):
            raise SystemExit("Current binary does not match accepted hardware image: " + platform)
        archive = out / f"viva-v0.1.0-{platform}.zip"
        manifest = {"product": "Viva", "version": "0.1.0", "platform": platform, "build_id": source,
                    "engineering_foundation": True, "config_version": 1, "protocol_version": 1,
                    "sha256": {name: hashlib.sha256(p.read_bytes()).hexdigest() for name, p in files}}
        with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_DEFLATED) as z:
            for name, file in files:
                z.write(file, name)
            z.writestr("manifest.json", json.dumps(manifest, indent=2) + "\n")
            z.write(ROOT / "docs/deployment/BUILD_AND_FLASH.md", "BUILD_AND_FLASH.md")
        print(archive)
    sums = [hashlib.sha256(p.read_bytes()).hexdigest() + "  " + p.name for p in sorted(out.glob("*.zip"))]
    (out / "SHA256SUMS.txt").write_text("\n".join(sums) + "\n")

if __name__ == "__main__":
    main()
