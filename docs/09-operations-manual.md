# NULLWEAR — Operations Manual (for the depot / quartermaster / asset manager)

**Audience:** equipment-locker quartermasters, depot technicians, asset managers, agency procurement officers, and anyone responsible for the lifecycle of NULLWEAR units in service.

---

## Table of contents

1. [Receipt and intake](#1-receipt-and-intake)
2. [Asset registration](#2-asset-registration)
3. [Issue to officer](#3-issue-to-officer)
4. [Daily operations](#4-daily-operations)
5. [Charging infrastructure](#5-charging-infrastructure)
6. [Periodic checks](#6-periodic-checks)
7. [Fault handling](#7-fault-handling)
8. [Returns and refurbishment](#8-returns-and-refurbishment)
9. [End-of-life](#9-end-of-life)
10. [Inventory and reporting](#10-inventory-and-reporting)
11. [Security](#11-security)
12. [Suppliers and replacements](#12-suppliers-and-replacements)

---

## 1. Receipt and intake

When a shipment of NULLWEAR/P, /V or /S devices arrives from the contract manufacturer:

1. Verify the carton against the packing list. Each carton typically contains 25 × NULLWEAR/P units in individual moulded blisters.
2. Inspect 5 random units per carton for cosmetic damage. Reject the carton if damage is observed in more than one of the five.
3. For each unit, scan the laser-etched serial number into the agency asset-management system using the supplied USB barcode/QR scanner. Do not skip this step — the asset system is the source of truth for which device belongs to which officer.
4. Place all units on the receiving-bench charging dock for 4 hours. New units ship at ~70% state-of-charge per IATA dangerous-goods rules; bring them to 100% before issue.
5. After charging, run the **intake acceptance procedure** in `docs/12-acceptance-test-procedure.md` on a 5% sample of every shipment. Reject the shipment if more than 1% of the sample fails.

## 2. Asset registration

Each NULLWEAR unit must be registered in the agency asset-management system with the following minimum fields:

| Field | Source |
|---|---|
| Serial number | Laser-etched on rear of unit |
| MFG date | Inside cover of unit blister |
| MFG batch | Inside cover of unit blister |
| Variant | /P, /V or /S |
| Firmware revision | Read via USB-CDC during intake test |
| Issued to | Officer badge number, on issue |
| Issue date | On issue |
| Last charge cycle | Auto-updated by the dock if equipped |
| Last fault report | On fault report |
| Status | InStock / Issued / Charging / FaultRMA / EndOfLife |

For agencies with no existing asset system, a CSV-backed register at minimum is acceptable. A reference SQLite schema is in `docs/asset-schema.sql` (TBD).

## 3. Issue to officer

When issuing a unit:

1. Verify the officer's identity and shift status in the agency duty roster.
2. Select an `InStock` unit from the charged-and-tested rack.
3. Update the asset record: `Status` → `Issued`, `Issued to` → badge number, `Issue date` → today.
4. Brief the officer on the User Manual (`docs/08-user-manual.md`). For a fleet roll-out, this can be a 5-minute group briefing during shift handover; no formal certification is required.
5. Hand over the unit, the supplied belt clip, and a copy of the User Manual.
6. Confirm the LED is green when the officer presses the button.

## 4. Daily operations

A NULLWEAR/P device requires no daily intervention from the depot once issued. The officer is responsible for:

- Wearing it on shift.
- Returning it to the dock at end of shift.
- Reporting fault states.

The depot is responsible for:

- Maintaining the dock infrastructure (see §5).
- Logging returned units.
- Monitoring overall fleet health (see §6).

## 5. Charging infrastructure

NULLWEAR/P charges over USB-C at 5 V / 500 mA. Acceptable charging sources:

- The **integrated NULLWEAR dock** (preferred) — installs alongside the existing Axon body-camera dock; auto-detects unit attachment; logs charge cycles to a central depot dashboard via Ethernet.
- A **standard USB-C wall charger** — any compliant 5 V / ≥ 1 A USB-C power source. Hands-off, but does not log to the asset system.
- A **standard USB-C cable from a laptop** — only for diagnostic/intake use. Not for routine end-of-shift charging.

Charging time: from empty (≤ 5% SoC) to 100% in ≤ 60 minutes. From 50% to 100% in ≤ 30 minutes (typical end-of-shift case). The unit may safely remain on the dock indefinitely; the charger automatically holds at 100% with no overcharge.

**Do not use** PD (USB Power Delivery) chargers above 5 V negotiated voltage. The unit will protect itself but charging will be slower.

## 6. Periodic checks

| Cadence | Check |
|---|---|
| Daily | Dock health: confirm all dock ports are powered and reporting status. |
| Weekly | Random functional sample: select 5 units from the dock, run the abbreviated functional test in §12.4 of the ATP. |
| Monthly | Asset reconciliation: confirm all unit serials physically present match the asset system. Investigate discrepancies same-day. |
| Quarterly | Field-test sample: select 10 units in service, run the field-testing protocol in `docs/10-field-testing-protocol.md` on each, report results. |
| Annually | Full fleet acceptance re-test of a 1% random sample. Battery wear assessment. |

## 7. Fault handling

When a fault report comes in from an officer:

| Reported fault | Action |
|---|---|
| LED red rapid blink | Receive unit; tag as `FaultRMA`; issue replacement; transmit unit to depot for diagnosis. |
| LED off after charge cycle | Same. |
| Cosmetic damage but functional | Visual assessment; if in-tolerance, return to service; if out-of-tolerance, RMA. |
| Officer suspects unit is not protecting them | Run the field-testing protocol against the unit (see `docs/10-field-testing-protocol.md`) immediately. If PAR < 0.95, RMA. |
| Lost / stolen | Immediately update asset system: `Status` → `Lost` or `Stolen`. Notify the agency security officer. Issue replacement. |

For depot diagnosis of an RMA unit:

1. Connect the unit via USB-C to a depot diagnostic computer.
2. The unit enumerates as a USB-CDC serial port. Open at 115200 baud.
3. Type `?` and press Enter. The unit will print its firmware version, build date, recent statistics, and recent fault log.
4. Cross-reference fault log entries with the firmware-known fault table in `docs/11-troubleshooting.md`.
5. If recoverable (battery deep-discharge, firmware-corrupt), reflash via the USB-C bootloader and re-test.
6. If unrecoverable (RF chain fault, mechanical damage), tag for end-of-life processing.

## 8. Returns and refurbishment

Units that fail in service but have repairable defects should be returned to the contract manufacturer for refurbishment under the supply contract. Refurbishment scope:

- Battery replacement (battery is not field-serviceable; depot-level replacement is also not supported because of the potting compound).
- Enclosure replacement (only if the original was sealed in a way that allows the PCB to be transferred).
- Firmware reflash to current revision.
- Re-acceptance test per `docs/12-acceptance-test-procedure.md`.

A refurbished unit is re-issued under the same serial number. The asset record is updated with the refurbishment date.

## 9. End-of-life

A NULLWEAR unit reaches end-of-life when:

- Battery has reached 80% capacity retention (typically 500 charge cycles ≈ 2 years of daily use, depending on shift pattern); **and**
- A refurbishment cycle is uneconomical or impractical; **or**
- The firmware revision is no longer supported by the current production stream.

End-of-life processing:

1. Update asset system: `Status` → `EndOfLife`.
2. Discharge the battery to ≤ 30% SoC for safe transport.
3. Bag and label per AS/NZS 5377 (lithium battery e-waste).
4. Send to an authorised e-waste recycler. Retain the certificate of disposal.
5. Issue a replacement unit to the same officer.

Expected service life: 7+ years with one mid-life refurbishment.

## 10. Inventory and reporting

Monthly reporting cadence to agency leadership:

| Metric | Target |
|---|---|
| Units in active issue | (depends on agency) |
| Units in stock | ≥ 5% of active fleet |
| Units in RMA | ≤ 2% of active fleet |
| Field-test PAR (rolling 90-day average) | ≥ 0.99 |
| Mean time between fault reports | ≥ 1000 days |
| Charge-cycle distribution (histogram) | Distributed across fleet |
| Fleet age distribution | Distributed |

Quarterly reporting to the National Coordination Committee (TBD) on:

- Aggregate PAR.
- Aggregate fault rates and RMA reasons.
- Any field observations of attempted attacker adaptation.

## 11. Security

NULLWEAR contains no operational, classified, or personally-identifying information. It is not in itself a security-controlled item. However:

- The **firmware** is signed with the agency's MCUboot key. Do not leak signing keys.
- The **fleet asset register** is operationally sensitive: it links serial numbers to badge numbers, and could in principle be used by an adversary to map officer identities. Treat the asset register as PROTECTED.
- The **fault log** stored in NVM on the unit contains nothing identifying, only RF event statistics.
- The **dock infrastructure** Ethernet network should be segmented from general agency LAN; treat the dock VLAN as a privileged management network.

Do not connect a NULLWEAR unit to any untrusted USB host. The unit will resist firmware modification by signature check, but the supply chain is the principal threat surface.

## 12. Suppliers and replacements

Reference suppliers (refer to the supply contract for current authorised vendors):

- Primary CM: domestic Australian (or NZ / UK / US / Canada) — **do not use Chinese contract manufacturers** for this project.
- Battery cell: PowerStream PGEB0042530 or domestically-sourced 200 mAh LiPo with PCM.
- nRF5340 SoC: Nordic Semiconductor (direct or via Mouser / Digi-Key / element14).
- RF front-end: Skyworks SKY66112-11.
- Chip antenna: Johanson 2450AT43A100.
- Enclosure tooling: domestic Australian injection-mould house.

For full BoM see `docs/05-hardware-spec.md`.

For replacement orders: agency procurement should retain an authorised stock of ~10% of active fleet. Lead time for new manufacture is 8–12 weeks; refurbished units can typically be turned around in 2 weeks.

---

For technical issues with the unit itself, see `docs/11-troubleshooting.md`. For acceptance testing of new shipments, see `docs/12-acceptance-test-procedure.md`. For operational verification in the field, see `docs/10-field-testing-protocol.md`.
