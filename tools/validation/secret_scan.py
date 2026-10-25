"""Conservative source scan, supplementary to manual staging review."""
from pathlib import Path
import re
import subprocess
ROOT = Path(__file__).resolve().parents[2]
PATTERNS = [
    re.compile(rb"gh[pousr]_" + rb"[A-Za-z0-9]{30,}"),
    re.compile(rb"github_pat_" + rb"[A-Za-z0-9_]{40,}"),
    re.compile(rb"AKIA" + rb"[0-9A-Z]{16}"),
    re.compile(rb"-----BEGIN " + rb"(?:RSA |EC |OPENSSH )?PRIVATE KEY-----"),
    re.compile(rb"sk-" + rb"[A-Za-z0-9_-]{32,}"),
]

def main():
    files = subprocess.check_output(["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"], cwd=ROOT).decode().split("\0")
    bad = []
    for name in filter(None, files):
        p = ROOT / name
        if p.is_file() and any(pattern.search(p.read_bytes()) for pattern in PATTERNS):
            bad.append(name)
    if bad:
        raise SystemExit("Potential secret in: " + ", ".join(bad))
    print("PASS source secret patterns (manual review still required)")

if __name__ == "__main__":
    main()
