# Field Testing Protocol

**Audience:** the field-test technician responsible for validating that NULLWEAR works in operationally realistic conditions.

**Scope:** how to verify, in the field, that a NULLWEAR/P device actually annihilates the BLE-OUI signal of a real Axon device (or a known test source).

**Result:** a measured Packet Annihilation Ratio (PAR), a pass/fail verdict, and a written report.

---

## Why field testing is mandatory

Lab acceptance testing (`docs/12-acceptance-test-procedure.md`) verifies that a unit meets engineering specifications under controlled bench conditions. Field testing verifies that **the engineering specification is operationally meaningful** — i.e. that the unit works in the messy, multipath, RF-busy real world the officer actually carries it in.

A unit that passes lab ATP but fails field testing is not deployable.

A unit that passes both is.

---

## Equipment list

Per test:

1. **One NULLWEAR/P unit under test.** Pre-charged to ≥ 80%. Pre-passed ATP.
2. **One known BLE source.** Either:
   - **Preferred:** the ESP32-based test source (`firmware/tools/test-source/`) preconfigured with MAC `00:25:DF:DE:AD:BE`. Permits unambiguous identification of the target signal in the receiver log.
   - **Acceptable:** a real Axon device under the agency's control. Read the unit's MAC via Axon's own management software or by direct BLE inspection beforehand.
3. **One reference receiver.** Either:
   - **Preferred:** a Linux laptop running the reference receiver (`firmware/tools/reference-receiver/ref_receiver.py`).
   - **Acceptable:** any second laptop or phone running an off-the-shelf BLE scanner app (e.g. nRF Connect for Mobile), which makes interpretation more manual.
4. **Tape measure** — for setting and confirming distances.
5. **A range of test environments** (see §5).
6. **A field-test logbook** (paper or electronic; template at end of this document).

---

## Pre-test checklist

- [ ] All three devices (NULLWEAR, source, receiver) charged.
- [ ] Reference receiver software installed and tested.
- [ ] Source confirmed to be broadcasting (`ref_receiver.py --duration 30` from 1 m, expect ~900 packets in 30 s).
- [ ] NULLWEAR unit initial state recorded (LED green, serial number logged).
- [ ] Test environment selected.
- [ ] Sufficient time allocated (each test sequence below takes ~15 minutes).

---

## Test sequence — basic verification

### Step 1 — Baseline

Place the test source at a fixed location. Place the reference receiver at the test distance (start with **5 m**). Ensure NULLWEAR is **off** (held button for 3 seconds to enter deep sleep, or simply absent from the test area).

Start the receiver:

```bash
python ref_receiver.py --duration 60 --out-json baseline_5m.json
```

Wait 60 seconds. Record the `target_packets_received` value. This is your baseline.

**Expected baseline at 5 m, source = ESP32 emulator:** ~1500 to ~1800 packets. (Ax Lower if walls / multipath are present.)

If baseline is < 200 packets at 5 m, the source or receiver is faulty — fix this before proceeding. The PAR measurement is only meaningful if the baseline is reliably non-zero.

### Step 2 — Annihilation

Without moving the source or the receiver, place NULLWEAR/P **between** them, ~1 m from the source, oriented vertically (LED facing up). Confirm LED is steady green.

Run the same scan again, this time supplying the baseline number:

```bash
python ref_receiver.py --duration 60 --baseline-target-rx <BASELINE> --out-json annihilation_5m.json
```

Record the resulting PAR value and the verdict.

### Step 3 — Distance walk

Repeat the baseline + annihilation sequence at the following distances between source and receiver:

| Distance | Notes |
|---|---|
| 2 m | Close-range — should give the strongest baseline and the cleanest PAR |
| 5 m | Reference distance for ATP |
| 10 m | Within typical NULLWEAR/P bubble |
| 20 m | At edge of typical bubble |
| 50 m | Outside typical NULLWEAR/P bubble — PAR may degrade |

For each distance, place NULLWEAR within 1 m of the **source**, not the receiver. NULLWEAR protects the source by killing its broadcasts; placing NULLWEAR near the receiver is meaningless because the packets have already crossed the air.

### Step 4 — Orientation sensitivity

At 5 m baseline distance, repeat annihilation with NULLWEAR oriented:

