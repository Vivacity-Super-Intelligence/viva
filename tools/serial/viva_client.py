"""Small synchronous development protocol client. No credentials or network IO."""
import json
import time

def decode(line):
    if not line.startswith("VIVA|"):
        return None
    fields = line.rstrip("\r\n").split("|")
    if len(fields) != 5 or fields[1] != "1" or not fields[2].isdigit():
        raise ValueError("Malformed Viva frame")
    return {"sequence": int(fields[2]), "type": fields[3], "payload": json.loads(fields[4])}

class Client:
    def __init__(self, serial, log):
        self.serial, self.log, self.sequence = serial, log, 0

    def line(self):
        raw = self.serial.readline()
        if raw:
            text = raw.decode("ascii", errors="replace")
            self.log.write(text)
            self.log.flush()
            return text
        return ""

    def request(self, kind, payload="", timeout=4, expect_error=False, version=1):
        self.sequence += 1
        frame = f"VIVA|{version}|{self.sequence}|{kind}|{payload}\n"
        self.log.write(f"HOST_TX {frame}")
        self.log.flush()
        self.serial.write(frame.encode("ascii"))
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            line = self.line()
            message = decode(line)
            if message and message["sequence"] == self.sequence:
                if (message["type"] == "ERROR") != expect_error:
                    raise AssertionError(f"Unexpected response: {message}")
                if not expect_error and message["type"] != kind:
                    raise AssertionError("Response type does not match request")
                return message["payload"]
        raise TimeoutError(f"No response to {kind}")
