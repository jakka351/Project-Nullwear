# Changelog

All notable changes to Project NULLWEAR will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project adheres to [Semantic Versioning](https://semver.org/) once it reaches an operational baseline.

## [Unreleased]

### Added — empirical findings from a second-corpus telemetry analysis (2025–2026 dataset)
- A new 2025-2026 BLE telemetry corpus, captured across multiple Australian jurisdictions in a fresh database after the user's collection app was reset, was analysed and folded into a Rev B of the *Threat Validation Report* (companion PDF). Notable new findings:
  - **Cleartext serial-number broadcast.** Every observed Axon Body 3 / Body 3 Plus broadcasts its full hardware model code (e.g. `X60J0xxxN`, `X60M0xxx4`) in plaintext as bytes 14–22 of the FE6B service-data payload. This is a third independent stable identifier per advertisement, in addition to the MAC and the 80-bit FE6B identifier.
  - **MAC[3] byte = jurisdiction signature.** The middle byte of the OUI-prefixed MAC correlates strongly with the deploying jurisdiction (Vic Pol = `0x68`, NSW Pol = `0xA1/9A/C0/C1/C2/C5`, WA Pol = `0x7F`). Sub-model variant (`X60J` vs `X60M`) further distinguishes equipment classes by jurisdiction. Agency attribution becomes inferrable from passive radio observation alone.
  - **Shift-change wave detection.** A 90-minute Sydney station session captured three discrete waves of officer arrivals — operationally significant because it means a passive external observer can characterise station shift-change times without entering the building.
  - **Court-environment radio profile** distinguished from station-environment radio profile (low Axon density, high enterprise-access-control device density, name-broadcasting wayfinding devices like `"Court EXIT"`).
  - **Five-year longitudinal Axon fleet view** (2021–2026, five SQLite snapshots): zero Axon devices in May 2021 → 46 by Oct 2024 (Vic Pol Body 3 mass deployment) → fully rotated fleet by 2026; 35 unique X60-series serial numbers catalogued in plain text by passive observation.
- *Threat Validation Report* PDF rebuilt as **Rev B** (13 pages, was 10) — adds §12 longitudinal-and-multi-corpus addendum.

### Changed — strategic Mitigation Report §11.3 (vendor recommendation)
- Expanded vendor recommendation to specify that **RPA on the MAC alone is an incomplete fix**. Axon must additionally:
  - Redact or rotate the model + serial cleartext fields in bytes 14–22 of the FE6B payload.
  - Redact or randomise the 80-bit per-device identifier in bytes 2–10 of the FE6B payload.
  - Avoid encoding agency / jurisdiction information in the MAC[3] production-batch byte.
- *Mitigation Report* PDF rebuilt with the expanded §11.3 (still 11 pages).

### Honest correction
- The Rev A claim of a "3-year persistent device" (`00:25:DF:1E:B9:17`) was incorrect and has been retracted in Rev B. RaMBLE's database is cumulative, so a single 2021 detection propagated into all subsequent exports until the user reset the app. The cross-snapshot MAC overlaps among the older snapshots are mostly database-cumulation artefacts, not re-detections of the same device. The 2024 → 2026 zero overlap remains valid evidence of fleet rotation because the 2026 dataset was captured in a fresh database.

### Operational redactions
- All references to broadcast media, filming, and specific identifiable third-party detected devices removed from the Threat Validation Report at user request.
- Specific date and city name of one regional court session redacted to jurisdiction-only ("WA Magistrates Court", date redacted) at user request.

### Added — implementation feasibility pack
- **Police Implementation Guide** ([`IMPLEMENTATION-GUIDE.md`](IMPLEMENTATION-GUIDE.md)) — single-document decision-tree for agency decision-makers, week-by-week schedule from disclosure to officer-in-the-field.
- **Hardware Feasibility Pack** ([`feasibility-pack/`](feasibility-pack/)) — eight-document CM-ready bundle: SOW template, RFQ quick reference, BoM CSV, ATP summary, firmware-handoff workflow, mechanical summary, regulatory summary, bidder checklist.
- **Procurement RFQ template** ([`docs/19-procurement-rfq-template.md`](docs/19-procurement-rfq-template.md)) — fillable RFQ template covering scope, pricing format, qualification, schedule.
- **Field-test protocol hardening** ([`docs/10-field-testing-protocol.md`](docs/10-field-testing-protocol.md)) — added Steps 11–20 covering 30-day in-service trial, Axon-equipment compatibility (Body 3 / Signal Sidearm / Axon Aware / Axon Fleet), and evidentiary integrity tests.
- **Engineering Specification PDF re-issued as Rev B** incorporating v1.1 firmware §12.3.

### Added — v1.1 firmware (audited as v1.1.1)
- **Dual-signature matcher** in the radio jammer: keep the v1.0 OUI match as the fast path, add a Phase-2 fallback that matches on the BLE 16-bit Service Data UUID `0xFE6B` (registered to Axon Public Safety). Catches Axon devices broadcasting in their docked / non-deployed state with sanitised AdvA. See [`docs/17-firmware-v1.1-dual-signature-matcher.md`](docs/17-firmware-v1.1-dual-signature-matcher.md).
- New stats counter `pkts_uuid_matched` distinguishing Phase-2 hits from Phase-1 OUI hits.
- Build-time toggle `NULLWEAR_ENABLE_PHASE_2` (default `1`) to opt out and revert to v1.0 matching exactly.

### Fixed (v1.1.1 hostile-reviewer audit)
- IRQ binding now via Zephyr `IRQ_DIRECT_CONNECT` / `IRQ_CONNECT` (the bare `XXX_IRQHandler` pattern from v1.0/v1.1 would never have fired on Zephyr).
- Channel-hop timer moved from `TIMER0` (claimed by Zephyr) to `TIMER1`.
- `nrfx_dppi` dropped entirely — the unconditional `DISABLED → TXEN` link fired on every disable, including normal RX-end. Replaced with synchronous spin-wait (~10 µs added latency, still inside the empirically-validated CRC corruption window).
- `radio_arm_rx()` now restores RX-mode SHORTS after `launch_jam_pulse()` overwrote them.
- `radio_arm_rx()` now clears stale `EVENTS_BCMATCH` / `EVENTS_END` / `EVENTS_ADDRESS` before re-arming.
- Channel-hop ISR no longer races the `DISABLED` ISR — DISABLED is now the single point of re-arm.
- `battery_mgmt.c` missing `<zephyr/sys/reboot.h>` include added.

### Initial release
- Initial release of the NULLWEAR repository.
- README, LICENSE, SECURITY, CONTRIBUTING, CHANGELOG.
- Reference firmware for nRF5340 (network-core radio jammer + app-core management).
- Reference receiver verification tool (Python / `bleak`).
- ESP32 Axon BLE source emulator for lab testing.
- Documentation suite: overview, architecture, BLE primer, CRC corruption theory, hardware spec (short form), firmware architecture, build instructions, user manual, operations manual, field testing protocol, troubleshooting, acceptance test procedure, pilot deployment plan, security considerations, legal and regulatory analysis.
- Logo and system-architecture SVG diagrams.
- Companion documents (issued separately as PDF):
  - Mitigation Report.
  - Engineering Specification.
  - (Original disclosure report previously issued.)

### Status
- All firmware is reference implementation; awaiting test-compile and bench verification by the pilot contract manufacturer.
- All documentation is Rev A; subject to refinement during the pilot.
- No operational deployments yet.

## [0.1.0] — 2026-XX-XX (planned)

The first version that has been built, tested in the lab, and validated in the field. Will be tagged when:

- A pilot CM has produced units.
- ATP `docs/12-acceptance-test-procedure.md` has been passed by ≥ 95% of pilot units.
- Field testing per `docs/10-field-testing-protocol.md` has shown PAR ≥ 0.99 across the criteria in §"Pass / fail verdict".
- ACMA class-licence determination is in place.

## [1.0.0] — TBD

The first version cleared for operational issue to officers. Will be tagged when:

- The 30-day pilot operational review (Work Package 6 of the deployment plan) has shown < 2% in-service fault rate, no officer-safety incident attributable to NULLWEAR, no Axon-equipment-interference reports.
- A formal go decision for national rollout has been made.

---

*This changelog will track future versions as the project moves from reference-implementation to deployed capability.*
