# NULLWEAR Reference Receiver

Verification tool. Use this to determine — empirically, on any computer with a Bluetooth radio — whether a NULLWEAR device is correctly annihilating Axon-OUI BLE advertising packets.

## What it does

Listens for **all** BLE advertising packets in the air. Filters by OUI prefix (`00:25:DF` by default). Reports:

- The total number of advertising packets received.
- The number of packets matching the target OUI.
- The unique MAC addresses observed within that OUI.
- Per-MAC packet count, RSSI min/max, and most-recently-observed device name.
- A **Packet Annihilation Ratio (PAR)** if you supply a baseline reference.

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

Note the `target_packets_received` figure in the summary. That's your baseline.

## Run the verification

With **NULLWEAR active** between you and the source, same source, same range:

```bash
python ref_receiver.py --duration 60 --baseline-target-rx <BASELINE> --out-json verify.json
```

The script will print a PAR figure and a PASS / MARGINAL / FAIL verdict.

## Acceptance criterion

| PAR  | Verdict | Meaning |
|------|---------|---------|
| ≥ 0.99 | **PASS** | NULLWEAR is correctly annihilating ≥ 99% of target packets at this range. |
| 0.90–0.99 | MARGINAL | Some packets are getting through. Likely cause: range, antenna orientation, or RX/TX timing margin on minimum-length advertisements. Investigate. |
| < 0.90 | FAIL | NULLWEAR is not effectively suppressing the target. Investigate. Likely cause: firmware bug, wrong board, OUI value not as expected, or the source is not actually the target you think it is. |

## What this tool does NOT do

- It does not actively transmit anything. It is a passive receiver.
- It does not need an SDR. The standard Bluetooth radio in any modern computer is sufficient.
- It does not validate timing, only outcomes. For deeper RF-level analysis, use a logic analyser or SDR capture (see `docs/12-acceptance-test-procedure.md`).

## Output formats

- **Live console** — refreshes every 2 s by default.
- **CSV** (with `--out-csv`) — every detected advertisement with timestamp, MAC, RSSI, name, and target-flag. For long-form analysis.
- **JSON** (with `--out-json`) — same plus a summary object. For automated test pipelines.

## Caveats

- **OS-level address filtering.** Some operating systems (especially recent Android and iOS) may filter or randomise BLE MAC addresses before they reach userspace. Use Linux for highest fidelity. macOS is acceptable. Windows is acceptable but has more variability.
- **Receiver sensitivity.** Your computer's Bluetooth radio is typically less sensitive than a purpose-built BLE sniffer. For PAR measurements you need to confirm the BASELINE measurement is valid — i.e. the source is genuinely visible at the range you intend to test.
- **Adversarial signal.** Anyone running this script is recording every BLE MAC in the area. This is a privacy-relevant capability. Do not run it persistently in unauthorised contexts.

License: MIT
