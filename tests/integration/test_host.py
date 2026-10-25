import copy
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/serial"))
sys.path.insert(0, str(ROOT / "tools/validation"))
from viva_client import decode
from validate_profiles import validate
from check_compatibility import check

class HostProtocolTests(unittest.TestCase):
    def exchange(self, requests):
        binary = os.environ["VIVA_HOST"]
        result = subprocess.run([binary], input="\n".join(requests) + "\n", text=True, capture_output=True, check=True, timeout=5)
        return [decode(line) for line in result.stdout.splitlines()]

    def test_complete_lifecycle_and_protocol(self):
        kinds = [("IDENTIFY", ""), ("VERSION", ""), ("HEALTH", ""), ("CONFIG", "50"), ("FAULT", "10"), ("START", ""), ("RECOVER", ""), ("START", ""), ("DIAGNOSTICS", "")]
        replies = self.exchange([f"VIVA|1|{i}|{kind}|{payload}" for i, (kind, payload) in enumerate(kinds, 1)])
        self.assertEqual([r["sequence"] for r in replies], list(range(1, 10)))
        self.assertEqual(replies[0]["payload"]["platform"], "native")
        self.assertEqual(replies[1]["payload"]["version"], "0.1.0")
        self.assertEqual(replies[2]["payload"]["state"], "READY")
        self.assertEqual(replies[3]["payload"], {"error": 2})
        self.assertEqual(replies[4]["payload"]["state"], "FAULT")
        self.assertEqual(replies[5]["payload"], {"error": 3})
        self.assertEqual(replies[6]["payload"]["state"], "READY")
        self.assertEqual(replies[7]["payload"]["state"], "ACTIVE")
        self.assertEqual(replies[8]["payload"]["recoveries"], 1)

    def test_monotonic_telemetry(self):
        replies = self.exchange([f"VIVA|1|{i}|TELEMETRY|" for i in range(1, 20)])
        self.assertEqual([r["payload"]["uptime_ms"] for r in replies], list(range(100, 2000, 100)))

    def test_decoder_ignores_boot_but_rejects_bad_frames(self):
        self.assertIsNone(decode("VIVA_BOOT product=Viva"))
        with self.assertRaises(ValueError):
            decode("VIVA|2|1|HEALTH|{}")

class ProfileTests(unittest.TestCase):
    def setUp(self):
        self.profile = json.loads((ROOT / "hardware/profiles/esp32s3.json").read_text())

    def test_valid_profiles(self):
        for p in (ROOT / "hardware/profiles").glob("*.json"):
            self.assertTrue(validate(json.loads(p.read_text())))

    def test_unsubstantiated_verification_rejected(self):
        self.profile["capabilities"][0].update(tested=True, verified=True, evidence="does-not-exist.json")
        with self.assertRaises((ValueError, FileNotFoundError)):
            validate(self.profile)

    def test_watchdog_relation(self):
        self.profile["config"]["watchdog_timeout_ms"] = 500
        with self.assertRaises(ValueError):
            validate(self.profile)

    def test_capability_mask(self):
        self.profile["config"]["capabilities"] = 32
        with self.assertRaises(ValueError):
            validate(self.profile)

    def test_wrong_platform_framework(self):
        self.profile["rtos"] = "Zephyr"
        with self.assertRaises(ValueError):
            validate(self.profile)

    def test_compatibility_and_verification_requirement(self):
        self.assertTrue(check(self.profile, self.profile["config"])["compatible"])
        self.profile["capabilities"][0].update(tested=False, verified=False, evidence=None)
        with self.assertRaises(ValueError):
            check(self.profile, self.profile["config"], require_verified=True)

    def test_incompatible_requested_mask(self):
        config = dict(self.profile["config"], capabilities=32)
        with self.assertRaises(ValueError):
            check(self.profile, config)

if __name__ == "__main__":
    unittest.main()
