# Pilot Deployment Plan

90-day rollout from authorisation to operational issue.

---

## Purpose

Take Project NULLWEAR from a paper specification to officers in service in 90 days, with a defensible audit trail at every step.

This plan is written for an Australian context. The same structure applies in any Five Eyes jurisdiction with substitution of equivalent regulators and agencies.

---

## Stakeholders

| Role | Responsible party |
|---|---|
| Programme sponsor | Department of Home Affairs — Cyber and Critical Infrastructure Coordination Centre (CICC) |
| Operational lead | Participating State Police service (initial pilot agency) |
| Technical authority | ASD ACSC + AFP Cyber Command |
| Regulatory liaison | ACMA (radio-spectrum class licence variation) |
| Procurement lead | Agency procurement office |
| Contract manufacturer | TBD — domestic Australian electronics manufacturer |
| Test authority | Agency Technical Surveillance Unit |
| Legal review | Agency General Counsel + Australian Government Solicitor |

---

## Timeline

```
        ─── Week 0─────2─────4─────6─────8────10────12─── Week 13
WP1 Stand-up        ████████░░
WP2 Pilot build              ████████░░░░
WP3 Lab acceptance                   ░░░░████░░░░
WP4 Field testing                              ░░░░████
WP5 Issue                                              ░░░░████
WP6 Operational review                                       ░░░░░
```

---

## Work Package 1 — Stand-up (weeks 0–2)

### Deliverables

- **WP1-D1:** Joint Capability Stand-up meeting, chaired by CICC, attended by all stakeholders.
- **WP1-D2:** Signed MoU between CICC and the participating state police service.
- **WP1-D3:** Procurement contract awarded to selected domestic CM. (Procurement spec packaged from this repository: `README.md`, `docs/05-hardware-spec.md`, `docs/12-acceptance-test-procedure.md`, `docs/15-legal-and-regulatory.md`.)
- **WP1-D4:** ACMA pre-application brief — formal request for class-licence variation under LIPD 2015 to authorise NULLWEAR's emission profile.
- **WP1-D5:** Legal-review brief from agency General Counsel covering operational liability framework, evidentiary impacts (e.g. does an officer wearing NULLWEAR constitute interference with their own body camera in a way that affects evidence?), and inter-jurisdictional implications.

### Acceptance criteria

- All deliverables signed off by week 2.

---

## Work Package 2 — Pilot build (weeks 2–6)

### Deliverables

- **WP2-D1:** CM produces 200 NULLWEAR/P units (pilot quantity).
- **WP2-D2:** CM produces 20 NULLWEAR/V units.
- **WP2-D3:** CM produces 5 NULLWEAR/S units.
- **WP2-D4:** Test fixtures and reference receiver hardware delivered to the agency Technical Surveillance Unit.

### Acceptance criteria

- All units pass ATP (`docs/12-acceptance-test-procedure.md`) at the CM's test bench before shipment.
- Reject rate < 5% during pilot manufacture (higher rate during initial production run is acceptable but should trigger root-cause analysis).

---

## Work Package 3 — Lab acceptance (weeks 6–8)

### Deliverables

- **WP3-D1:** Independent lab re-test of 10% of pilot units against ATP.
- **WP3-D2:** Lab report signed off by ASD ACSC engineering.
- **WP3-D3:** ACMA technical evaluation if requested as part of class-licence determination.

### Acceptance criteria

- ≥ 95% of independently-tested units pass ATP.
- No systematic failure mode identified.
- ACMA technical findings, if any, addressed in firmware revision.

---

## Work Package 4 — Field testing (weeks 8–11)

### Deliverables

- **WP4-D1:** Field test conducted per `docs/10-field-testing-protocol.md` on 30 units in real operational environments by the Technical Surveillance Unit.
- **WP4-D2:** Red-team verification: stand-up of the recovered attacker stack inside a controlled environment, deployed against pilot officers, demonstrating zero scanner-side detection.
- **WP4-D3:** Red-team report.
- **WP4-D4:** Officer feedback collection (wearability, daily-use friction).

### Acceptance criteria

