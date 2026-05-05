# Bidder Checklist — Submit With Bid Response

The Manufacturer shall complete this checklist and submit it as part of the bid response. Each item is a yes/no with optional comment.

---

## A. Bidder identification

| Field | Value |
|---|---|
| Bidder legal name | |
| Trading name (if different) | |
| Country of registration | |
| Final-assembly site address | |
| ABN / equivalent business number | |
| Primary contact for this bid | |
| Primary contact email | |
| Primary contact phone | |
| Years in business | |
| Annual revenue (most recent FY) | |

## B. Mandatory qualifications (yes/no required)

- [ ] Bidder is registered and operates final assembly in `<Australia / NZ / UK / US / Canada>`. ☐ Yes ☐ No
- [ ] Bidder has current ISO 9001:2015 certification (attach copy). ☐ Yes ☐ No
- [ ] Bidder can deliver pilot quantity (200 units) within 12 weeks of contract execution. ☐ Yes ☐ No
- [ ] Bidder can scale to production option quantities (50,000 units) within 12 months. ☐ Yes ☐ No
- [ ] Bidder accepts the MCUboot signed-firmware workflow (Agency holds private key). ☐ Yes ☐ No
- [ ] Bidder's facility passes pre-award cyber-hygiene audit per Essential Eight (or jurisdictional equivalent). ☐ Yes ☐ No
- [ ] Bidder declares no foreign-government affiliation incompatible with Agency procurement policy. ☐ Yes ☐ No
- [ ] Bidder has the in-house or contracted RF test capability to run ATP T4/T5 (anechoic chamber or equivalent quiet RF environment). ☐ Yes ☐ No
- [ ] Bidder has read the *Engineering Specification* PDF (Rev B) in full. ☐ Yes ☐ No
- [ ] Bidder agrees to MIT licence terms on the reference design and to assignment of bidder-developed CAD to the Agency at acceptance. ☐ Yes ☐ No

**A "no" response to any of the above will disqualify the bid.** Add a comment explaining the situation if a partial match is the case.

## C. Past-performance references

Provide three (3) reference projects of comparable scope (electronics manufacture for government / safety-critical / law-enforcement / defence). For each:

| # | Project | Customer (sanitised if required) | Volume | Year | Reference contact |
|---|---|---|---|---|---|
| 1 | | | | | |
| 2 | | | | | |
| 3 | | | | | |

## D. Production capability

| Question | Answer |
|---|---|
| Number of SMT lines | |
| Component pick-and-place precision (e.g. ±25 µm) | |
| AOI / AXI capability | |
| In-house or third-party EMC testing | |
| In-house or third-party environmental testing (temperature chamber, drop fixture, IP67 immersion) | |
| Maximum throughput (units/day) at single-line operation | |
| Lead time from PO to first-article completion | |

## E. Substitutions and deviations

If the Bidder proposes any substitution from the reference BoM (`feasibility-pack/03-bill-of-materials.csv`), list each proposed substitute below with rationale:

| Reference part | Proposed substitute | Rationale | Cost impact |
|---|---|---|---|
| | | | |

If the Bidder proposes any mechanical, firmware, or process deviation from the reference design, list each below:

| Item | Reference value | Proposed value | Rationale |
|---|---|---|---|
| | | | |

## F. Pricing summary

(Use the format in RFQ §7. Include below for cross-reference.)

| Line | Variant | Pilot qty | Pilot unit price | Production qty | Production unit price |
|---|---|---|---|---|---|
| 1 | NULLWEAR/P | 200 | | 50,000 | |
| 2 | NULLWEAR/V | 20 | | 25,000 | |
| 3 | NULLWEAR/S | 5 | | 600 | |
| 4 | Tooling NRE | | | | |
| 5 | Test fixtures NRE | | | | |
| 6 | Per-unit firmware programming + serialisation | | | | |
| 7 | Per-unit ATP run | | | | |
| 8 | Packaging | | | | |
| 9 | Refurbishment | | | | |

Pricing assumptions:
- Currency:
- Lead time validity (price hold period):
- Payment terms:
- Volume discount tiers (if any):

## G. Cyber-hygiene attestation

The Bidder declares (yes/no):

- [ ] No portion of the firmware-flashing or test infrastructure resides on a network with any party outside the Bidder.
- [ ] All employees with access to the Agency's MCUboot signing public key and the firmware-flashing line have been background-vetted to the Bidder's standard.
- [ ] The Bidder has a documented incident-response procedure in case of suspected supply-chain compromise.
- [ ] The Bidder maintains audit logs of every signed firmware image flashed, retained for the contract period.

## H. Sign-off

Signed by an officer of the Bidder authorised to bind the Bidder contractually:

```
Name:            ____________________________________________
Title:           ____________________________________________
Date:            ____________________________________________
Signature:       ____________________________________________
```

---

Submit this completed checklist alongside the priced response per the RFQ format to the Agency procurement officer named on the RFQ cover sheet.
