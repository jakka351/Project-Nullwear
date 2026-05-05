# Hardware Feasibility Pack

A self-contained bundle of every artefact a contract manufacturer needs to **scope, quote, build, and test** Project NULLWEAR units. Designed to be sent as a single ZIP to candidate CMs alongside the RFQ.

If you are a procurement officer: this directory is what you attach to your RFQ.

If you are a contract manufacturer reviewing: every document referenced in the agency's RFQ is either in this directory or linked from this README. You should not need to chase the agency for any reference material.

---

## What's in this pack

```
feasibility-pack/
├── README.md                        ← this file
├── 01-scope-of-work.md              ← the SOW template (fill in agency blanks)
├── 02-rfq-quick-reference.md        ← RFQ summary; full template at docs/19
├── 03-bill-of-materials.csv         ← every component, MPN, supplier, indicative cost
├── 04-acceptance-test-summary.md    ← summary of ATP; full procedure at docs/12
├── 05-firmware-handoff.md           ← what the CM needs from agency to flash signed firmware
├── 06-mechanical-summary.md         ← enclosure, ingress, drop, materials
├── 07-regulatory-summary.md         ← per-jurisdiction radio compliance pathway
└── 08-bidder-checklist.md           ← what the CM submits back with their bid
```

Reference documents NOT in this pack but linked from above:

- Full Engineering Specification PDF (companion document, 24 pages)
- Full Mitigation Report PDF (strategic context, 11 pages)
- Threat Validation Report PDF (empirical evidence, 10 pages)
- Open-source repository (this entire `Project-Nullwear/` tree)

---

## How a contract manufacturer should use this pack

1. **Read 01-scope-of-work.md** first — that defines what you are quoting on.
2. Read **04-acceptance-test-summary.md** to understand what each unit must pass before shipment.
3. Cross-check **03-bill-of-materials.csv** against your component-supply database.
4. Review **05-firmware-handoff.md** to understand the signing-key workflow you must support.
5. Check **07-regulatory-summary.md** for the compliance regime in the agency's jurisdiction.
6. Submit the **08-bidder-checklist.md** completed, alongside your priced response per the RFQ format.

If any item is unclear, ask the agency procurement officer (NOT a public GitHub issue) — see the agency's contact in the RFQ cover sheet.

---

## How a procurement officer should use this pack

1. **Customise 01-scope-of-work.md** with your agency's specifics in the `<bracketed>` placeholders.
2. **Set the quantities and delivery schedule** in the SOW.
3. **Update 07-regulatory-summary.md** with any agency-specific guidance from your radio regulator.
4. **Bundle everything** in this directory, plus the four PDFs listed above, into a single ZIP.
5. **Send to your candidate CMs** alongside your standard RFQ cover sheet.

That's the entire procurement-side workflow. Bid responses come back in 4 weeks; award decision in week 6; first-article in week 12; pilot delivery in week 14.

---

## License

Everything in this pack is MIT-licensed (see top-level `LICENSE`). A CM is free to reuse the design without IP encumbrance.
