# Procurement Request for Quotation (RFQ) — Template

A ready-to-send RFQ template that an agency procurement officer can fill in with `<bracketed>` agency-specific values and submit to candidate contract manufacturers.

The template assumes the bidder has access to this open-source repository plus the companion *Project NULLWEAR Engineering Specification* PDF (which together constitute the complete reference design).

---

## How to use this template

1. Open this file in any markdown / text editor.
2. Replace every `<placeholder>` with your agency's actual value.
3. Delete any sections not relevant to your jurisdiction.
4. Convert to PDF or your agency's standard RFQ format and circulate.
5. Suggested distribution: minimum 3 candidate contract manufacturers, **all domestic** (Australian, NZ, UK, US, or Canadian — explicitly NOT Chinese — for supply-chain integrity reasons documented in `docs/16-secrets-and-publishing-policy.md`).

---

# REQUEST FOR QUOTATION

## Project NULLWEAR — Counter-Surveillance Radio Device

**Issuing agency:** `<agency name>`
**RFQ reference:** `<agency RFQ number>`
**Issue date:** `<date>`
**Bid deadline:** `<date — recommend 4 weeks from issue>`
**Procurement officer:** `<name and contact>`
**Technical liaison:** `<name and contact>`
**Classification of this RFQ:** `<jurisdictional equivalent of Australia OFFICIAL: Sensitive>`

---

## 1. Background

The agency is procuring a counter-surveillance radio device, designated **NULLWEAR/P** (personal-issue), to mitigate a documented passive third-party tracking threat against officer-issued Axon Enterprise Bluetooth Low Energy equipment. The technical, legal and operational rationale is set out in the companion documents listed in §3 below. This RFQ is the manufacturing-side follow-on to that work.

The agency intends an initial pilot purchase of `<200>` units, with options to scale to `<50,000>` units across the agency's fleet over the following `<12 months>` if pilot acceptance is met.

## 2. Scope of supply

| Variant | Quantity (pilot) | Quantity (option, full rollout) |
|---|---|---|
| NULLWEAR/P — personal-issue body-worn | `<200>` | `<50,000>` |
| NULLWEAR/V — vehicle-mounted | `<20>` | `<25,000>` |
| NULLWEAR/S — station-mounted distributed-antenna | `<5>` | `<600>` |

Inclusions for each unit:

- Sealed device per the engineering specification (BoM, mechanical, firmware, signing pipeline).
- Individual moulded blister packaging.
- Belt clip / mounting hardware as relevant to variant.
- One-page laminated user quick-start card (text supplied by agency; CM to print and insert).
- Per-unit acceptance test report conforming to the JSON schema in `firmware/tools/atp/atp_schema.json`.
- Asset-system upload of per-unit serial number and firmware revision.

Exclusions:

- The agency-specific MCUboot signing key generation and management (agency holds this; CM uses agency-provided public key for verification only).
- Asset-management database integration (agency to perform; CM provides the data feed in the JSON-schema format).

## 3. Reference documents (supplied by agency)

The bidder is referred to the following documents, all available at `<URL of the agency-mirrored repository or the public GitHub repository>`:

1. **Engineering Specification (PDF, Rev B)** — full technical specification including BoM, mechanical drawings, electrical reference design, firmware reference, and acceptance test procedure.
2. **Mitigation Report (PDF)** — strategic rationale (read for context only; not part of the deliverable).
3. **Project repository** — open-source, MIT-licensed, containing:
   - Reference firmware sources (`firmware/nullwear-p/`)
   - Verification tools (Python reference receiver + ESP32 emulator)
   - Acceptance test procedure (`docs/12-acceptance-test-procedure.md`)
   - Field test protocol (`docs/10-field-testing-protocol.md`)
   - Hardware specification (`docs/05-hardware-spec.md`)
   - All other documentation listed in `docs/00-INDEX.md`

The bidder is expected to read all of the above before submitting a quotation.

## 4. Hardware reference (summary — see Engineering Specification for full BoM)

Bidders may propose alternative parts where they offer equivalent or better performance at lower cost, provided alternatives are flagged as such in the response and the substitution preserves all acceptance criteria.

### NULLWEAR/P principal BoM (~30 components per unit)

| Function | Reference part (MPN) | Quantity per unit |
|---|---|---|
| SoC (dual-core, 2.4 GHz radio) | Nordic nRF5340 (any QFN-94 variant) | 1 |
| RF front-end | Skyworks SKY66112-11 | 1 |
| LiPo charger | Microchip MCP73831T | 1 |
| Fuel gauge | Maxim MAX17048 | 1 |
| 3.3 V LDO | TI TLV75533 | 1 |
| Load switch | TI TPS22918 | 1 |
| ESD/TVS for USB-C | ST USBLC6-2SC6Y | 1 |
| Reverse-polarity P-MOSFET | Diodes DMP3098L | 1 |
| 32 MHz crystal | Abracon ABM3B-32.000MHZ | 1 |
| 32.768 kHz crystal | Abracon ABS07-32.768KHZ | 1 |
| 2.4 GHz chip antenna | Johanson 2450AT43A100 | 1 |
| LiPo cell (200 mAh, with PCM) | PowerStream PGEB0042530 (or 2nd-source LP402530) | 1 |
| USB-C right-angle SMD receptacle | Amphenol 12401610E4#2A | 1 |
| RGB LED (0805) | Cree CLV1A-FKB | 1 |
| Tactile switch (sealed) | C&K KMR2 | 1 |
| Passives (MLCC, resistors, inductors) | Various | ~35 |
| PCB | 4-layer FR-4, 1.2 mm, ENIG finish | 1 |
| Enclosure | PC-ABS injection-moulded, 2-piece, ultrasonically welded | 1 |
| Belt clip | Stainless 304 | 1 |
| Light pipe | 3 mm acrylic rod, 12 mm length | 1 |
| Polyurethane potting (per unit fill) | Wevolt PU-7000 | 1 |

