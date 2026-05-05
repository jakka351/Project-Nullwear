# RFQ Quick Reference

The full RFQ template is at [`docs/19-procurement-rfq-template.md`](../docs/19-procurement-rfq-template.md). This file is a one-page summary so a CM can quickly understand what is being asked.

## Bottom-line ask

| Item | Pilot | Production option |
|---|---|---|
| NULLWEAR/P units | 200 | 50,000 over 12 months |
| NULLWEAR/V units | 20 | 25,000 |
| NULLWEAR/S units | 5 | 600 |

## Bottom-line schedule

- **Bid deadline:** 4 weeks from RFQ issue
- **First-article:** Contract + 6 weeks
- **Pilot delivery:** Contract + 8 weeks
- **Production option exercise:** Contract + 14 weeks (if pilot accepted)

## Bottom-line price expectation

Indicative reference: **~AUD 56 per NULLWEAR/P unit at 50k volume** (components + assembly + test + packaging + margin). Bidders are not required to match this — quote your own pricing.

## Mandatory disqualifiers

A bid will not be considered if any of the following are true:

- Bidder proposes Chinese final assembly
- Bidder cannot demonstrate ISO 9001:2015 certification
- Bidder cannot demonstrate prior safety-critical or government-grade electronics experience
- Bidder declares foreign-government affiliation incompatible with the Agency's procurement policy
- Bidder's quoted lead-time for pilot delivery exceeds 12 weeks from contract execution
- Bidder declines to support the MCUboot signed-firmware workflow (Agency holds the private key)

## Required deliverables in the bid response

1. Cover letter
2. Filled pricing table (per the RFQ §7 format)
3. Qualification evidence (per RFQ §8)
4. Technical proposal — including any deviations from the reference design
5. Project plan (Gantt or equivalent)
6. Three (3) past-performance references

## Where to look for the reference design

| Question | Where the answer is |
|---|---|
| What is the device? | `docs/01-overview.md` + `docs/02-architecture.md` |
| What's in the BoM? | `feasibility-pack/03-bill-of-materials.csv` (and `docs/05-hardware-spec.md`) |
| What's the firmware? | `firmware/nullwear-p/` (source) + `docs/06-firmware-architecture.md` (description) |
| What does the unit have to pass before shipment? | `docs/12-acceptance-test-procedure.md` (full) + `feasibility-pack/04-acceptance-test-summary.md` (summary) |
| What's the mechanical spec? | `docs/05-hardware-spec.md` + Engineering Specification PDF |
| What's the regulatory pathway? | `docs/15-legal-and-regulatory.md` + `feasibility-pack/07-regulatory-summary.md` |
| What's the IP situation? | MIT-licensed; CM-developed CAD becomes Agency property at acceptance |
| Who do I contact with technical questions? | Agency Technical Liaison, named on RFQ cover sheet |
