# Statement of Work — Project NULLWEAR

**Issuing agency:** `<agency name>`
**Project reference:** `<agency PR number>`
**SOW version:** `<v1.0>`
**Date:** `<date>`

This SOW is a contractual schedule. The contractor (the "Manufacturer") shall perform all work described herein to produce the deliverables described in §3.

## 1. Background

The Manufacturer shall produce small body-worn (and vehicle / station variant) radio devices for the protection of `<agency name>`'s sworn officers from a documented passive third-party tracking threat. The complete reference design is in the open-source project repository at `<URL>`. This SOW assumes the Manufacturer has read the *Engineering Specification* (Rev B) PDF before signing.

## 2. Definitions

| Term | Meaning |
|---|---|
| Unit | A single completed NULLWEAR device of any variant. |
| Variant | One of NULLWEAR/P (personal), NULLWEAR/V (vehicle), NULLWEAR/S (station). |
| ATP | Acceptance Test Procedure (`docs/12-acceptance-test-procedure.md`). |
| FAT | First-Article Test (the first 5 units of any new tooling are subject to additional inspection). |
| AQL | Acceptable Quality Level (statistical sampling per ISO 2859-1). |

## 3. Deliverables

### 3.1 Units

| Variant | Pilot quantity | Production option quantity |
|---|---|---|
| NULLWEAR/P | `<200>` | `<50,000>` over `<12>` months |
| NULLWEAR/V | `<20>` | `<25,000>` over `<12>` months |
| NULLWEAR/S | `<5>` | `<600>` over `<12>` months |

### 3.2 Per-unit deliverables

For every Unit:

- The Unit, conforming to the engineering specification.
- Sealed individual packaging (moulded blister + a one-page laminated quick-start card).
- A signed ATP report in JSON format conforming to the schema in `firmware/tools/atp/atp_schema.json`.
- An asset-system feed entry (per §6 below).

### 3.3 Tooling

- Injection-mould tool for the NULLWEAR/P enclosure (one-off, retained for future Production Option exercise).
- PCB assembly stencil and pick-and-place programs.
- RF acceptance-test fixture per ATP Test 4.
- Final-assembly jig and through-hole hand-solder fixture.

Tooling becomes the Agency's property at acceptance, with rights for the Agency to re-tender future production using the tooling and CAD.

### 3.4 Documentation

- Production assembly process document.
- Per-batch component traceability log.
- FAT report.
- Per-unit ATP reports (as §3.2).
- Final consignment manifest.

## 4. Specifications

The complete specification is in the *Engineering Specification* (Rev B) PDF. Key acceptance criteria (mandatory):

| Property | Required |
|---|---|
| Functional | All ATP tests pass. PAR ≥ 0.99 at 5 m on Test T4. |
| Mechanical | IP67 (1 m, 30 min), 1.5 m drop survival on 6 faces. |
| Operating temperature | -10 to +55 °C |
| Battery endurance | ≥ 18 hours per charge from full |
| Radio compliance | Per `<jurisdictional regulator>` determination supplied by Agency |
| EMC | AS/NZS CISPR 32 Class B (or jurisdictional equivalent) |
| Lithium-battery transport | IATA UN3481 PI967 |
| RoHS / REACH | As applicable |

## 5. Manufacturing requirements

### 5.1 Location

Final assembly, test and packaging shall be performed at the Manufacturer's `<Australian / NZ / UK / US / Canadian>` facility. Components may be sourced internationally from authorised distributors.

**Chinese assembly is not authorised under this SOW.**

### 5.2 Component substitution

Substitution of any component listed in the BoM (see `03-bill-of-materials.csv`) requires written approval from the Agency's Technical Liaison. The approval will be granted only when:

- The substitute is from an authorised distributor (Mouser, Digi-Key, Element14, Arrow, or domestic equivalent).
- The substitute meets or exceeds the original part's electrical and mechanical specification.
- The substitution does not cause any ATP criterion to fail.

### 5.3 Yield

| Phase | Maximum acceptable reject rate |
|---|---|
| Pilot (first 200 units) | 5% |
| Production (next 1,000) | 2% |
| Steady-state production | 1% |

### 5.4 Sample-rate testing

| Test | Sample rate |
|---|---|
| ATP T1 (visual) | 100% |
| ATP T2 (POST) | 100% |
| ATP T3 (battery system) | 100% |
| ATP T4 (RF annihilation) | 100% |
| ATP T5 (selective isolation) | 100% |
| ATP T6/T7 (charge/sleep current) | 100% on a 5% sample, statistical inference for the rest |
| ATP T8 (mechanical: IP67 + drop) | 1% AQL per batch |
| ATP T9 (firmware integrity SHA-256) | 100% |

## 6. Asset-system integration

For each shipped Unit, the Manufacturer shall transmit a JSON record to the Agency's asset-management endpoint within 24 hours of dispatch. Format per `firmware/tools/atp/atp_schema.json`. Endpoint URL and authentication details to be supplied by Agency at contract execution.

## 7. Firmware signing pipeline

The Agency holds the MCUboot signing private key in an HSM. The Agency shall provide the corresponding public key to the Manufacturer at contract execution.

The Manufacturer shall:

- Use only signed firmware images that verify against the Agency-supplied public key.
- Receive signed firmware images via the Agency's secure-distribution channel (per `CONTACT.md`); never receive or hold the private key.
- Verify each unit boots and the bootloader confirms image authenticity before passing ATP T9.
- Log per-unit firmware revision in the ATP report.

## 8. Schedule

| Milestone | Days from contract execution |
|---|---|
| Kick-off meeting | 5 |
| BoM confirmation, tooling order | 14 |
| First-article (5 units) | 35 |
| FAT report submitted | 42 |
| Pilot 200-unit consignment shipped | 56 |
| Pilot consignment received by Agency | 63 |

If the Production Option is exercised:

| Milestone | Days from option exercise |
|---|---|
| First production batch (1,000 units) | 30 |
| Steady-state production rate (`<5,000>` units/month) | 60 |
| Production complete | per option terms |

## 9. Pricing

Per the RFQ pricing format. Variations after contract execution must be requested in writing and accepted by the Agency procurement officer.

## 10. Warranty

The Manufacturer warrants each Unit for `<24>` months from delivery. Warranty covers manufacturing defects (electrical, mechanical, firmware-flashing). Warranty excludes battery wear past the specified 500-cycle service life and any user damage.

## 11. Refurbishment

The Manufacturer shall offer a refurbishment service for Units returned post-warranty (or for in-warranty units with user damage). Refurbishment scope: battery replacement, enclosure replacement, firmware reflash, re-acceptance test. Pricing per the RFQ pricing format.

## 12. Termination

Standard agency termination clauses apply. In addition:

- The Agency may terminate for material breach (e.g. yield falling below the §5.3 thresholds).
- The Agency may terminate for convenience subject to payment for in-progress work to date.
- Upon termination, all tooling, CAD, and per-unit records become Agency property.

## 13. Acceptance

A consignment is accepted upon the Agency's signature of a delivery receipt PROVIDED:

- All Units in the consignment are accompanied by valid ATP reports.
- The yield falls within the §5.3 thresholds.
- The packaging is intact and tamper-evident.

The Agency reserves the right to re-test 10% of any consignment at its own bench within 14 days of receipt. If the re-test reject rate exceeds the §5.3 threshold, the consignment may be rejected and returned at the Manufacturer's cost.

---

**Signed for the Agency:** `<name, title, date>`

**Signed for the Manufacturer:** `<name, title, date>`
