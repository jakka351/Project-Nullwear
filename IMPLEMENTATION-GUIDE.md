# Police Implementation Guide

**Audience:** the senior officer or government decision-maker who has just been handed Project NULLWEAR and asked "what do we actually do with this?"

**Reading time:** 15 minutes for the whole document. 3 minutes for the executive summary alone.

**Outcome of following this guide:** units in the hands of officers, on a 90-day schedule, for under AUD 30 million at full national rollout (or under AUD 1 million for a single state-police pilot).

---

## Executive summary (3 minutes)

You have been given an open-source mitigation for a real, demonstrated radio-frequency surveillance threat against police officers carrying Axon body cameras / Tasers / smart holsters. The threat: passive third-party Bluetooth scanners can identify and track any officer with Axon equipment from up to ~400 m away. The mitigation: a small body-worn radio device, **NULLWEAR/P**, that destroys the Axon BLE broadcast in flight before any third-party scanner can receive it. Cost per device: about **AUD 56**. Time from contract to officer-in-the-field: **90 days** for a pilot.

What you do next, in this order:

1. **Read the threat.** The strategic *Mitigation Report* PDF. Half an hour. (If you've read it, skip.)
2. **Decide whether to act.** Your call. If yes, read on. If no, archive.
3. **Phone three people in this order:** your General Counsel; your radio-spectrum regulator (ACMA in Australia); your procurement officer.
4. **Issue the RFQ in `docs/19-procurement-rfq-template.md`** to three domestic contract manufacturers. Award in 4 weeks.
5. **Take delivery of 200 pilot units** in 8 weeks from contract award.
6. **Run the field test in `docs/10-field-testing-protocol.md`** in 6 weeks.
7. **Issue to officers** at week 14.
8. **Decide on national rollout** at week 20 based on pilot data.

The detail of each step is in the rest of this document.

---

## Step 0 — Confirm you understand what NULLWEAR is and is not

| It is | It is not |
|---|---|
| A small radio device worn by each officer | A network application |
| ~50 × 35 × 11 mm, ~28 g, $56 each | An Axon firmware update |
| Carried alongside the officer's existing Axon body camera | A modification to the body camera |
| A defensive jammer for ONE specific frequency-and-protocol pattern | A broadband jammer |
| Open-source and freely buildable by any contract manufacturer | A vendor-supplied black box |
| Already empirically validated against 3.4 years of real BLE telemetry | Untested theory |

If you are still not sure what NULLWEAR is, read [`docs/01-overview.md`](docs/01-overview.md) before going further.

---

## Step 1 — General Counsel meeting (week 1)

**Bring to the meeting:** this guide + [`docs/15-legal-and-regulatory.md`](docs/15-legal-and-regulatory.md) + the strategic Mitigation Report PDF.

**Decision required:** Is the agency willing to deploy a device whose operation may be characterised as "selectively interfering with a third party's radio reception of the agency's own equipment"?

**Counsel's likely response:**

- *Q: Is this legal?* Answer: requires a determination from the radio-spectrum regulator (ACMA in AU; FCC / Ofcom / ISED elsewhere). The legal pathway is identified in `docs/15-legal-and-regulatory.md`. Subject to confirmation by counsel familiar with the *Radiocommunications Act 1992* (Cth) or jurisdictional equivalent.
- *Q: Are there evidentiary risks?* Answer: NULLWEAR operates at the BLE radio link layer. It does not interact with the body camera's video / audio capture, encoding, storage, hash, or upload. This is verified in the field-test protocol (`docs/10`, Steps 18–20). General Counsel should pre-prepare a one-page disclosure information sheet for any future court tendering of pilot recordings.
- *Q: What's the public-relations risk if this leaks?* Answer: the project is open-source. The threat has been publicly disclosed (DEF CON 31, 2023; this repository, 2026). Operating a defensive countermeasure against a publicly-known threat is a defensible posture; concealing the threat while not acting is not.

**Decision matrix:**

| Counsel's verdict | Your next move |
|---|---|
| Green light | Step 2 |
| Conditional (specific concerns) | Address each, then Step 2 |
| Red light | Document the reasons, archive the project, and ensure the agency's officer-safety committee has the threat brief on file for future re-consideration. Do not proceed. |

---

## Step 2 — Radio regulator engagement (weeks 1–2)

**Action:** Pre-application meeting with `<your jurisdiction's radio regulator>`.

In Australia: the Australian Communications and Media Authority (ACMA). The pathway is described in `docs/15-legal-and-regulatory.md`.

**Bring to the meeting:** the strategic Mitigation Report PDF + the Engineering Specification PDF + this Implementation Guide.

**Outcome required:** a Ministerial determination under s.27 of the *Radiocommunications Act 1992* (Cth) authorising operation of devices in the NULLWEAR class for law-enforcement officer-protection purposes. (Or: confirmation that the device falls within the LIPD 2015 class licence as currently written, in which case no determination is needed.)

**Timeline expectation:** 4–12 weeks from initial meeting to determination. **Do not gate the procurement on this completing — issue the RFQ in parallel** and have the determination in hand before Day-1 of pilot issue.

---

## Step 3 — Procurement (weeks 2–6)

**Action:** Issue the RFQ in [`docs/19-procurement-rfq-template.md`](docs/19-procurement-rfq-template.md) to three domestic contract manufacturers.

**Domestic = `<Australian / New Zealand / UK / US / Canadian>`. Chinese contract manufacturers are not eligible** for supply-chain integrity reasons documented in `docs/16-secrets-and-publishing-policy.md`.

**Expected response time:** 4 weeks from issue.

**Award criteria:** lowest compliant bid, weighted by:

- Price (40%)
- Delivery schedule (20%)
- Manufacturing-quality track record (20%)
- Domestic content (10%)
- Cyber-hygiene posture (10%)

**Pilot quantities to specify in the RFQ:** 200 × NULLWEAR/P, 20 × NULLWEAR/V, 5 × NULLWEAR/S. The RFQ template is pre-populated with these numbers; adjust if your agency size requires different.

**Contract value ballpark:** AUD 50,000–80,000 for the pilot, plus tooling NRE of perhaps AUD 30,000–50,000.

---

## Step 4 — Build (weeks 6–8)

The contract manufacturer does this work. Your role:

| Week | Action |
|---|---|
| 6 | Confirm the CM has access to the agency-mirror of the repo (or the public GitHub repository). |
| 6 | Hand off the agency MCUboot signing public key per `CONTACT.md`. (You retain the private key in your HSM.) |
| 7 | Witness or receive the CM's first-article acceptance test report. |
| 8 | Receive the 200-unit pilot consignment at the agency depot. |

**What "the CM does":**
- Sources the BoM
- Manufactures the PCBs (4-layer FR-4)
- Produces the enclosure tooling (one-off, ~AUD 30k)
- Assembles the units (SMT + final)
- Programs each unit with signed firmware that verifies against your public key
- Runs the per-unit Acceptance Test Procedure (`docs/12-acceptance-test-procedure.md`)
- Ships per-unit reports to your asset system in the schema in `firmware/tools/atp/atp_schema.json`
- Packages and delivers

**What you should look for:**
- Reject rate ≤ 5% on the pilot batch (declines to ≤ 1% at full production)
- Per-unit serial numbers laser-etched and matched to the CM's manufacturing log
- All units charged to ≥ 70% (IATA shipping requirement)
- Tamper-evident packaging intact

---

## Step 5 — Lab acceptance (weeks 8–10)

**Action:** Independently re-test 10% of the pilot units against the same Acceptance Test Procedure at your agency Technical Surveillance Unit's bench.

**Pass criterion:** ≥ 95% of independently-tested units pass ATP.

**Failure handling:** if more than 1 in 10 units fails, return the entire consignment to the CM for root-cause analysis before the field test proceeds.

---

## Step 6 — Field test (weeks 10–14)

**Action:** Run the field-test protocol in [`docs/10-field-testing-protocol.md`](docs/10-field-testing-protocol.md) on 30 units in real operational environments.

**This is the most important phase.** The field test answers four questions, each of which can stop the pilot:

| Question | Test step | Stop-the-pilot if... |
|---|---|---|
| Does NULLWEAR actually annihilate the BLE signal in the field? | Steps 1–10 | PAR < 0.95 in any reasonable wear orientation |
| Will officers actually wear it for 30 days? | Step 11 | Compliance < 95% officer-days |
| Does it break the body camera or Taser? | Steps 14–17 | Any Axon-equipment compatibility failure (especially Signal Sidearm) |
| Does it affect evidentiary integrity? | Steps 18–20 | Any artefact or chain-of-custody discrepancy |

**Decision gate:** end of Step 6 = **Go / No-Go for operational issue**. Programme sponsor (typically the agency's operations executive or commissioner-level decision-maker) signs off.

---

## Step 7 — Issue to officers (weeks 14–16)

If Step 6 was Go:

**Action:** Issue NULLWEAR/P units to a defined cohort of officers. Initially recommend a single command or precinct (~200 officers) for the first 30 days of operational issue, before scaling to the rest of the agency.

**Operational integration:**
- Add NULLWEAR to the equipment-issue checklist alongside the body camera, radio, and weapon.
- Brief officers per the User Manual ([`docs/08-user-manual.md`](docs/08-user-manual.md)) — **5 minutes** per officer at the next shift handover. No formal training course needed.
- Add NULLWEAR-asset-register entries to the agency asset-management system using the schema in [`docs/asset-schema.sql`](docs/asset-schema.sql).
- Stand up the dock infrastructure (each NULLWEAR/P charges via USB-C; agency-issued docks recommended for telemetry but plain USB-C wall chargers are acceptable).

---

## Step 8 — 30-day operational review (weeks 16–20)

**Action:** Daily monitoring + a formal 30-day review report.

**Monitoring metrics** (from the asset system + dock telemetry + officer feedback):

- Compliance: % of officer-days the unit was actually worn.
- Faults: count + type, vs the baseline expected reject rate.
- Officer feedback: solicited at end of week 1, week 2, week 4.
- Axon-equipment reports: any "my body camera did X weird thing" reports — investigate immediately.
- Charging-dock telemetry: did the unit return to dock at end of every shift?

**Decision at end of 30 days:** national rollout? Programme sponsor signs off based on the data, NOT on opinion.

---

## Step 9 — National rollout (weeks 20–52)

If Step 8 confirms the pilot was successful:

**Action:** Exercise the option in the contract for full-fleet quantities. Roll out by command / region / shift over 12 months.

**Pacing recommendation:** 5–10% of total fleet per month. Don't try to issue the entire fleet in a single week — both the CM's production capacity and the agency's logistics can't absorb that.

**Costs at full rollout** (indicative, your agency size will scale these):

| Line | Quantity | Unit | Total (AUD) |
|---|---|---|---|
| NULLWEAR/P | 50,000 | $56 | $2,800,000 |
| NULLWEAR/V | 25,000 | $80 | $2,000,000 |
| NULLWEAR/S | 600 | $400 | $240,000 |
| Dock infrastructure (rough estimate; depends on whether agency uses agency-issued docks vs plain USB-C wall chargers) | per station | varies | ~$200,000 |
| Logistics, integration, training (rough estimate) | once | — | ~$300,000 |
| Year-1 contingency / refurbishment pool (~10% of unit cost) | — | — | ~$500,000 |
| **All-in Year 1 (national, indicative)** | | | **~$6 million** |

This is national-scale; a single-state pilot is roughly 1/10 of these numbers. **Only the unit-price lines flow directly from the engineering specification BoM analysis** (and even those are subject to ±30% per real CM quote). The dock-infrastructure, logistics and contingency lines are operational estimates; the agency procurement office will refine them against actual CM quotes and the agency's own logistics constraints.

---

## Step 10 — Steady-state operations (year 2 onward)

Once the fleet is fielded:

- **Procurement cadence:** ~10% of fleet refresh per year (battery wear at 500 cycles ≈ 2 years per unit; planned 5-year service life with one mid-life refurbishment).
- **Ongoing budget:** ~AUD 600k/year for a national fleet (refurbishment + replacements + new-officer issue).
- **Firmware revision:** approximately one revision per year. Bench-level depot tooling, not officer-level.
- **Reporting:** quarterly aggregate metrics to the National Coordination Committee (or jurisdictional equivalent) — fleet PAR, fault rates, any field-observed attacker adaptation.

---

## Things that can go wrong, and what to do

| If... | Do this |
|---|---|
| ACMA / regulator says "no" or "we need 6 months to think" | Pause the rollout; continue with the pilot as a tech-test inside an RF-shielded environment to keep momentum |
| Field test shows the body camera breaks | STOP. Investigate; do not issue. The Axon-equipment compatibility tests are mandatory pass criteria |
| First batch from the CM fails ATP at high rate | Reject the batch; renegotiate the production-quality clause |
| Officers refuse to wear the unit | Investigate why (comfort, weight, perceived stigma); adjust the form factor or mounting; do NOT mandate against officer concerns |
| A pilot recording is challenged in court | Use the pre-prepared NULLWEAR information sheet; the chain-of-custody evidence from field test Step 18 should resolve |
| Press finds out and writes a story | Refer to the project's open-source nature and the strategic Mitigation Report; emphasise officer-safety motivation; let counsel handle attribution questions |
| Axon Enterprise contacts the agency | Standard vendor relations posture; the project does not require Axon's cooperation; if Axon is willing to ship a default-RPA firmware on their equipment, NULLWEAR can later be retired |
| Another agency wants to buy in | Refer them to this repository; the design is MIT-licensed and freely usable |

---

## Single-page checklist (print this)

Print this page. Stick it on the wall of the project office. Cross items off as they happen.

```
Week  1   [ ] Counsel meeting, decision
Week  1   [ ] Radio regulator pre-application meeting
Week  2   [ ] RFQ issued to ≥ 3 domestic CMs
Week  6   [ ] Contract award
Week  6   [ ] MCUboot signing-key handoff to CM
Week  8   [ ] First-article acceptance from CM
Week  8   [ ] 200 pilot units delivered to agency depot
Week 10   [ ] Independent re-test of 10% of units
Week 10   [ ] Field-test cohort selected
Week 14   [ ] Field-test report; Go/No-Go decision
Week 14   [ ] Regulator determination in hand (final blocker)
Week 16   [ ] Pilot issue to 200 officers
Week 16   [ ] Asset register populated
Week 20   [ ] 30-day operational review
Week 20   [ ] National rollout decision
Week 52   [ ] Year-1 fleet review
```

Indicative budget for a single state pilot: under AUD 1 million.
Indicative budget for national rollout: under AUD 30 million over 18 months.

---

## What this guide deliberately does NOT cover

- Inter-agency coordination: this is your agency's call.
- Public communications strategy: your media and ministerial-relations teams.
- The decision to engage Axon Enterprise about the underlying vulnerability: not required for NULLWEAR to function; not the project's recommendation either way; agency call.
- Modifications to officer training curricula: nothing about wearing NULLWEAR requires training, but if the agency wants to add a brief module to academy classes, that's an internal decision.
- HR / industrial-relations matters: if the agency's union has views on additional issued equipment, deal with them through normal channels.

---

## Cross-references

- Strategic context: companion *Mitigation Report* PDF.
- Why the threat is real: companion *Threat Validation Report* PDF.
- Engineering detail: companion *Engineering Specification* PDF (Rev B).
- Vulnerability disclosure: companion *Disclosure Report* PDF.
- Open-source repository: this directory.

---

## Final word

Three years of disclosure to Victoria Police, since late 2022, did not result in vendor action. This project is the answer to that absence of action — a sovereign capability that any agency can deploy without waiting for the vendor to do anything. The work is done. **The remaining question is whether your agency is willing to act on it.**

If yes, follow this guide.

If no, please ensure the threat brief is on file for the next person who will inevitably ask the same question.

— *Project NULLWEAR maintainer team*
