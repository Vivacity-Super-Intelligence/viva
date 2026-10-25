"""Timestamped passive serial capture; does not claim hardware acceptance."""
import argparse
from datetime import datetime, timezone
from pathlib import Path
import time
import serial

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", required=True)
    parser.add_argument("--seconds", type=int, default=30)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with serial.Serial(args.port, 115200, timeout=0.5) as port, args.output.open("w", encoding="utf-8") as log:
        end = time.monotonic() + args.seconds
        while time.monotonic() < end:
            raw = port.readline()
            if raw:
                line = datetime.now(timezone.utc).isoformat() + " " + raw.decode("ascii", errors="replace").rstrip()
                print(line)
                log.write(line + "\n")
                log.flush()

if __name__ == "__main__":
    main()
