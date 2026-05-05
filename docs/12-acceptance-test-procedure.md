# Acceptance Test Procedure (ATP)

**Audience:** the contract manufacturer's test technician, the depot's intake QA officer, and any independent verifier.

**Purpose:** to determine whether a single NULLWEAR/P unit, fresh from manufacture or after refurbishment, is fit for issue.

This is the bench-level test. It is necessary but not sufficient — every unit must also pass [`10-field-testing-protocol.md`](10-field-testing-protocol.md) before issue to officers.

---

## Equipment per test bench

- One **DUT** (device under test).
- One **reference test source** (ESP32 emulator, MAC `00:25:DF:DE:AD:BE`, fixed location 1 m from the DUT).
- One **reference receiver** (Linux laptop running `ref_receiver.py`, fixed location 1 m on the other side of the DUT, line-of-sight).
- One **bench USB-C power supply** capable of 5 V / 1 A.
- One **digital multimeter** (for charge-current measurements).
- One **bench J-Link** (for firmware version verification).
- One **computer** running the ATP test harness (a Python script that orchestrates the receiver and emits a pass/fail report).
- A **shielded RF enclosure** is recommended but not required; a quiet RF environment (no Wi-Fi closer than 5 m) is sufficient.

---

## Test sequence

The full ATP takes approximately 8 minutes per unit, of which most time is automated.

### Test 1 — Visual inspection

| Check | Pass criterion |
|---|---|
| Enclosure intact, no cracks, no weld seam gaps | Pass / Fail |
| LED light pipe flush with enclosure | Pass / Fail |
| USB-C port clean, no debris, no bent pins | Pass / Fail |
| Belt clip / mounting holes M3-thread present | Pass / Fail |
| Serial number laser-etched legibly | Pass / Fail |

### Test 2 — Power-on self-test

Connect to USB. Within 2 seconds, the LED must illuminate. Within 5 seconds, the firmware must complete boot. Connect via J-Link RTT and verify:

- Boot banner present, firmware revision matches expected.
- `nullwear_app: Net-core IPC bound` log present.
- `nullwear_net: Starting radio jammer` log present.
- No ERROR-level log lines.

Pass / Fail.

### Test 3 — Battery system

| Measurement | Pass criterion |
|---|---|
| VBAT at startup (read via fuel gauge) | ≥ 3.7 V |
| Reported state-of-charge | ≥ 65% |
| MAX17048 detected and version-read OK | Pass |
| Charger STAT pin reads "charging" when USB connected | Pass |
| Quiescent current with USB disconnected and radio idle | ≤ 100 µA |

### Test 4 — RF receiver sensitivity

DUT placed at 5 m from the test source, line-of-sight. Test source confirmed to be broadcasting (run `ref_receiver.py --duration 30` from 1 m before placing the DUT, expect ~900 packets).

Power off the test source. Wait 5 seconds for receiver bleed-off.

Power on the test source.

Run on the receiver:

```bash
python ref_receiver.py --duration 60 --baseline-target-rx 1500 --out-json atp_test4.json
```

Expected: with a working DUT in the path, target_packets_received ≤ 15. PAR ≥ 0.99.

Pass / Fail.

### Test 5 — Selective isolation

While the test source remains active, also activate **two non-target BLE devices** in the test area:

- A smartphone with Bluetooth on, advertising as a generic device.
- A second ESP32 emulator broadcasting MAC `02:00:00:00:00:01` (a non-Axon OUI).

Run a second receiver scan:

```bash
python ref_receiver.py --duration 60 --out-json atp_test5.json
```

Verify:

- Target packets (00:25:DF:DE:AD:BE): ≤ 15 (still being annihilated).
- Non-target packets: in line with their normal advertising rates (~tens to hundreds per device per minute).

Failure mode: if non-target packets are also being suppressed, the OUI matcher is malfunctioning. RMA the unit.

Pass / Fail.

### Test 6 — Charge cycle

Connect DUT to bench USB-C supply. Confirm:

| Measurement | Pass criterion |
|---|---|
| Charge current at start (battery low) | 400–500 mA |
| Charge current at 90% SoC | 80–150 mA |
| Charge current at 100% SoC (sustained) | ≤ 1 mA (trickle / cutoff) |
| Time from < 5% to 100% | ≤ 70 minutes |
| No abnormal heating (case > 45 °C) at any point | Pass |

### Test 7 — Sleep current

Disconnect USB. Place unit in deep-sleep mode (long-press button 3 seconds; net core should remain awake; app core enters STOP mode).

Measure DUT current draw with multimeter in series with battery.

| State | Pass criterion |
|---|---|
| Deep sleep (radio off) | ≤ 5 µA |
| Idle (radio scanning, no traffic) | ≤ 4 mA |

### Test 8 — Mechanical (sample-rate)

Sample 1% of every batch:

| Test | Method | Pass criterion |
|---|---|---|
| IP67 immersion | 1 m water, 30 min, then re-run Tests 2–4 | All pass |
| Drop test | 1.5 m onto concrete, 6 faces | No cosmetic damage > 1 mm; functional pass |
| Temperature | –10 °C and +55 °C operation | Functional pass |

### Test 9 — Firmware integrity

Connect to USB-CDC. Issue command `version`. Verify response matches expected build SHA256 hash.

For signed images, verify the MCUboot signature is valid by attempting to flash an unsigned test image — the bootloader must reject it.

Pass / Fail.

### Test 10 — Final visual + serial registration

Re-inspect after testing. Record serial number, firmware version, all test results, and pass/fail verdict in the asset system.

---

## Automated test harness

A Python script (`tools/atp/run_atp.py`) automates Tests 2–7 and emits a JSON report. To be supplied by the contract manufacturer's test rig integration:

```python
result = atp.run_full(dut_serial="NW-P-2026-00001",
                     bench_id="ATP-BENCH-3")
assert result.verdict == "PASS"
print(result.summary)
```

The Python script:
1. Connects to the bench fixtures (programmable USB power, current meter, J-Link, RTT).
2. Programs the DUT with the production firmware image.
3. Runs each test in sequence.
4. Captures all measurements.
5. Emits a signed PASS/FAIL report.

---

## Acceptance threshold

A unit passes ATP only if **all** Tests 1–7, 9, 10 are individually pass, and the unit is in a sample that has met the sample-rate criteria for Test 8.

Reject rate budgeting: with mature manufacturing, the expected reject rate at ATP is ≤ 1%. Higher rates indicate a manufacturing process drift; investigate.

---

## Reporting

Per-unit ATP reports must be retained for the operational lifetime of the unit plus 7 years (i.e. typically ~14 years total). They form part of the agency asset record.

Aggregate ATP statistics (reject rate, common reject causes) should be reported monthly by the contract manufacturer to the agency procurement officer.

Cross-reference: [`09-operations-manual.md`](09-operations-manual.md) §1 — Receipt and intake.