Indicative reference unit cost (from agency's prior costing — bidders are not required to match): **~AUD 56 per unit at 50,000 volume**, components only. Bidders to quote their own pricing including assembly, test, packaging, margin, and any tooling amortisation.

NULLWEAR/V and NULLWEAR/S BoM deltas are listed in the Engineering Specification §27 and §28 respectively.

## 5. Specifications and acceptance criteria

### 5.1 Functional

The unit must pass every test in the **Acceptance Test Procedure** (`docs/12-acceptance-test-procedure.md`) at the CM's bench prior to shipment to the agency. Specifically:

- T1 (Visual inspection): pass.
- T2 (Power-on self-test): boot banner, IPC bound, radio jammer started.
- T3 (Battery system): VBAT > 3.7 V at shipment, MAX17048 detected.
- **T4 (RF receiver / annihilation): PAR ≥ 0.99 at 5 m against the supplied ESP32 reference source.**
- **T5 (Selective isolation): no corruption of non-target BLE traffic.**
- T6/T7 (Charge / sleep current): per the spec.
- T9 (Firmware integrity): SHA-256 matches CI-built artefact.

Reject rate at ATP must be < 5% during the pilot phase, declining to < 1% at full production.

### 5.2 Mechanical

- IP67 (dust-tight, 1 m water immersion for 30 minutes). Verified at 1% AQL per batch.
- 1.5 m drop onto concrete on each of 6 faces. Verified at 1% AQL.
- Operating temperature -10 to +55 °C.
- Storage temperature -20 to +60 °C.
- Salt-fog resistance per MIL-STD-810G Method 509.6, 48 h (for coastal / maritime use).

### 5.3 Regulatory compliance

- **Radio compliance for the operating jurisdiction** (`<ACMA / FCC / Ofcom / ISED / RSM>`). The agency will provide the regulatory determination authorising the device class; the CM is responsible for ensuring the manufactured units comply with the granted determination's parameters.
- **EMC compliance** to AS/NZS CISPR 32 Class B (Australia) or jurisdictional equivalent.
- **Battery transport compliance** (IATA UN3481 PI967 lithium-cell-installed-in-equipment).
- **RoHS, REACH** as applicable to the destination jurisdiction.
- **Conflict-mineral disclosure** per the agency's standard procurement requirements.

### 5.4 Supply chain integrity

The agency requires:

- **Domestic manufacturing.** All assembly and test must be performed in `<Australia / NZ / UK / US / Canada>`. Components may be sourced internationally but final assembly must be domestic. **Chinese contract manufacturers are not eligible for this procurement.**
- **Component traceability.** Every unit's serial number is logged against the silicon batch and battery batch used in its assembly.
- **Tamper-evident packaging** for individual unit blisters.
- **Signed firmware only.** The agency provides the public half of the MCUboot signing keypair; the CM flashes only firmware images that verify against this key. The CM does not hold the private signing key under any circumstances.
- Bidder must declare any foreign-government ownership or control of the bidder entity or its supply chain.

## 6. Production schedule

| Milestone | Target |
|---|---|
| Contract award | `<date>` |
| First-article acceptance | Contract award + 6 weeks |
| Pilot quantity (200 NULLWEAR/P units) delivered | Contract award + 8 weeks |
| Pilot field-test results published by agency | Pilot delivery + 6 weeks |
| Decision on full rollout | Contract award + 14 weeks |
| Full rollout commencement | Per option exercise |

## 7. Pricing format

Bidders to quote in `<currency>` and provide:

| Line | Variant | Pilot qty | Pilot unit price | Production qty | Production unit price |
|---|---|---|---|---|---|
| 1 | NULLWEAR/P | 200 | `<>` | 50,000 | `<>` |
| 2 | NULLWEAR/V | 20 | `<>` | 25,000 | `<>` |
| 3 | NULLWEAR/S | 5 | `<>` | 600 | `<>` |
| 4 | Tooling (one-off, mechanical) | n/a | `<>` (pilot share) | n/a | `<>` (production share) |
| 5 | Test fixtures (one-off, RF chamber + bench harness) | n/a | `<>` | n/a | n/a |
| 6 | NRE (per-unit firmware programming + serialisation) | per unit | `<>` | per unit | `<>` |
| 7 | Per-unit ATP run | per unit | `<>` | per unit | `<>` |
| 8 | Packaging (blister + outer carton) | per unit | `<>` | per unit | `<>` |
| 9 | Refurbishment service (per returned unit) | n/a | `<>` | n/a | `<>` |

Pricing assumptions, lead times, and the expiry date of the quoted prices to be stated separately.

## 8. Bidder qualification

Bidders must demonstrate:

| Requirement | Acceptable evidence |
|---|---|
| ISO 9001:2015 certification | Current certificate |
| Prior experience with safety-critical or government-grade electronics | Reference projects (sanitised) |
| Domestic manufacturing capability | Site address, photographs |
| RF design / EMC test experience | Test-house relationship or in-house chamber |
| Capacity to deliver pilot quantity in 8 weeks | Signed statement |
| Capacity to scale to 50k in 12 months | Production-line plan |
| Ability to handle MCUboot signed-firmware workflow | Brief technical description |
| Cyber-hygiene posture (Essential Eight or equivalent) | Self-attestation |
| Australian Industry Capability content (or jurisdictional equivalent) | Percentage and breakdown |
| Disclosure of foreign-government affiliation | Statutory declaration |

## 9. Submission

Submit your response in PDF to `<procurement officer email>` no later than `<deadline>`. Include:

- Cover letter and bidder details
- Filled §7 pricing table
- §8 qualification evidence
- Technical proposal (any deviations from the reference design must be flagged here)
- Project plan
- References (3 minimum)

Late or incomplete responses will not be considered.

## 10. Confidentiality

This RFQ document and any agency-supplied attachments are classified `<jurisdictional equivalent of OFFICIAL: Sensitive>` and may not be circulated outside the bidder's responding organisation without the agency's written consent.

The companion documents (Engineering Specification PDF, Mitigation Report PDF, Threat Validation Report PDF if shared) are CONFIDENTIAL — RESPONSIBLE DISCLOSURE and are subject to the same restriction.

The open-source repository itself is publicly available; bidders may reference it freely.

## 11. Contracting

Standard agency procurement terms apply. The agency reserves the right to:

- Award to one or more bidders (split award).
- Decline to award (no contract).
- Negotiate with shortlisted bidders prior to award.
- Vary the option quantities at the agency's discretion.

Intellectual property:

- The reference design is open-source MIT-licensed; no IP transfer is required.
- Bidder-developed CAD files (PCB layout, mechanical CAD, test fixtures) become the agency's property at acceptance, with rights for the agency to re-tender future production using those artefacts.

---

# END OF RFQ TEMPLATE

---

## Appendix A — Indicative unit-cost reasonableness check

Sanity numbers the procurement officer can use to assess whether a bidder's quote is in the right neighbourhood. **These are indicative only. Real bids will vary by ±30% based on the bidder's overheads, tooling amortisation, margin policy, and current component-market conditions.**

| Cost component | Indicative AUD per unit (50k volume) | Source |
|---|---|---|
| BoM total (components only) | $32 | Engineering Spec §26 |
| PCB fabrication (4-layer, ENIG) | included in BoM | |
| SMT assembly + AOI + AXI | $4–6 | typical CM quote for ~30 components |
| Through-hole + final assembly | $2–3 | |
| Conformal coating + potting | $2 | |
| ATP per-unit testing | $3 | |
| Packaging | $1.20 | |
| **Direct cost** | **~$45** | |
| Margin, overhead, contingency | $11–15 | typical 25–30% on direct |
| **All-in unit cost** | **~$56–60** | engineering target |

A bid coming in at < $40 should be scrutinised for whether the bidder has actually read the spec or has hidden costs. A bid coming in at > $90 should be scrutinised for what the bidder is including beyond the reference design (or whether they are over-margining).

## Appendix B — Pre-submission technical questions (bidders should ask)

Encourage bidders to ask any of these BEFORE submitting, not after award:

1. Will the agency provide an example signed firmware image so we can validate our flashing pipeline against the production keypair?
2. Are there any agency-specific deviations to the per-unit packaging (logo, asset-tag printing, language localisation)?
3. Does the agency intend to operate the dock infrastructure that NULLWEAR/P charges on, or is dock supply also part of this procurement?
4. What is the agency's policy on returns / refurbishment of units that fail in the field after issue?
5. What is the agency's expected schedule for the next firmware revision (so the CM can plan firmware-update tooling)?

## Appendix C — Post-award integration checklist

Once a CM is selected, the agency procurement office should ensure the following are sequenced in the first two weeks:

- [ ] NDA executed.
- [ ] Agency MCUboot public key handed off to CM via the secure channel in `CONTACT.md`.
- [ ] CM granted access to the agency's mirror of the project repository (or the public GitHub repository).
- [ ] First-article RF acceptance test scheduled at the CM's bench.
- [ ] Asset-management feed format confirmed.
- [ ] Logistics: shipping labels, receiving address at the agency's depot, consignment insurance.
- [ ] Public-relations posture agreed (does the agency want to publicise the procurement, or run quietly?).
