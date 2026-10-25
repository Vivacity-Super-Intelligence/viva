# Validation strategy

Compilation, host tests and physical tests are separate evidence classes.
No hardware test is skipped and reported as PASS. Hosted CI has no physical board.

| Layer | What it proves | Runner |
| --- | --- | --- |
| Portable C tests | Lifecycle, configuration, faults, recovery, uptime, capabilities, error boundaries | Native CTest |
| Protocol C tests | Framing, strict numbers, version errors, overflow, unknown commands, mutation rules | Native CTest |
| Host integration | Python decoder and real compiled C process, equivalent command journey | Python unittest through CTest |
| Schema/profile tests | JSON structure, mask/timing invariants, rejected unsupported evidence claims | Python jsonschema and unittest |
| Sanitized build | Same host behavior under ASan and UBSan | CTest sanitize preset |
| Board builds | Actual ESP-IDF and Zephyr compile/link for exact targets | Local WSL and GitHub Actions |
| Physical acceptance | Flash/reset/boot, shared tests and live serial command behavior on each board | Windows with connected hardware |

The common functional specification is `tests/hardware/FUNCTIONAL_SPEC.md`.
Both target images compile the same 18-assertion core self-test; its isolated test
context allows safe fault and recovery testing without damaging runtime state.
The real runtime is also exercised through the serial host acceptance sequence.

Native testing:

```powershell
wsl -d Ubuntu-24.04 -u root -- bash /mnt/p/VIVACITY/viva-foundation/tools/build/build-wsl.sh native
wsl -d Ubuntu-24.04 -u root -- bash /mnt/p/VIVACITY/viva-foundation/tools/build/build-wsl.sh sanitize
```

Portable C checks include NULL arguments, invalid initialization/configuration,
uint64 uptime crossing the 32-bit boundary, forbidden lifecycle transitions,
failed self-test, failed recovery, clock regression, unknown capabilities,
watchdog timing constraints and saturating fault counts. Protocol tests reject
control bytes, numeric overflow, missing/extra fields and invalid config while
preserving prior values. The Python suite checks full fault/recovery journeys,
monotonic telemetry, decoding, profile mismatch and fabricated evidence paths.

Use `tools/validation/record_build.py --target TARGET -- COMMAND...` to retain a
real command, source ID, exit code, UTC timestamp, duration and complete log under
`tests/integration/artifacts/`. Failed runs are retained. Successful runs from an
older source fingerprint do not satisfy the local release gate.

Physical acceptance is fail-closed: missing port, incorrect ST-LINK, failed reset,
missing boot line, wrong version/build, failed self-test, faulty health, absent
reply or stalled uptime produces FAIL with an error. The default observation
lasts 60 seconds, at least six watchdog periods. Minimum accepted duration is 30
seconds. The script captures all serial text and each transmitted request.
It verifies watchdog start/feed/liveness; deliberate reset-on-starvation is
outside that specific test and must not be inferred from its PASS.

The release evidence gate requires current-source native, sanitizer, ESP32-S3 and
NUCLEO builds plus all seven physically verified capabilities on each target:

```powershell
python tools/validation/release_gate.py
python tools/validation/secret_scan.py
```

Additionally review source/staging, ignored files, GitHub remote, default branch
and CI. Tag v0.1.0 only after both boards pass and published CI is green. Physical
evidence stays tied to firmware source fingerprint and binary SHA256, so docs-only
commits do not invalidate a board run. Changing firmware requires a rebuild and
fresh physical acceptance.

Limitations: no exhaustive RTOS scheduling verification, brownout/EMI/thermal
testing, formal proof, persistent fault storage, real-time certification, long
endurance run, automated screen-reader UI checks (there is no UI), or functional
safety certification. Re-run physical tests after USB routing/jumper changes.
