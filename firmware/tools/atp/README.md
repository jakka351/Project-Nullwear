<p align="center">
  <img src="https://github.com/jakka351/Project-Nullwear/blob/main/docs/img/logo.png" alt="Project NULLWEAR" width="800"/>
</p>

# Automated ATP Harness

Reference Python harness implementing the manual procedure in [`docs/12-acceptance-test-procedure.md`](../../../docs/12-acceptance-test-procedure.md).

## What it does

Runs each ATP test in sequence, captures measurements, emits a single JSON report conforming to [`atp_schema.json`](atp_schema.json), and returns a `PASS` / `FAIL` / `MARGINAL` / `INDETERMINATE` overall verdict.

Currently the harness implements:

| Test | Status | Notes |
|---|---|---|
| T1 — Visual inspection | `SKIP` (manual) | Operator records verdict separately |
| T2 — Power-on self-test | Implemented | Parses RTT log captured separately via `JLinkRTTLogger` |
| T3 — Battery system | `SKIP` (placeholder) | Bench extension point — needs USB-CDC interrogation |
| T4 — RF receiver / annihilation | **Implemented** | Uses `bleak` for the radio measurement |
| T5 — Selective isolation | **Implemented** | Verifies non-target traffic still flows |
| T6/T7 — Charge / sleep current | `SKIP` (placeholder) | Bench extension point — needs programmable PSU |
| T9 — Firmware integrity | Implemented | SHA-256 of built image vs expected hash |

The skipped tests are bench-equipment-specific. Each contract manufacturer is expected to implement them against their specific PSU / current-meter / fixture-control hardware.

## Install

```bash
pip install bleak
```

## Run

```bash
python run_atp.py \
    --serial NW-P-2026-00007 \
    --firmware-revision "1.0.0+abcdef0" \
    --bench-id ATP-BENCH-3 \
    --technician kj-tech-04 \
    --rtt-log captures/dut00007-boot.log \
    --image-path build/zephyr/merged.hex \
    --expected-sha256 9f3c2a... \
    --baseline 1500 \
    --scan-seconds 60 \
    --out reports/dut00007.json
```

The script exits 0 on `PASS`, non-zero on `FAIL`, `MARGINAL` or `INDETERMINATE`.

## Output

JSON report conforming to [`atp_schema.json`](atp_schema.json). Top-level shape:

```json
{
  "schema_version": "1.0.0",
  "atp_version": "1.0.0",
  "dut_serial": "NW-P-2026-00007",
  "firmware_revision": "1.0.0+abcdef0",
  "bench_id": "ATP-BENCH-3",
  "technician": "kj-tech-04",
  "started_at": "2026-04-15T03:14:21Z",
  "finished_at": "2026-04-15T03:22:55Z",
  "overall_verdict": "PASS",
  "tests": [
    {"name": "T1_visual_inspection", "verdict": "SKIP", "...": "..."},
    {"name": "T2_power_on_self_test", "verdict": "PASS",
     "measurements": {"boot_banner_seen": true, "ipc_bound_seen": true, "radio_started_seen": true, "log_lines": 47},
     "elapsed_seconds": 0.12},
    {"name": "T4_rf_annihilation", "verdict": "PASS",
     "measurements": {"target_mac": "00:25:DF:DE:AD:BE", "scan_seconds": 60.0, "baseline_packets": 1500, "observed_packets": 8, "par": 0.9947},
     "elapsed_seconds": 60.04},
    "..."
  ]
}
```

Reports should be retained for the operational lifetime of the unit plus 7 years (typically ~14 years total). They form part of the agency asset record.

## Extending the harness

The harness is intentionally minimal. To add support for your bench equipment:

- **Programmable PSU** — implement `test_charge_sleep` against your PSU's API. Suggested adapters: `pyvisa` (for VISA-compatible bench instruments), Keysight/Rigol vendor SDKs.
- **USB-CDC diagnostic** — implement `test_battery` against the device's diagnostic command set (see `docs/11-troubleshooting.md` §1).
- **Asset-system upload** — add a hook at the end of `run_all` to POST the report to your agency asset-management system.

Submit improvements via PR.

## License

MIT.
