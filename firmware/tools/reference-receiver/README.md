# NULLWEAR Reference Receiver

Verification tool. Use this to determine — empirically, on any computer with a Bluetooth radio — whether a NULLWEAR device is correctly annihilating Axon BLE advertising packets.

## What it does

Listens for **all** BLE advertising packets in the air and applies the same dual-signature matching strategy as the v1.1 firmware:

| Phase | Match rule | What it catches |
|-------|------------|-----------------|
| **1 — OUI**            | MAC begins with `00:25:DF` (Taser International / Axon Enterprise IEEE OUI)        | In-service Axon devices broadcasting their unfiltered MAC. |
| **2 — Service UUID**   | Advertisement contains 16-bit Service Data UUID `0xFE6B` (Axon Public Safety)      | **Docked / non-deployed** Axon devices that broadcast with a sanitised AdvA (typically `00:00:00:00:00:00`) and would never be caught by an OUI-only filter. |
| **3 — Sanitised MAC**  | Advertiser address is `00:00:00:00:00:00` or `FF:FF:FF:FF:FF:FF`                   | Catch-all for anything broadcasting with a privacy-zeroed MAC, even if the FE6B UUID is also stripped. |

A match in **any** phase counts toward the target hit total. The live display breaks down hits by phase, including a `UUID-only` count that tells you how many advertisements were attributable to Axon **only** because of the FE6B UUID — i.e. the docked-device population that the v1.0 OUI-only firmware would have missed entirely.

The receiver also opportunistically extracts the Axon X60-series cleartext serial number (`X60J0xxxN`, `X60M0xxx4`, etc.) from the FE6B service-data payload when present, and shows it in the per-MAC table.

Reports:

- Total advertising packets received.
- Packets matching each of the three phases (with combined and UUID-only counters).
- Unique target MAC addresses.
- Per-MAC packet count, RSSI min/max, match-kind label (`OUI` / `UUID` / `SANITISED` or combinations), most recent device name, recovered cleartext serial.
- A **Packet Annihilation Ratio (PAR)** if you supply a baseline reference, computed against the **combined** target hit count.

## Install

```bash
pip install bleak
```

Works on Linux, macOS, Windows. Some Linux distributions need `bluez` and the user added to the `bluetooth` group.

## Quickstart — measure a baseline

With **no NULLWEAR active** and a known Axon BLE source nearby (the ESP32 emulator in `../test-source/`, or a real Axon device under controlled conditions):

```bash
python ref_receiver.py --duration 60 --out-json baseline.json
```

For docked-device coverage, place an Axon Body 3 in its dock during part of the baseline. You should see Phase 2 (UUID) hits arrive against a `00:00:00:00:00:00` MAC.

Note the `target_packets_received` figure in the summary. That's your baseline (it already combines all three phases).

## Run the verification

With **NULLWEAR active** between you and the source, same source, same range:

```bash
python ref_receiver.py --duration 60 --baseline-target-rx <BASELINE> --out-json verify.json
```

The script will print a PAR figure and a PASS / MARGINAL / FAIL verdict.

## Acceptance criterion

| PAR  | Verdict | Meaning |
|------|---------|---------|
| ≥ 0.99 | **PASS** | NULLWEAR is correctly annihilating ≥ 99% of target packets at this range — *including* the docked-device path. |
| 0.90–0.99 | MARGINAL | Some packets are getting through. Likely cause: range, antenna orientation, RX/TX timing margin on minimum-length advertisements, or Phase-2 hits arriving with payload-shape variants the firmware didn't anticipate. Investigate. |
| < 0.90 | FAIL | NULLWEAR is not effectively suppressing the target. Investigate. Likely cause: firmware bug, wrong board, OUI value not as expected, build with `NULLWEAR_ENABLE_PHASE_2=0` against a docked-only target, or the source is not actually the target you think it is. |

## Reproducing v1.0 behaviour

To verify against the OUI-only path (e.g. when bringing up a v1.0 board, or isolating regressions):

```bash
python ref_receiver.py --no-phase2 --no-phase3 --duration 60
```

This restricts matching to the IEEE OUI prefix exactly as the v1.0 firmware did.

## What this tool does NOT do

- It does not actively transmit anything. It is a passive receiver.
- It does not need an SDR. The standard Bluetooth radio in any modern computer is sufficient.
- It does not validate timing, only outcomes. For deeper RF-level analysis, use a logic analyser or SDR capture (see `docs/12-acceptance-test-procedure.md`).

## Output formats

- **Live console** — refreshes every 2 s by default. Shows per-phase hit counts and a per-MAC table with match kind and recovered serials.
- **CSV** (with `--out-csv`) — every detected advertisement with timestamp, MAC, RSSI, name, target-flag, per-phase match flags, raw FE6B payload hex, and recovered Axon serial. For long-form analysis.
- **JSON** (with `--out-json`) — same plus a summary object including the phase breakdown and the de-duplicated list of recovered Axon serials. For automated test pipelines.

## Caveats

- **OS-level address filtering.** Some operating systems (especially recent Android and iOS) may filter or randomise BLE MAC addresses before they reach userspace. Use Linux for highest fidelity. macOS is acceptable. Windows is acceptable but has more variability.
- **OS-level service-data exposure.** Phase 2 depends on the OS surfacing the raw FE6B service-data record to the bleak library. All three of Linux/BlueZ, macOS CoreBluetooth, and Windows WinRT BLE expose service-data dictionaries, but if you run on a stack that strips them, Phase 2 will silently miss. Confirm by inspecting `events[*].fe6b_payload_hex` in the JSON output during a baseline measurement.
- **Receiver sensitivity.** Your computer's Bluetooth radio is typically less sensitive than a purpose-built BLE sniffer. For PAR measurements you need to confirm the BASELINE measurement is valid — i.e. the source is genuinely visible at the range you intend to test.
- **Sanitised-MAC false positives.** Phase 3 will fire on *any* peripheral broadcasting with the all-zero or all-ones MAC. In dense enterprise environments this is essentially Axon-specific, but if you are testing somewhere with non-Axon equipment that does this for unrelated reasons, disable Phase 3 with `--no-phase3` and rely on Phases 1 and 2.
- **Adversarial signal.** Anyone running this script is recording every BLE MAC in the area, plus extracting plaintext device serials from FE6B broadcasts. This is a privacy-relevant capability. Do not run it persistently in unauthorised contexts.

License: MIT
