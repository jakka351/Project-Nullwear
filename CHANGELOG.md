# Changelog

All notable changes to Project NULLWEAR will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project adheres to [Semantic Versioning](https://semver.org/) once it reaches an operational baseline.

## [Unreleased]

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
