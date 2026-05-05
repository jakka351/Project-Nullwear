# Acceptance Test — Summary for Bidders

The full procedure is at [`docs/12-acceptance-test-procedure.md`](../docs/12-acceptance-test-procedure.md). This summary tells the bidder what test fixtures they need to build and what level of test capability they must have.

## Tests the CM must run on every unit

| Test | What it verifies | Equipment needed |
|---|---|---|
| T1 Visual | No cosmetic damage; serial laser-etched legibly | Operator visual inspection |
| T2 Power-on | Boots cleanly; firmware boot banner present in RTT log | J-Link + RTT logger |
| T3 Battery | VBAT > 3.7 V, MAX17048 detected | I²C bench probe |
| T4 RF annihilation | PAR ≥ 0.99 against ESP32 emulator at 5 m | RF anechoic chamber + ESP32 source + reference receiver |
| T5 Selective isolation | Non-target BLE traffic passes through | Same as T4 + ≥ 2 unrelated BLE devices |
| T6 Charge cycle | Charge curve within spec | Programmable PSU + ammeter |
| T7 Sleep current | Idle < 100 µA | Same |
| T9 Firmware integrity | SHA-256 of flashed image matches expected | Local hash check |

## Tests at sample rate (1% AQL per batch)

| Test | What it verifies | Equipment needed |
|---|---|---|
| T8a IP67 | 1 m water immersion for 30 min, then re-test T2/T4 | Immersion tank |
| T8b Drop test | 1.5 m onto concrete on each of 6 faces | Drop fixture |
| T8c Temperature | Operation at -10 °C and +55 °C | Environmental chamber |

## Pass criteria

A unit passes only if **all** Tests T1–T7 and T9 are individually pass, AND the unit is in a sample that has met sample-rate criteria for T8.

Reject rate target:
- Pilot batch (first 200): ≤ 5%
- First production batch (next 1,000): ≤ 2%
- Steady-state production: ≤ 1%

## Equipment the CM must build / acquire

The Agency does NOT supply test fixtures. The CM must equip its bench with:

1. **RF anechoic chamber** or quiet RF environment for T4/T5
2. **ESP32-based reference test source** — source code in `firmware/tools/test-source/axon_emulator.ino`
3. **Reference receiver** — Python tool in `firmware/tools/reference-receiver/ref_receiver.py`
4. **J-Link debugger** for RTT log capture
5. **Programmable PSU + DMM** for T6/T7 measurements
6. **Environmental chamber** for T8c
7. **Immersion tank** for T8a
8. **Drop-test fixture** for T8b

A standard electronics-CM bench can run all of T1–T9 with off-the-shelf equipment. The Agency expects this is in scope of the bidder's normal capability — **bidders that need to acquire substantial new equipment to run the ATP should reflect that in the NRE line of their bid**.

## ATP harness

A reference Python harness for orchestrating T1–T9 is at `firmware/tools/atp/run_atp.py`. CMs are expected to integrate it (or implement an equivalent) into their production-line software so that every unit's pass/fail and measurements are captured automatically.

Output format: JSON conforming to `firmware/tools/atp/atp_schema.json`. Sample report: `firmware/tools/atp/example-report.json`.

## What the CM submits with each shipment

For every consigned unit, the CM provides one ATP report JSON file. These are bundled into a per-consignment manifest and transmitted to the Agency's asset-management endpoint.

The Agency reserves the right to re-test 10% of any consignment at its own bench within 14 days of receipt and reject the consignment if the re-test reject rate exceeds the contracted threshold.
