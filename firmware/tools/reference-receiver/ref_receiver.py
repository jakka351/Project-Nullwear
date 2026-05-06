#!/usr/bin/env python3
"""
Project NULLWEAR — Reference receiver for verification

This script is the canonical way to verify whether NULLWEAR is working.
It listens for BLE advertising packets and counts how many of them are
attributable to Axon Enterprise gear, using the same dual-signature
matching strategy as the v1.1 firmware:

    Phase 1 — OUI match
        Advertiser MAC begins with 00:25:DF (IEEE OUI assigned to
        Taser International / Axon Enterprise).

    Phase 2 — Service Data UUID match
        Advertisement contains BLE 16-bit Service Data UUID 0xFE6B
        (registered to Axon Public Safety with the Bluetooth SIG).
        This catches *docked / non-deployed* devices, which broadcast
        with a sanitised AdvA (typically the all-zero MAC
        00:00:00:00:00:00) and so cannot be matched on OUI alone.

    Phase 3 — Sanitised MAC heuristic
        Any advertisement whose MAC is the all-zero or all-ones address
        is flagged as a "sanitised AdvA" candidate even if the
        UUID/OUI checks miss. These should never appear from normal
        consumer BLE peripherals; their presence almost always means
        an enterprise device with a privacy-zeroed advertiser address.

If the v1.1 firmware is doing its job, *all three* of these should drop
toward zero in the protected zone — including the previously-undetected
docked Axon devices.

Usage scenarios
===============

1. **Baseline measurement (no NULLWEAR active):** run this script with a
   known Axon BLE source nearby (a real Axon device, or the ESP32
   emulator in firmware/tools/test-source). Confirm that you see the
   source's MAC at high reception rate. For docked-device testing,
   place an Axon Body 3 in its dock and confirm Phase 2 (UUID match)
   detections appear with a 00:00:00:00:00:00 MAC.

2. **Annihilation test (NULLWEAR active):** with the same source still
   broadcasting, place a NULLWEAR/P device between the source and the
   receiver. Re-run this script. The reception rate of *both* the OUI
   path and the UUID path should drop to ~0%.

3. **Continuous field test:** run this script for an extended period
   (e.g. an hour) in a real operational environment. Log every detection.
   Compare against expected officer presence and dock locations.

Verification metric
===================

The simplest acceptance criterion is the **packet annihilation ratio (PAR)**:

    PAR = 1 - (rx_with_nullwear / rx_without_nullwear)

A PAR of ≥ 0.99 at distances up to 5 m is the engineering target stated
in the Engineering Specification §AR-03. The PAR is computed against the
**combined** target hit count (OUI ∪ UUID ∪ sanitised-MAC) so that
docked-device broadcasts count toward the verdict.

Requirements
============

- Python 3.9+
- The `bleak` library (cross-platform BLE)
- A computer with a working Bluetooth adapter

Install:

    pip install bleak

License: MIT
"""

import argparse
import asyncio
import csv
import json
import re
import sys
import time
from collections import defaultdict
from datetime import datetime, timezone

try:
    from bleak import BleakScanner
    from bleak.backends.scanner import AdvertisementData, BLEDevice
except ImportError:
    print("ERROR: the 'bleak' library is required. Install with:")
    print("    pip install bleak")
    sys.exit(1)


# Phase 1 — IEEE OUI assigned to Taser International / Axon Enterprise.
AXON_OUI_PREFIX = "00:25:DF"

# Phase 2 — BLE 16-bit Service Data UUID registered to Axon Public Safety.
# bleak exposes service_data dict keys as the full 128-bit form, lowercase.
AXON_FE6B_UUID_SHORT = "FE6B"
AXON_FE6B_UUID_FULL  = "0000fe6b-0000-1000-8000-00805f9b34fb"

# Phase 3 — Sanitised AdvA addresses observed in the wild from docked /
# non-deployed enterprise gear. A normal consumer BLE peripheral never
# advertises with these MACs, so a hit here is a strong tell on its own.
SANITISED_MACS = {
    "00:00:00:00:00:00",
    "FF:FF:FF:FF:FF:FF",
}

# Cleartext serial pattern observed in FE6B service-data payloads of
# Axon Body 3 / Body 3 Plus units (e.g. "X60J0xxxN", "X60M0xxx4").
# 9 ASCII chars: "X60" + model letter + 5 alphanumerics, occupying
# bytes 14-22 of the FE6B payload. The match length is permitted to
# extend to 10 chars to absorb any field variant we haven't seen yet.
AXON_SERIAL_RE = re.compile(rb"X60[A-Z][0-9A-Z]{5,6}")


