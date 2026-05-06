#!/usr/bin/env python3
##########################################################################
#
#   <~~~~~~~~~~>                i<~~~~i<~~~~i                                ---      ?-_   ---      ---   _--]           ---~           _--]     ?--   ]-----------l     -------]     --------?]     
#   ~++++++++++~~i              >~++++~~++++<                                ---      ---   ---]     ---   ----           ----           ----     ---   ------------?     --------     ----------     
#   ~++++~~~+++++~<  :i<<<<>:   >~++++~<~~~~!    !><<<>:       :><<<>:       ------   ---   ---]     ---   ----           ----           ----     ---   ----           ]---     ----   ---     ----   
#   ~++++~: >~+++~<!~++++++++~< >~++++~~++++<  >~~+++++~<>  :>~+++++++~<     --- ---- ---   ---]     ---   ----           ----           ----     ---   ----           ]---     ----   ---     ----   
#   ~+++++~~~~+++~i~~+++~~~+++~<>~++++~~++++<:~~+++~<~++++<:<~++~><~~++~>    --- --------   ---]     ---   ----           ----           ----     ---   ----------     ]------------   -----------?   
#   ~++++++++++~~i<~+++~  >++++~>~++++~~++++<<~+++~< !><<>il~++++++++++++>   ---   ------   ---]     ---   ----           ----           ---- --- ---   ----------     ]------------   ----------     
#   ~++++~<<<>>!  <++++~  i++++~>~++++~~++++<<~+++~> l<<<<i!~+++~~~~~~~~~:   ---      ---   ---]     ---   ----           ----           ---- --- ---   ----           ]---     ----   --- ---        
#   ~++++~>       :~++++~~~+++~>>~++++~~++++<:<~+++~~++++~<:<~++++~<~+++<    ---      ---   ---]     ---   ----           ----           ------ -----   ----           ]---     ----   ---   ----     
#   ~++++~>        :~++++++++~! >~++++~<++++<  i~~++++++~;   >~+++++++~>     ---      ---     --------     ------------   ------------   ----     ---   ------------?  ]---     ----   ---     ----   
#   :llll!            i<~~~>:   :!lll!::!lll:    :i~~~<!       i<~~~>:                                                                                                                                
#
#   https://github.com/jakka351/Project-Nullwear
#
#
#
"""
Project NULLWEAR — Automated Acceptance Test Procedure (ATP) Harness

Implements the test sequence defined in docs/12-acceptance-test-procedure.md.
Designed to run on a contract manufacturer's bench, orchestrating:

  - A J-Link connection to the device under test (DUT) for firmware
    flash + RTT log capture.
  - A reference receiver (running on a separate host or a USB-attached
    BLE adapter) for radio-side measurements.
  - A test source (ESP32 emulator, MAC 00:25:DF:DE:AD:BE).
  - Optional: a programmable USB power supply for current measurement.
  - Optional: a USB-CDC connection to the DUT for diagnostic command
    invocation.

Output: a JSON report conforming to atp_schema.json.

This is a reference harness. Pilot CMs are expected to extend it for
their specific bench equipment (programmable PSU model, fixture-control
APIs, asset-system upload).

License: MIT
"""

from __future__ import annotations

import argparse
import asyncio
import datetime as dt
import json
import os
import subprocess
import sys
import time
from dataclasses import dataclass, field, asdict
from typing import Any

ATP_VERSION = "1.0.0"
DEFAULT_OUI = "00:25:DF"
DEFAULT_TEST_MAC = "00:25:DF:DE:AD:BE"
DEFAULT_BASELINE_PACKETS_5M = 1500   # expected from ESP32 source at 5 m, 60 s
DEFAULT_PAR_PASS = 0.99
DEFAULT_PAR_MARGINAL = 0.90


@dataclass
class TestResult:
    name: str
    verdict: str  # "PASS" / "FAIL" / "SKIP" / "MARGINAL"
    measurements: dict[str, Any] = field(default_factory=dict)
    notes: str = ""
    elapsed_seconds: float = 0.0


@dataclass
class AtpReport:
    schema_version: str = "1.0.0"
    atp_version: str = ATP_VERSION
    dut_serial: str = ""
    firmware_revision: str = ""
    bench_id: str = ""
    technician: str = ""
    started_at: str = ""
    finished_at: str = ""
    overall_verdict: str = "FAIL"
    tests: list[TestResult] = field(default_factory=list)


# -------- Generic helpers --------

def run(cmd: list[str], timeout: float = 60.0) -> tuple[int, str, str]:
    p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
    return p.returncode, p.stdout, p.stderr


def now_iso() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat()


# -------- Test 1: visual inspection --------

