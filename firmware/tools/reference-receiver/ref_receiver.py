#!/usr/bin/env python3
"""
Project NULLWEAR — Reference receiver for verification

This script is the canonical way to verify whether NULLWEAR is working.
It listens for BLE advertising packets and counts how many of them have
a MAC prefixed with the Axon Enterprise OUI 00:25:DF.

Usage scenarios
===============

1. **Baseline measurement (no NULLWEAR active):** run this script with a
   known Axon BLE source nearby (a real Axon device, or the ESP32
   emulator in firmware/tools/test-source). Confirm that you see the
   source's MAC at high reception rate.

2. **Annihilation test (NULLWEAR active):** with the same source still
   broadcasting, place a NULLWEAR/P device between the source and the
   receiver. Re-run this script. The reception rate of the target MAC
   should drop to ~0%.

3. **Continuous field test:** run this script for an extended period
   (e.g. an hour) in a real operational environment. Log every detection.
   Compare against expected officer presence.

Verification metric
===================

The simplest acceptance criterion is the **packet annihilation ratio (PAR)**:

    PAR = 1 - (rx_with_nullwear / rx_without_nullwear)

A PAR of ≥ 0.99 at distances up to 5 m is the engineering target stated
in the Engineering Specification §AR-03.

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


AXON_OUI_PREFIX = "00:25:DF"


class ReceptionLog:
    """Records every observed BLE advertisement and computes statistics."""

    def __init__(self, target_oui: str = AXON_OUI_PREFIX):
        self.target_oui = target_oui.upper()
        self.start_time = time.time()
        self.total_rx = 0
        self.target_rx = 0
        self.per_mac_counts: dict[str, int] = defaultdict(int)
        self.per_mac_first_seen: dict[str, float] = {}
        self.per_mac_last_seen: dict[str, float] = {}
        self.per_mac_rssi_min: dict[str, int] = {}
        self.per_mac_rssi_max: dict[str, int] = {}
        self.events: list[dict] = []

    def record(self, mac: str, rssi: int, name: str | None):
        now = time.time()
        mac = mac.upper()
        self.total_rx += 1
        self.per_mac_counts[mac] += 1
        if mac not in self.per_mac_first_seen:
            self.per_mac_first_seen[mac] = now
            self.per_mac_rssi_min[mac] = rssi
            self.per_mac_rssi_max[mac] = rssi
        self.per_mac_last_seen[mac] = now
        self.per_mac_rssi_min[mac] = min(self.per_mac_rssi_min[mac], rssi)
        self.per_mac_rssi_max[mac] = max(self.per_mac_rssi_max[mac], rssi)

        is_target = mac.startswith(self.target_oui)
        if is_target:
            self.target_rx += 1

        self.events.append({
            "timestamp": datetime.now(timezone.utc).isoformat(),
            "mac": mac,
            "rssi": rssi,
            "name": name or "",
            "is_target": is_target,
        })

    def elapsed(self) -> float:
        return time.time() - self.start_time

    def summary(self) -> dict:
        elapsed = max(self.elapsed(), 1e-6)
        target_macs = {m: c for m, c in self.per_mac_counts.items()
                       if m.startswith(self.target_oui)}
        return {
            "elapsed_seconds": round(elapsed, 2),
            "total_packets_received": self.total_rx,
            "target_packets_received": self.target_rx,
            "target_packets_per_second": round(self.target_rx / elapsed, 3),
            "unique_target_macs": len(target_macs),
            "target_macs": sorted(target_macs.keys()),
            "per_mac_target": target_macs,
        }


async def scan_loop(log: ReceptionLog, duration_s: float):
    """Run the BLE scanner for `duration_s` seconds, logging every adv."""

    def on_detection(device: BLEDevice, adv: AdvertisementData):
        # bleak normalises MAC addresses with colons and uppercase
        log.record(device.address, adv.rssi, device.name)

    scanner = BleakScanner(detection_callback=on_detection)
    await scanner.start()
    try:
        await asyncio.sleep(duration_s)
    finally:
        await scanner.stop()


def print_live_summary(log: ReceptionLog, target_oui: str):
    print("\033[2J\033[H", end="")  # clear screen
    s = log.summary()
    print(f"NULLWEAR Reference Receiver — listening for OUI {target_oui}")
    print(f"Elapsed: {s['elapsed_seconds']:>8.1f} s")
    print(f"Total RX: {s['total_packets_received']:>6}   "
          f"Target RX: {s['target_packets_received']:>6}   "
          f"Target rate: {s['target_packets_per_second']:.2f} pkt/s")
    print(f"Unique target MACs seen: {s['unique_target_macs']}")
    if s['target_macs']:
        print()
        print(f"{'MAC':<22} {'count':>8} {'rssi min':>10} {'rssi max':>10}  "
              f"{'name':<20}")
        for mac in s['target_macs']:
            count = log.per_mac_counts[mac]
            rmin = log.per_mac_rssi_min.get(mac, 0)
            rmax = log.per_mac_rssi_max.get(mac, 0)
            # Look up most recent name from event log
            name = ""
            for ev in reversed(log.events):
                if ev["mac"] == mac and ev["name"]:
                    name = ev["name"]
                    break
            print(f"{mac:<22} {count:>8} {rmin:>10} {rmax:>10}  {name:<20}")
    else:
        print("(no target packets observed yet)")


async def live_summary_loop(log: ReceptionLog, target_oui: str, interval: float):
    while True:
        await asyncio.sleep(interval)
        print_live_summary(log, target_oui)


async def main_async(args):
    log = ReceptionLog(target_oui=args.oui)

    print(f"Scanning for {args.duration:.0f} s. Target OUI: {args.oui}")
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
                               else ["timestamp", "mac", "rssi", "name", "is_target"])
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
            print(f"Packet Annihilation Ratio (PAR):   {par:.4f}")
            verdict = "PASS" if par >= 0.99 else \
                      "MARGINAL" if par >= 0.90 else "FAIL"
            print(f"Acceptance verdict (target ≥ 0.99): {verdict}")


def main():
    parser = argparse.ArgumentParser(
        description="NULLWEAR reference receiver — verify per-packet "
                    "annihilation of BLE adv packets matching a target OUI.")
    parser.add_argument("--oui", default=AXON_OUI_PREFIX,
        help=f"Target OUI prefix (default: {AXON_OUI_PREFIX})")
    parser.add_argument("--duration", type=float, default=60.0,
        help="Scan duration in seconds (default: 60)")
    parser.add_argument("--refresh", type=float, default=2.0,
        help="Live-summary refresh interval in seconds (default: 2.0)")
    parser.add_argument("--out-csv", help="Optional CSV output path")
    parser.add_argument("--out-json", help="Optional JSON output path")
    parser.add_argument("--baseline-target-rx", type=int, default=None,
        help="If supplied, computes Packet Annihilation Ratio against "
             "this baseline target-packet count (number observed in a "
             "preceding baseline scan with NULLWEAR off).")
    args = parser.parse_args()

    try:
        asyncio.run(main_async(args))
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