def extract_axon_serial(payload: bytes) -> str | None:
    """Return the first Axon X60-series serial found in `payload`, or None."""
    if not payload:
        return None
    m = AXON_SERIAL_RE.search(payload)
    return m.group(0).decode("ascii", errors="replace") if m else None


def classify_advertisement(mac: str, adv: "AdvertisementData",
                           target_oui: str) -> dict:
    """
    Run all three matching phases on a single advertisement and return
    a structured verdict.

    Output keys:
        oui_match           bool
        uuid_match          bool
        sanitised_mac       bool
        is_target           bool   (any of the above)
        fe6b_payload_hex    str | None
        axon_serial         str | None
    """
    mac_u = mac.upper()
    oui_match = mac_u.startswith(target_oui.upper())

    # bleak's service_data: dict[str (full lowercase UUID) -> bytes]
    service_data = getattr(adv, "service_data", {}) or {}
    fe6b_payload: bytes | None = None
    for k, v in service_data.items():
        # Be defensive — accept either the canonical full 128-bit form
        # or any key that contains the 16-bit FE6B short form.
        kl = (k or "").lower()
        if kl == AXON_FE6B_UUID_FULL or AXON_FE6B_UUID_SHORT.lower() in kl:
            fe6b_payload = bytes(v) if v is not None else b""
            break
    uuid_match = fe6b_payload is not None

    sanitised_mac = mac_u in SANITISED_MACS

    return {
        "oui_match":        oui_match,
        "uuid_match":       uuid_match,
        "sanitised_mac":    sanitised_mac,
        "is_target":        oui_match or uuid_match or sanitised_mac,
        "fe6b_payload_hex": fe6b_payload.hex() if fe6b_payload else None,
        "axon_serial":      extract_axon_serial(fe6b_payload) if fe6b_payload else None,
    }


class ReceptionLog:
    """Records every observed BLE advertisement and computes statistics."""

    def __init__(self, target_oui: str = AXON_OUI_PREFIX,
                 enable_phase2: bool = True,
                 enable_phase3: bool = True):
        self.target_oui = target_oui.upper()
        self.enable_phase2 = enable_phase2
        self.enable_phase3 = enable_phase3
        self.start_time = time.time()

        self.total_rx = 0
        self.target_rx = 0          # union of all enabled phases
        self.rx_oui = 0             # Phase 1 hits
        self.rx_uuid = 0            # Phase 2 hits
        self.rx_sanitised = 0       # Phase 3 hits
        self.rx_uuid_only = 0       # Phase 2 hit AND not Phase 1 (docked-device tell)

        self.per_mac_counts: dict[str, int] = defaultdict(int)
        self.per_mac_first_seen: dict[str, float] = {}
        self.per_mac_last_seen:  dict[str, float] = {}
        self.per_mac_rssi_min:   dict[str, int] = {}
        self.per_mac_rssi_max:   dict[str, int] = {}
        self.per_mac_match_kind: dict[str, set[str]] = defaultdict(set)
        self.per_mac_serials:    dict[str, set[str]] = defaultdict(set)
        self.serials_seen: set[str] = set()
        self.events: list[dict] = []

    def record(self, mac: str, rssi: int, name: str | None,
               adv: "AdvertisementData"):
        now = time.time()
        mac_u = mac.upper()
        self.total_rx += 1
        self.per_mac_counts[mac_u] += 1
        if mac_u not in self.per_mac_first_seen:
            self.per_mac_first_seen[mac_u] = now
            self.per_mac_rssi_min[mac_u] = rssi
            self.per_mac_rssi_max[mac_u] = rssi
        self.per_mac_last_seen[mac_u] = now
        self.per_mac_rssi_min[mac_u] = min(self.per_mac_rssi_min[mac_u], rssi)
        self.per_mac_rssi_max[mac_u] = max(self.per_mac_rssi_max[mac_u], rssi)

        v = classify_advertisement(mac_u, adv, self.target_oui)

        # Apply user-disable flags.
        oui  = v["oui_match"]
        uuid = v["uuid_match"]       and self.enable_phase2
        san  = v["sanitised_mac"]    and self.enable_phase3
        is_target = oui or uuid or san

        if oui:  self.rx_oui += 1
        if uuid: self.rx_uuid += 1
        if san:  self.rx_sanitised += 1
        if uuid and not oui:
            self.rx_uuid_only += 1
        if is_target:
            self.target_rx += 1

        if oui:  self.per_mac_match_kind[mac_u].add("OUI")
        if uuid: self.per_mac_match_kind[mac_u].add("UUID")
        if san:  self.per_mac_match_kind[mac_u].add("SANITISED")

        if v["axon_serial"]:
            self.per_mac_serials[mac_u].add(v["axon_serial"])
            self.serials_seen.add(v["axon_serial"])

        self.events.append({
            "timestamp": datetime.now(timezone.utc).isoformat(),
            "mac": mac_u,
            "rssi": rssi,
            "name": name or "",
            "is_target": is_target,
            "oui_match": oui,
            "uuid_match": uuid,
            "sanitised_mac": san,
            "fe6b_payload_hex": v["fe6b_payload_hex"],
            "axon_serial": v["axon_serial"],
        })

    def elapsed(self) -> float:
        return time.time() - self.start_time

    def summary(self) -> dict:
        elapsed = max(self.elapsed(), 1e-6)
        target_macs = {
            m: c for m, c in self.per_mac_counts.items()
            if self.per_mac_match_kind.get(m)
        }
        return {
            "elapsed_seconds": round(elapsed, 2),
            "total_packets_received": self.total_rx,
            "target_packets_received": self.target_rx,
            "target_packets_per_second": round(self.target_rx / elapsed, 3),
            "phase_breakdown": {
                "phase1_oui_hits":          self.rx_oui,
                "phase2_uuid_hits":         self.rx_uuid,
                "phase2_uuid_only_hits":    self.rx_uuid_only,
                "phase3_sanitised_mac_hits": self.rx_sanitised,
            },
            "phases_enabled": {
                "phase1_oui":          True,
                "phase2_uuid":         self.enable_phase2,
                "phase3_sanitised":    self.enable_phase3,
            },
            "unique_target_macs": len(target_macs),
            "target_macs": sorted(target_macs.keys()),
            "per_mac_target": target_macs,
            "per_mac_match_kind": {m: sorted(k) for m, k in
                                   self.per_mac_match_kind.items()},
            "axon_serials_recovered": sorted(self.serials_seen),
        }


