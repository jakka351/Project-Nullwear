# Documentation Index

All documentation for Project NULLWEAR. Numbered for sequential reading; you can also dip in by topic.

| # | Document | Audience |
|---|---|---|
| 00 | This index | Everyone |
| 01 | [Overview](01-overview.md) | Everyone — read first |
| 02 | [Architecture](02-architecture.md) | Engineers, reviewers |
| 03 | [Bluetooth LE Primer](03-bluetooth-le-primer.md) | Engineers, security reviewers, technically-curious officers |
| 04 | [BLE CRC Corruption — How NULLWEAR Annihilates a Packet](04-ble-crc-corruption.md) | Engineers, security reviewers |
| 05 | [Hardware Specification (short form)](05-hardware-spec.md) | Procurement, contract manufacturers, engineers |
| 06 | [Firmware Architecture](06-firmware-architecture.md) | Firmware engineers, reviewers |
| 07 | [Build Instructions](07-build-instructions.md) | Firmware engineers, contract manufacturers |
| 08 | [User Manual (for the officer)](08-user-manual.md) | Sworn officers carrying the device |
| 09 | [Operations Manual (for the depot)](09-operations-manual.md) | Quartermasters, depot technicians, asset managers |
| 10 | [Field Testing Protocol](10-field-testing-protocol.md) | Field-test technicians |
| 11 | [Troubleshooting](11-troubleshooting.md) | Depot technicians |
| 12 | [Acceptance Test Procedure](12-acceptance-test-procedure.md) | Contract manufacturers, depot intake QA |
| 13 | [Pilot Deployment Plan](13-pilot-deployment-plan.md) | Programme managers, agency procurement, sponsors |
| 14 | [Security Considerations](14-security-considerations.md) | Security reviewers, agency CSO |
| 15 | [Legal and Regulatory](15-legal-and-regulatory.md) | Agency General Counsel, regulatory liaisons |
| 16 | [Secrets, Keys, and Publishing Policy](16-secrets-and-publishing-policy.md) | Project maintainer, agency security officer, anyone with commit access |
| — | [References](REFERENCES.md) | Everyone |

## Companion documents (PDF, not in this repo)

- **Mitigation Report** — *Axon Bluetooth Vulnerability — A Mitigation for Law Enforcement.* The strategic case.
- **Engineering Specification** — *Project NULLWEAR — Engineering Specification.* The detailed engineering reference, including mechanical drawings and full BoM.
- **Disclosure Report** — *Weaponised Bluetooth Tracking of Law Enforcement Personnel.* The threat NULLWEAR mitigates.

These three PDFs are issued under the same confidentiality regime as this repository and accompany it through the responsible-disclosure channels.

## Suggested reading orders

### "I am an officer who has been issued the device"
1. [User Manual](08-user-manual.md) — that's all you need.

### "I am a watch commander rolling this out to my team"
1. [Overview](01-overview.md)
2. [User Manual](08-user-manual.md)
3. [Pilot Deployment Plan](13-pilot-deployment-plan.md) — for context on how it gets to your team

### "I am a depot quartermaster"
1. [Operations Manual](09-operations-manual.md)
2. [Acceptance Test Procedure](12-acceptance-test-procedure.md)
3. [Troubleshooting](11-troubleshooting.md)
4. [Field Testing Protocol](10-field-testing-protocol.md)

### "I am the contract manufacturer"
1. [Hardware Specification](05-hardware-spec.md)
2. [Build Instructions](07-build-instructions.md)
3. [Acceptance Test Procedure](12-acceptance-test-procedure.md)
4. The Engineering Specification PDF

### "I am an engineer reviewing the technique"
1. [Overview](01-overview.md)
2. [Architecture](02-architecture.md)
3. [Bluetooth LE Primer](03-bluetooth-le-primer.md)
4. [BLE CRC Corruption](04-ble-crc-corruption.md)
5. [Firmware Architecture](06-firmware-architecture.md)
6. The actual source code under `firmware/`
7. [References](REFERENCES.md)

### "I am Agency General Counsel"
1. [Overview](01-overview.md)
2. [Security Considerations](14-security-considerations.md)
3. [Legal and Regulatory](15-legal-and-regulatory.md)
4. The Mitigation Report PDF

### "I am a security researcher writing an independent assessment"
1. [Overview](01-overview.md)
2. [Architecture](02-architecture.md) → [BLE CRC Corruption](04-ble-crc-corruption.md) → [Firmware Architecture](06-firmware-architecture.md)
3. The actual source code under `firmware/`
4. [Security Considerations](14-security-considerations.md)
5. [Field Testing Protocol](10-field-testing-protocol.md) — to validate empirically

### "I am an MP, an ASIO/ACSC analyst, or NSC of Cabinet"
1. The Mitigation Report PDF (companion document) — strategic context
2. [Overview](01-overview.md)
3. [Pilot Deployment Plan](13-pilot-deployment-plan.md)