| Orientation | PAR |
|---|---|
| LED up (vertical) | (your measurement) |
| LED down (inverted) | (your measurement) |
| Flat horizontal | (your measurement) |
| In a back trouser pocket | (your measurement) |
| In a chest pocket | (your measurement) |

Antenna orientation matters. The acceptance criterion is that PAR ≥ 0.99 holds for at least **two** of the five orientations, and PAR ≥ 0.90 for all five. If PAR < 0.90 in any reasonable wear orientation, the antenna design needs adjustment.

### Step 5 — Mobility

Have a tester carry the source on their belt and walk a measured 100 m loop while the receiver remains stationary. Repeat with NULLWEAR also on their belt. Compare baseline-walk vs. annihilation-walk target packet counts.

This validates the typical operational scenario: an officer moving through the receiver's coverage zone.

---

## Test sequence — adversarial environment

### Step 6 — Multi-source resilience

Set up **multiple non-target BLE devices** in the test area: a smartphone with Bluetooth on, a Bluetooth speaker, a fitness tracker, an unrelated smart-home BLE beacon. These should generate non-OUI traffic that NULLWEAR must NOT corrupt.

Run the receiver on a separate test, this time without the OUI filter, and verify:

```bash
python ref_receiver.py --duration 120
```

You should see all the non-target devices reporting normally. If NULLWEAR is corrupting non-target traffic, the OUI matcher is broken — investigate.

### Step 7 — Wi-Fi co-existence

In an environment with active Wi-Fi (any home or office), repeat Step 1-2. Confirm that NULLWEAR's operation does not degrade Wi-Fi throughput on a co-located access point. Use any Wi-Fi speed test tool to validate.

### Step 8 — RF-busy environment

Repeat Steps 1-2 in:

| Environment | Acceptance |
|---|---|
| Quiet residential street | PAR ≥ 0.99 |
| Inside a busy office | PAR ≥ 0.95 |
| Inside a shopping centre | PAR ≥ 0.90 |
| At a major intersection during peak | PAR ≥ 0.90 |
| Inside a car at highway speed | PAR ≥ 0.90 |

Lower PAR thresholds in high-RF environments are acceptable because the receiver's own ability to capture every packet is also degraded. The acceptance criterion is that the **ratio** between baseline and annihilation remains high.

---

## Test sequence — operational verification

### Step 9 — Real shift simulation

Conduct a 4-hour observation in conditions matching a typical shift:

1. NULLWEAR worn by a tester acting as an officer.
2. ESP32 source on the same tester's belt (simulating an Axon body camera).
3. Reference receiver static at a representative location (e.g. corner of a building).
4. Tester walks a representative beat (a 4 km circuit through residential, commercial, transit areas).
5. Receiver continuously logs.

At end of 4 hours, expected target_packets_received: < 100 (would be tens of thousands without NULLWEAR).

If the receiver records more than ~200 target packets in 4 hours, investigate per-environment log to identify when leakage occurred.

### Step 10 — Multi-officer scenario

With **5 testers** each wearing NULLWEAR + an ESP32 source (each with a unique fake MAC: 00:25:DF:DE:AD:01 through 00:25:DF:DE:AD:05), have all five gather at a common location for 10 minutes. Receiver static at 30 m.

Expected: zero or near-zero target packets received during the 10-minute window. This validates that NULLWEAR's protection bubble works at officer-density typical of a briefing or muster.

---

---

## Test sequence — wearability and 30-day endurance

These tests answer a different question from PAR: "will an officer actually wear this every shift for two years?"

### Step 11 — 30-day in-service trial

A representative cohort of 10 officers is issued NULLWEAR/P units and instructed to wear them on every shift for 30 consecutive days. Each unit is logged daily on:

| Daily log item | Pass criterion |
|---|---|
| Worn for the entire shift? | ≥ 95% of officer-days |
| Returned to dock at end of shift? | ≥ 95% of officer-days |
| Charged successfully overnight? | ≥ 98% of unit-nights |
| Reported faults | ≤ 5 across the cohort over 30 days |
| Officer comfort complaints | None of severity-3 (refused to wear) |
| Officer-reported interference with other equipment | None |
| Lost units | 0 |
| Damaged units | ≤ 1 across the cohort |

### Step 12 — Drop / impact survival