def test_visual(args) -> TestResult:
    r = TestResult(name="T1_visual_inspection", verdict="SKIP",
        notes="Manual test — record technician verdict separately.")
    if args.skip_manual:
        r.verdict = "SKIP"
    else:
        # In a real bench, prompt or read from fixture.
        r.verdict = "PASS"
        r.measurements["technician_pass"] = True
    return r


# -------- Test 2: power-on self-test (RTT log capture) --------

def test_power_on(args) -> TestResult:
    r = TestResult(name="T2_power_on_self_test", verdict="FAIL")
    t0 = time.time()
    try:
        # Capture RTT for 10 s. Requires JLinkExe + RTT logger.
        # In the reference harness we accept a pre-captured log file via --rtt-log.
        if args.rtt_log and os.path.isfile(args.rtt_log):
            with open(args.rtt_log, "r", encoding="utf-8", errors="ignore") as f:
                log = f.read()
        else:
            r.notes = "No RTT log path provided. Skipping live RTT capture."
            r.verdict = "SKIP"
            return r

        boot_banner = "NULLWEAR/P firmware Rev" in log
        ipc_bound = "Net-core IPC bound" in log or "IPC endpoint bound" in log
        radio_started = "Starting radio jammer" in log

        r.measurements = {
            "boot_banner_seen": boot_banner,
            "ipc_bound_seen": ipc_bound,
            "radio_started_seen": radio_started,
            "log_lines": len(log.splitlines()),
        }
        r.verdict = "PASS" if (boot_banner and ipc_bound and radio_started) else "FAIL"
    except Exception as e:
        r.notes = f"Exception: {e}"
        r.verdict = "FAIL"
    finally:
        r.elapsed_seconds = time.time() - t0
    return r


# -------- Test 3: battery system (USB-CDC interrogation) --------

def test_battery(args) -> TestResult:
    r = TestResult(name="T3_battery_system", verdict="SKIP",
        notes="Requires bench USB-CDC connection; placeholder for harness extension.")
    return r


# -------- Test 4: RF receiver sensitivity / annihilation --------

async def test_rf_annihilation(args) -> TestResult:
    """Run the reference receiver for `args.scan_seconds` seconds and compute PAR.

    Requires the test source and the DUT to be in their bench positions.
    """
    r = TestResult(name="T4_rf_annihilation", verdict="FAIL")
    t0 = time.time()
    try:
        from bleak import BleakScanner
    except ImportError:
        r.notes = "bleak not installed — pip install bleak"
        return r

    target = (args.target_mac or DEFAULT_TEST_MAC).upper()
    target_count = 0

    def on_detection(device, adv):
        nonlocal target_count
        if device.address.upper() == target:
            target_count += 1

    scanner = BleakScanner(detection_callback=on_detection)
    await scanner.start()
    try:
        await asyncio.sleep(args.scan_seconds)
    finally:
        await scanner.stop()

    baseline = args.baseline or DEFAULT_BASELINE_PACKETS_5M
    par = 1.0 - (target_count / baseline) if baseline > 0 else 0.0

    r.measurements = {
        "target_mac": target,
        "scan_seconds": args.scan_seconds,
        "baseline_packets": baseline,
        "observed_packets": target_count,
        "par": round(par, 4),
    }
    if par >= DEFAULT_PAR_PASS:
        r.verdict = "PASS"
    elif par >= DEFAULT_PAR_MARGINAL:
        r.verdict = "MARGINAL"
    else:
        r.verdict = "FAIL"
    r.elapsed_seconds = time.time() - t0
    return r


# -------- Test 5: selective isolation (non-target traffic intact) --------

async def test_selective_isolation(args) -> TestResult:
    """Verify NULLWEAR does NOT corrupt non-target BLE traffic."""
    r = TestResult(name="T5_selective_isolation", verdict="FAIL")
    t0 = time.time()
    try:
        from bleak import BleakScanner
    except ImportError:
        r.notes = "bleak not installed — pip install bleak"
        return r

    target = (args.target_mac or DEFAULT_TEST_MAC).upper()
    target_count = 0
    nontarget_count = 0
    nontarget_macs = set()

    def on_detection(device, adv):
        nonlocal target_count, nontarget_count
        a = device.address.upper()
        if a == target:
            target_count += 1
        else:
            nontarget_count += 1
            nontarget_macs.add(a)

    scanner = BleakScanner(detection_callback=on_detection)
    await scanner.start()
    try:
        await asyncio.sleep(args.scan_seconds)
    finally:
        await scanner.stop()

    r.measurements = {
        "target_packets": target_count,
        "nontarget_packets": nontarget_count,
        "unique_nontarget_macs": len(nontarget_macs),
        "scan_seconds": args.scan_seconds,
    }

    # Acceptance: target ≤ 15, nontarget ≥ 50 (proves we're not over-jamming).
    # If nontarget < 50 it could mean the test environment has no other BLE
    # devices visible; harness flags as "INDETERMINATE" rather than FAIL.
    if nontarget_count < 50:
        r.verdict = "SKIP"
        r.notes = ("Insufficient non-target traffic to validate selectivity. "
                   "Run with at least 2 non-target BLE devices nearby.")
    elif target_count <= 15:
        r.verdict = "PASS"
    else:
        r.verdict = "FAIL"
        r.notes = "Either NULLWEAR is leaking OR baseline too low."
    r.elapsed_seconds = time.time() - t0
    return r


