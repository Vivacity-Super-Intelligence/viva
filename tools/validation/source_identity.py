"""Firmware source fingerprint shared by build, physical evidence, release gate."""
import hashlib
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]

def source_identity():
    digest = hashlib.sha256()
    candidates = list((ROOT / "firmware").rglob("*"))
    candidates += [ROOT / name for name in ["VERSION", "CMakeLists.txt", "CMakePresets.json", "tools/build/toolchains.json"]]
    candidates += list((ROOT / "tools/build").glob("*.sh"))
    for path in sorted(candidates, key=lambda p: p.relative_to(ROOT).as_posix()):
        relative = path.relative_to(ROOT)
        if path.is_file() and (relative.parts[0] != "firmware" or "build" not in relative.parts) and path.name not in {"sdkconfig", "sdkconfig.old"}:
            digest.update(path.relative_to(ROOT).as_posix().encode())
            digest.update(b"\0")
            digest.update(path.read_bytes().replace(b"\r\n", b"\n"))
    return "src-" + digest.hexdigest()[:16]

if __name__ == "__main__":
    print(source_identity())