Per ATP §8 the unit is rated for 1.5 m drop onto concrete, 6 faces. In service the relevant test is "did anyone's actually broken?" — log all impacts reported by officers (including incidents like running through brush, falling onto pavement during a chase, etc.) and at end of trial inspect each unit for:

- Enclosure crack or seal compromise
- USB-C port damage
- LED light-pipe damage
- Belt-clip mount failure

Pass: ≥ 90% of units in original cosmetic and functional condition after the 30-day trial.

### Step 13 — Battery degradation

Measure cell voltage at 100% charge for each unit at start of trial and again at end. Acceptable degradation: ≤ 2% capacity loss over 30 cycles (~4–5% per year extrapolated, on track for the 500-cycle service life specified in the engineering spec).

---

## Test sequence — Axon equipment compatibility

These tests answer the question that General Counsel will ask: "does NULLWEAR break the body camera?"

### Step 14 — Body-camera basic operation

For each pilot officer, with NULLWEAR/P clipped to the duty belt within ~30 cm of an Axon Body 3 (or whatever Axon model is in service):

| Function | Test method | Pass criterion |
|---|---|---|
| Manual record activation | Press body-cam button | Recording starts within 1 s |
| Recording continues for full intended duration | Record 5-minute test clip | Recording reaches 5 min |
| Recording uploads to Evidence.com | Re-dock at end of test | Upload completes within agency's normal timeframe |
| Pre-event buffer | Trigger recording, verify pre-buffer captured | Pre-buffer present and intact |
| Audio capture | Speak during test | Audio present in playback |
| Video capture | Record visible test pattern | Video present in playback |
| Battery level reporting | Compare cam display to Evidence.com | Match within ±5% |

### Step 15 — Signal Sidearm (holster trigger)

For officers issued Axon's Signal Sidearm holster sensor:

| Test | Method | Pass criterion |
|---|---|---|
| Holster-trigger auto-record activates | Draw firearm from holster | Body-cam recording starts within 1 s |
| Auto-recording captures full event | Hold draw for 30 s | 30 s of footage available |
| Holster reset triggers correctly | Re-holster | Body cam continues recording per agency policy |

If Signal Sidearm fails inside the NULLWEAR bubble, this is a **STOP-THE-PILOT** finding. Officer-safety capability cannot be degraded by NULLWEAR.

### Step 16 — Axon View / Axon Aware mobile pairing

If the agency uses the Axon View mobile app or Axon Aware live-streaming:

- Confirm body camera pairs to the officer's phone within normal time limits.
- Confirm live-stream initiates and remains stable for ≥ 5 minutes.
- Confirm GPS fix from camera reaches Aware platform.

If pairing FAILS inside NULLWEAR bubble, document specifically: does pairing succeed when NULLWEAR/P is removed temporarily? If yes, this is a real conflict; if no, it's a separate Axon-side issue.

### Step 17 — In-vehicle Axon Fleet

For officers in vehicles equipped with Axon Fleet (in-car video):

- Confirm the in-car unit pairs to the officer's body camera as expected.
- Confirm dual-camera recording engages on light-bar activation (or other agency-defined trigger).
- Confirm both video feeds upload normally at end of shift.

This is the test that involves NULLWEAR/V, not just NULLWEAR/P. Run with both layers of protection active.

---

## Test sequence — evidentiary integrity

These tests produce the documentation the agency's evidence custodian will rely on to assert that NULLWEAR's operation does not affect the chain of custody or admissibility of body-camera evidence.

### Step 18 — Evidence chain-of-custody

For 10 randomly-selected pilot recordings:

| Check | Method | Pass criterion |
|---|---|---|
| Recording captured and uploaded normally | Compare video file vs incident occurred | Video present |
| Video file metadata intact | Inspect Evidence.com metadata | Officer ID, timestamps, GPS, camera serial all populated as normal |
| Video hash unchanged from camera to Evidence.com | Compare hashes | Match |
| Video duration matches expected | Compare to dispatch logs | ±2 seconds |
| Audio synchronisation | Playback test | A/V sync within camera's normal tolerance |

### Step 19 — Adversarial-evidence test

The agency evidence custodian (or a designated counsel) reviews 5 random recordings made with NULLWEAR active and assesses:

- Is there ANY visible artefact in the video that could be attributed to NULLWEAR's operation?
- Is there ANY auditory artefact?
- Could opposing counsel plausibly argue the recording has been altered?