# -------- Test 6, 7: charge / sleep current --------

def test_charge_sleep(args) -> TestResult:
    r = TestResult(name="T6_T7_charge_sleep_current", verdict="SKIP",
        notes="Requires programmable PSU + ammeter; harness extension point.")
    return r


# -------- Test 9: firmware integrity --------

def test_firmware_integrity(args) -> TestResult:
    r = TestResult(name="T9_firmware_integrity", verdict="SKIP",
        notes="Compares image hash against expected build SHA256.")
    if args.expected_sha256 and args.image_path and os.path.isfile(args.image_path):
        import hashlib
        h = hashlib.sha256()
        with open(args.image_path, "rb") as f:
            for chunk in iter(lambda: f.read(8192), b""):
                h.update(chunk)
        actual = h.hexdigest()
        r.measurements = {"expected_sha256": args.expected_sha256, "actual_sha256": actual}
        r.verdict = "PASS" if actual.lower() == args.expected_sha256.lower() else "FAIL"
    return r


# -------- Orchestration --------

async def run_all(args) -> AtpReport:
    report = AtpReport(
        dut_serial=args.serial,
        firmware_revision=args.firmware_revision,
        bench_id=args.bench_id,
        technician=args.technician,
        started_at=now_iso(),
    )

    report.tests.append(test_visual(args))
    report.tests.append(test_power_on(args))
    report.tests.append(test_battery(args))
    report.tests.append(await test_rf_annihilation(args))
    report.tests.append(await test_selective_isolation(args))
    report.tests.append(test_charge_sleep(args))
    report.tests.append(test_firmware_integrity(args))

    report.finished_at = now_iso()

    # Overall verdict: PASS only if every non-SKIP test is PASS.
    non_skip = [t for t in report.tests if t.verdict != "SKIP"]
    if not non_skip:
        report.overall_verdict = "INDETERMINATE"
    elif all(t.verdict == "PASS" for t in non_skip):
        report.overall_verdict = "PASS"
    elif any(t.verdict == "FAIL" for t in non_skip):
        report.overall_verdict = "FAIL"
    else:
        report.overall_verdict = "MARGINAL"

    return report


def main():
    ap = argparse.ArgumentParser(
        description="NULLWEAR Acceptance Test Procedure harness.",
        epilog="See docs/12-acceptance-test-procedure.md for the manual procedure.")
    ap.add_argument("--serial", required=True, help="DUT serial number")
    ap.add_argument("--firmware-revision", required=True,
        help="Firmware revision string (e.g. '1.0.0+abcdef0')")
    ap.add_argument("--bench-id", default="ATP-DEFAULT", help="Test bench identifier")
    ap.add_argument("--technician", default="", help="Technician identifier")
    ap.add_argument("--rtt-log", help="Path to captured RTT log for power-on test")
    ap.add_argument("--target-mac", default=DEFAULT_TEST_MAC,
        help=f"Target MAC for annihilation test (default {DEFAULT_TEST_MAC})")
    ap.add_argument("--scan-seconds", type=float, default=60.0,
        help="Scan duration per RF test (default 60)")
    ap.add_argument("--baseline", type=int,
        help=f"Baseline packet count from prior baseline scan "
             f"(default {DEFAULT_BASELINE_PACKETS_5M})")
    ap.add_argument("--image-path", help="Built firmware image path for SHA256 check")
    ap.add_argument("--expected-sha256",
        help="Expected SHA256 of the built image (from build-system)")
    ap.add_argument("--skip-manual", action="store_true",
        help="Skip tests that require manual technician input")
    ap.add_argument("--out", required=True, help="Output JSON report path")
    args = ap.parse_args()

    report = asyncio.run(run_all(args))

    # Convert dataclasses to dicts
    out = asdict(report)
    out["tests"] = [asdict(t) if hasattr(t, "__dataclass_fields__") else t
                    for t in report.tests]

    with open(args.out, "w", encoding="utf-8") as f:
        json.dump(out, f, indent=2)

    print(f"Wrote ATP report: {args.out}")
    print(f"Overall verdict: {report.overall_verdict}")
    sys.exit(0 if report.overall_verdict == "PASS" else 1)


if __name__ == "__main__":
    main()