async def scan_loop(log: ReceptionLog, duration_s: float):
    """Run the BLE scanner for `duration_s` seconds, logging every adv."""

    def on_detection(device: BLEDevice, adv: AdvertisementData):
        # bleak normalises MAC addresses with colons and uppercase
        log.record(device.address, adv.rssi, device.name, adv)

    scanner = BleakScanner(detection_callback=on_detection)
    await scanner.start()
    try:
        await asyncio.sleep(duration_s)
    finally:
        await scanner.stop()


def _kind_label(kinds: set[str]) -> str:
    """Compact label for the live table."""
    if not kinds:
        return "-"
    order = ["OUI", "UUID", "SANITISED"]
    return "+".join(k for k in order if k in kinds)


def print_live_summary(log: ReceptionLog, target_oui: str):
    print("\033[2J\033[H", end="")  # clear screen
    s = log.summary()
    pb = s["phase_breakdown"]
    pe = s["phases_enabled"]
    print(f"NULLWEAR Reference Receiver — dual-signature matcher")
    print(f"  Phase 1 OUI       : {target_oui}")
    print(f"  Phase 2 UUID      : 0x{AXON_FE6B_UUID_SHORT}  "
          f"({'enabled' if pe['phase2_uuid'] else 'DISABLED'})")
    print(f"  Phase 3 sanitised : {sorted(SANITISED_MACS)}  "
          f"({'enabled' if pe['phase3_sanitised'] else 'DISABLED'})")
    print(f"Elapsed: {s['elapsed_seconds']:>8.1f} s")
    print(f"Total RX: {s['total_packets_received']:>6}   "
          f"Target RX: {s['target_packets_received']:>6}   "
          f"Target rate: {s['target_packets_per_second']:.2f} pkt/s")
    print(f"Phase hits: OUI={pb['phase1_oui_hits']}  "
          f"UUID={pb['phase2_uuid_hits']} "
          f"(of which UUID-only={pb['phase2_uuid_only_hits']})  "
          f"SANITISED={pb['phase3_sanitised_mac_hits']}")
    print(f"Unique target MACs seen: {s['unique_target_macs']}   "
          f"Axon serials recovered: {len(s['axon_serials_recovered'])}")
    if s['target_macs']:
        print()
        print(f"{'MAC':<22} {'kind':<14} {'count':>7} "
              f"{'rssi-':>6} {'rssi+':>6}  {'name':<18} {'serial':<12}")
        for mac in s['target_macs']:
            count = log.per_mac_counts[mac]
            rmin = log.per_mac_rssi_min.get(mac, 0)
            rmax = log.per_mac_rssi_max.get(mac, 0)
            kind = _kind_label(log.per_mac_match_kind.get(mac, set()))
            serials = log.per_mac_serials.get(mac, set())
            serial = next(iter(serials)) if serials else ""
            # Look up most recent name from event log
            name = ""
            for ev in reversed(log.events):
                if ev["mac"] == mac and ev["name"]:
                    name = ev["name"]
                    break
            print(f"{mac:<22} {kind:<14} {count:>7} "
                  f"{rmin:>6} {rmax:>6}  {name:<18} {serial:<12}")
    else:
        print("(no target packets observed yet)")


