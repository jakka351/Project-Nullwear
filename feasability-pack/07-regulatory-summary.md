# Regulatory Summary

The full regulatory analysis is in [`docs/15-legal-and-regulatory.md`](../docs/15-legal-and-regulatory.md). This summary tells the bidder what regulatory regime applies to manufacturing and operation.

## Australia (default jurisdiction)

| Concern | Regime | Manufacturer's responsibility |
|---|---|---|
| Radio compliance | *Radiocommunications Act 1992* (Cth) + LIPD 2015 class licence | Build to the technical parameters of the Agency-supplied determination (TX power, channels, duty cycle). |
| Operation authorisation | Ministerial determination under s.27 of the Act | Held by the Agency, not the Manufacturer. |
| EMC compliance | AS/NZS CISPR 32 Class B | Test and document at the Manufacturer's bench. |
| Electrical safety | AS/NZS 60950-1 (low-voltage device) | Test and document. |
| Lithium-battery transport | IATA UN3481 PI967 | Comply with shipping documentation. |
| RoHS / REACH | Standard | Disclose component compliance. |
| RCM marking | ACMA RCM scheme | Apply RCM mark to each unit. |

## Other Five Eyes jurisdictions

If the Agency is outside Australia, substitute the local equivalent:

| Country | Radio regulator | Class-licence equivalent | EMC standard |
|---|---|---|---|
| New Zealand | RSM (Radio Spectrum Management) | General User Radio Licence | AS/NZS CISPR 32 |
| United Kingdom | Ofcom | IR 2030 — Licence-Exempt Short Range Devices | EN 55032 |
| United States | FCC | Part 15.247 (2.4 GHz ISM) | 47 CFR Part 15 |
| Canada | ISED | RSS-247 | ICES-003 |

The Manufacturer is responsible for ensuring the manufactured units comply with the chosen jurisdiction's regime. The Agency provides the determination / authorisation under which the units OPERATE; the Manufacturer ensures the units BUILT meet the technical parameters of that determination.

## Australian Industry Capability

For Australian-government procurements, AIC content (locally-sourced labour and materials as a percentage of contract value) is a standard reporting requirement. Bidders should be prepared to declare AIC content in their bid.

## Conflict-mineral disclosure

Where applicable to the Agency's jurisdiction (e.g. Defence procurements may require US Conflict Minerals Rule reporting). The Manufacturer is responsible for ensuring its component supply chain is compliant.

## What the Agency needs to do BEFORE production starts

The Agency has 4 weeks to obtain the radio-spectrum determination from the regulator. The Manufacturer can begin tooling and BoM procurement IN PARALLEL with this process, but the first units cannot be issued to officers until the determination is in hand.

If the Agency's regulator requires modification to the technical parameters (e.g. lower TX power, different frequency hopping schedule), the Manufacturer will need to update the firmware accordingly. The reference firmware exposes TX power as a build-time parameter; channel hopping parameters are similarly configurable. Such modifications are NRE-line items.

## What the Manufacturer should ask the Agency at contract execution

1. Has the regulator determination been granted? If not, when is it expected?
2. Are there any agency-jurisdiction-specific marking requirements (RCM, FCC ID, CE)?
3. Should the Manufacturer apply the agency's logo to the enclosure?
4. Does the Agency require any additional certifications for export of refurbishment units?

## Reference

- AS/NZS CISPR 32: <https://www.standards.org.au/>
- LIPD 2015 (AU): <https://www.legislation.gov.au/F2015L01438>
- Bluetooth SIG 16-bit UUID registry (cite for the FE6B Axon UUID): <https://www.bluetooth.com/specifications/assigned-numbers/>
- IATA Lithium Battery Guidance: <https://www.iata.org/lithiumbatteries>