- ≥ 95% of field-tested units achieve PAR ≥ 0.99 at 5 m.
- Red-team confirms attacker dashboard reports zero detections of pilot officers in the test area.
- No critical wearability complaints from pilot officers.

### Decision gate

End of WP4: **Go / No-Go decision** by programme sponsor on whether to proceed to operational issue.

---

## Work Package 5 — Operational issue (weeks 11–13)

### Deliverables

- **WP5-D1:** First 200 units issued to a defined cohort of pilot officers (typically a single command or precinct).
- **WP5-D2:** Quick-start cards distributed.
- **WP5-D3:** Asset-management system populated with pilot serial numbers.
- **WP5-D4:** Daily monitoring dashboard live (dock telemetry → asset system).

### Acceptance criteria

- 100% of pilot cohort officers issued and briefed.
- Asset system reconciles with physical inventory.

---

## Work Package 6 — Operational review (weeks 13+, ongoing)

### Deliverables

- **WP6-D1:** Daily operational reports for first 30 days (any fault, any incident, any officer feedback).
- **WP6-D2:** 30-day operational review report to programme sponsor.
- **WP6-D3:** Decision on national rollout.

### Acceptance criteria

- < 2% in-service fault rate during first 30 days.
- No officer-safety incident attributable to NULLWEAR malfunction.
- No reported interference with Axon equipment normal operation.

---

## Concurrent activities (continuous)

- **Layer 3 (Cloud-side poisoning):** AFP Cyber Command stands up and tests the cloud-poisoning capability against the recovered attacker backend or red-team backend. (Independent of WP1–6.)
- **Layer 4 (Telco fingerprinting):** ASIO + AFP coordinate with Telstra, Optus, Vodafone, TPG to pilot the SIM fingerprint detection rules.
- **Layer 5 (Vendor track):** Joint procurement letter to Axon Enterprise drafted and signed by participating Five Eyes agencies, demanding default-RPA firmware as a condition of next contract renewal.

---

## Risks

| Risk | Mitigation |
|---|---|
| ACMA class-licence variation takes longer than 4 weeks | Begin engagement in WP1; have legal escalation path to Minister if required. |
| CM cannot meet quality at pilot quantities | Selecting a CM with prior Defence-grade experience; full ATP at CM bench before shipment. |
| Field testing reveals lower PAR than lab predicted | Scheduled WP4 before issue gate; firmware revision cycle built into timeline. |
| Officer pushback on additional equipment | User Manual minimises friction; carry weight is ~28 g; daily interaction time is 30 s. |
| Axon ships RPA firmware before pilot completes | Best possible outcome — NULLWEAR becomes redundant against new firmware. Continue pilot for legacy fielded equipment that has not been updated. |
| Recovered attacker stack is variant of described system, with different OUI or different protocol | Firmware is parameterised on OUI; 1-line change to target a different OUI; no architectural change. |

---

## Governance

Weekly steering committee for WP1–6. Members:

- CICC programme sponsor (chair)
- State police operational lead
- ASD ACSC technical lead
- AFP cyber lead
- Procurement lead

Reports to NSC of Cabinet at end of WP3 (lab acceptance), WP4 (field test), and WP6 (operational review).

---

## Cost envelope (indicative)

| Line | AUD |
|---|---|
| WP1: Stand-up + legal | $250,000 |
| WP2: Pilot build (225 units across 3 variants) | $80,000 |
| WP3: Lab acceptance | $60,000 |
| WP4: Field testing + red-team | $200,000 |
| WP5: Pilot issue + dock infrastructure | $80,000 |
| WP6: First 30 days operational support | $50,000 |
| **Total — pilot** | **$720,000** |

National scale-out cost estimated separately in the strategic Mitigation Report (~AUD 7M for 50 000 units + supporting infrastructure).

---

## Success measures

A successful pilot is one that produces:

1. A demonstrable, measured PAR ≥ 0.99 in real operational conditions.
2. A ratified ACMA class-licence determination authorising the device class for nationwide deployment.
3. An officer cohort wearing the device with no safety incidents and minimal friction.
4. A documented red-team finding that the attacker's dashboard is rendered useless against the protected cohort.
5. A clear go/no-go for national rollout, made on evidence rather than projection.
