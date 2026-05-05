# Changelog

All notable changes to Project NULLWEAR will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project adheres to [Semantic Versioning](https://semver.org/) once it reaches an operational baseline.

## [Unreleased]

### Added
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