Expected: NULLWEAR operates on the BLE radio at the link layer. It does not interact with the camera's video or audio pipelines. Recordings should be indistinguishable from non-NULLWEAR recordings.

If the answer to any of the above is "yes", this requires a documented rebuttal **before** the next pilot phase. A common counter-rebuttal: provide chain-of-custody and hash evidence (Step 18) showing the file is byte-identical to what the camera generated.

### Step 20 — Disclosure to court

If any pilot recording is to be tendered as evidence in proceedings during or after the trial, the agency's standard disclosure obligations apply. The pilot programme should pre-prepare a one-page "NULLWEAR information sheet" that the prosecution can append to disclosure if asked. The information sheet should:

- Describe what NULLWEAR is (in lay terms).
- Explicitly state that NULLWEAR does not affect the camera's video/audio capture, encoding, storage, hash, or upload chain.
- Cite the chain-of-custody verification performed in Step 18.
- Identify the agency contact for further technical questions.

---

## Pass / fail verdict (revised)

A unit passes field testing if:

1. PAR ≥ 0.99 at 5 m in vertical orientation. **(Mandatory.)**
2. PAR ≥ 0.95 at 5 m in at least 4 of 5 wear orientations.
3. PAR ≥ 0.90 in all of the listed RF-busy environments.
4. Real-shift simulation observes < 200 target packets over 4 hours.
5. No corruption of non-target BLE traffic.
6. No measurable Wi-Fi degradation.
7. **30-day in-service trial passes Step 11 acceptance criteria.** (Mandatory before national rollout; pilot units may be issued before this completes provided officers consent to early-pilot status.)
8. **All Axon-equipment compatibility tests (Steps 14–17) pass.** (Mandatory — failure of any of these is a stop-the-pilot finding.)
9. **Evidentiary integrity tests (Steps 18–20) pass.** (Mandatory before any pilot recording is used as evidence.)

A unit fails if any mandatory criterion is missed, OR if more than one non-mandatory criterion is missed.

---

## Reporting

For each unit field-tested, generate a report containing:

- Unit serial number, firmware revision.
- Test technician name.
- Date, time, location (general — not GPS-precise).
- Source type and MAC.
- Receiver type.
- Each test step, recorded PAR, environment.
- Any anomalies.
- Final pass / fail.

Submit reports to the depot quartermaster within 24 hours of test completion.

---

## Logbook template (cut and use)

```
NULLWEAR Field Test Logbook

Unit S/N:                                    Tester:
Date:                  Start:        End:    Location:
Source: ☐ ESP32 emulator (MAC 00:25:DF:DE:AD:BE)
        ☐ Real Axon device (MAC ____________________)
Receiver: ☐ Linux laptop running ref_receiver.py
          ☐ Other: __________________________________

Step 1 — Baseline at 5 m:    target_packets = _________
Step 2 — Annihilation at 5 m: target_packets = ________   PAR = ______

Step 3 — Distance walk:
  2 m:   baseline ____  annihilation ____  PAR ____
  5 m:   baseline ____  annihilation ____  PAR ____
  10 m:  baseline ____  annihilation ____  PAR ____
  20 m:  baseline ____  annihilation ____  PAR ____
  50 m:  baseline ____  annihilation ____  PAR ____

Step 4 — Orientation:
  LED up:        PAR ____
  LED down:      PAR ____
  Flat:          PAR ____
  Back pocket:   PAR ____
  Chest pocket:  PAR ____

Step 5 — Mobility loop: baseline ____  annihilation ____  PAR ____

Step 6 — Non-target traffic intact?  ☐ Yes  ☐ No
Step 7 — Wi-Fi unaffected?           ☐ Yes  ☐ No
Step 8 — RF-busy environments tested:
  Residential: PAR ____   Office: PAR ____
  Shopping:    PAR ____   Intersection: PAR ____   Vehicle: PAR ____

Step 9 — 4-hour simulation: target_packets received = ________
Step 10 — 5-officer scenario: target_packets received = ________

Mandatory criteria pass?  ☐ Yes (PROCEED)  ☐ No (FAIL — RMA unit)

Tester signature: ___________________________________

Notes / anomalies:
```

---

End of protocol.