async def live_summary_loop(log: ReceptionLog, target_oui: str, interval: float):
    while True:
        await asyncio.sleep(interval)
        print_live_summary(log, target_oui)


async def main_async(args):
    log = ReceptionLog(
        target_oui=args.oui,
        enable_phase2=not args.no_phase2,
        enable_phase3=not args.no_phase3,
    )

    print(f"Scanning for {args.duration:.0f} s.")
    print(f"  Phase 1 OUI prefix       : {args.oui}")
    print(f"  Phase 2 UUID match (FE6B): {'OFF' if args.no_phase2 else 'ON'}")
    print(f"  Phase 3 sanitised MAC    : {'OFF' if args.no_phase3 else 'ON'}")
    print("Press Ctrl+C to stop early.")
    print()

    summary_task = asyncio.create_task(
        live_summary_loop(log, args.oui, args.refresh))
    try:
        await scan_loop(log, args.duration)
    except KeyboardInterrupt:
        print("\nInterrupted")
    finally:
        summary_task.cancel()
        try:
            await summary_task
        except asyncio.CancelledError:
            pass

    s = log.summary()
    print()
    print("=== FINAL SUMMARY ===")
    print(json.dumps(s, indent=2))

    if args.out_csv:
        with open(args.out_csv, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=list(log.events[0].keys()) if log.events
                               else ["timestamp", "mac", "rssi", "name",
                                     "is_target", "oui_match", "uuid_match",
                                     "sanitised_mac", "fe6b_payload_hex",
                                     "axon_serial"])
            w.writeheader()
            for e in log.events:
                w.writerow(e)
        print(f"Wrote raw event log: {args.out_csv}")

    if args.out_json:
        with open(args.out_json, "w") as f:
            json.dump({
                "summary": s,
                "events": log.events,
            }, f, indent=2)
        print(f"Wrote JSON summary + events: {args.out_json}")

    # Compute simple acceptance verdict if a baseline was provided
    if args.baseline_target_rx is not None:
        baseline = args.baseline_target_rx
        observed = log.target_rx
        if baseline <= 0:
            print("Cannot compute PAR — baseline was 0 (no detections seen "
                  "without NULLWEAR active). Re-run baseline measurement.")
        else:
            par = 1.0 - (observed / baseline)
            print()
            print(f"Baseline target RX (no NULLWEAR): {baseline}")
            print(f"Observed target RX (with NULLWEAR): {observed}")
            print(f"  (combined across OUI + UUID + sanitised-MAC phases)")
            print(f"Packet Annihilation Ratio (PAR):   {par:.4f}")
            verdict = "PASS" if par >= 0.99 else \
                      "MARGINAL" if par >= 0.90 else "FAIL"
            print(f"Acceptance verdict (target ≥ 0.99): {verdict}")


def main():
    parser = argparse.ArgumentParser(
        description="NULLWEAR reference receiver — verify per-packet "
                    "annihilation of BLE adv packets matching Axon's "
                    "OUI, FE6B service UUID, or a sanitised-AdvA pattern "
                    "(matches the v1.1 firmware dual-signature matcher).")
    parser.add_argument("--oui", default=AXON_OUI_PREFIX,
        help=f"Phase 1 target OUI prefix (default: {AXON_OUI_PREFIX})")
    parser.add_argument("--no-phase2", action="store_true",
        help="Disable Phase 2 (FE6B service-data UUID match). Use to "
             "reproduce v1.0 behaviour or isolate the OUI-only path.")
    parser.add_argument("--no-phase3", action="store_true",
        help="Disable Phase 3 (sanitised AdvA detection). Use if your "
             "environment legitimately advertises the all-zero or "
             "all-ones MAC for some non-Axon reason.")
    parser.add_argument("--duration", type=float, default=60.0,
        help="Scan duration in seconds (default: 60)")
    parser.add_argument("--refresh", type=float, default=2.0,
        help="Live-summary refresh interval in seconds (default: 2.0)")
    parser.add_argument("--out-csv", help="Optional CSV output path")
    parser.add_argument("--out-json", help="Optional JSON output path")
    parser.add_argument("--baseline-target-rx", type=int, default=None,
        help="If supplied, computes Packet Annihilation Ratio against "
             "this baseline target-packet count (number observed in a "
             "preceding baseline scan with NULLWEAR off). PAR is "
             "computed against the COMBINED hit count across all "
             "enabled phases.")
    args = parser.parse_args()

    try:
        asyncio.run(main_async(args))
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
