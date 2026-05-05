# Security Considerations

The threat model NULLWEAR addresses, and the threats it does NOT address.

---

## What NULLWEAR defends against

NULLWEAR is designed to defeat one specific class of attack:

> **Passive third-party identification, location, and tracking of law enforcement officers via the BLE-OUI signature broadcast by their issued Axon Enterprise equipment, when the officer is within the radio coverage of an unauthorised BLE scanner.**

It addresses the threat at the radio-frequency layer, by destroying target packets in the air before they cross the radio perimeter of the protection bubble.

---

## What NULLWEAR does NOT defend against

| Threat | Why NULLWEAR does not address it | What does |
|---|---|---|
| Active attack on Axon equipment (firmware exploit, BLE pairing attack, etc.) | NULLWEAR does not interact with Axon equipment | Axon firmware hardening; vendor responsibility |
| Compromise of agency police networks | Out of scope for radio-layer mitigation | Network security; IRAP-assessed systems |
| Malware on officer phones | Out of scope | Mobile device management |
| Visual identification of officers | NULLWEAR does not affect visible identifiers (uniforms, vehicles, faces) | Officer safety procedures |
| ALPR-based vehicle tracking by attackers | NULLWEAR does not affect licence plates | Vehicle registration management; covert plates for special operations |
| Crowd-sourced reporting of officer presence (e.g. real-time community apps) | NULLWEAR is not a counter to human intelligence collection | Community engagement; legislative review |
| RFID-based equipment tracking (if Axon adds RFID) | NULLWEAR is BLE-specific | Would require an analogous RFID-aware countermeasure |
| Cellular triangulation of officer phones | NULLWEAR does not affect cellular | Officer phone hardening |
| Tracking via satellite imagery | Out of scope | n/a |
| Insider threats with access to the agency asset register linking officers to equipment serials | NULLWEAR does not prevent privileged-access misuse | Access control; audit |

It is critical that the deployment of NULLWEAR not lead to a false sense of security. Officers and agencies should understand precisely which threat NULLWEAR mitigates and which it does not.

---

## Attack surfaces of NULLWEAR itself

### Physical

- An attacker who recovers a NULLWEAR unit gets:
  - Hardware identical to the open-source design.
  - Firmware (signed; attacker cannot trivially modify in place).
  - A serial number that links to an officer (via the agency asset register, which is access-controlled).
- An attacker who modifies a NULLWEAR unit and reissues it could substitute compromised firmware that does not actually annihilate.
- Mitigation: the agency asset register tracks every issued serial; field-test sampling catches anomalies; signing keys are held by the agency.

### Supply chain

- A compromised contract manufacturer could substitute components, alter firmware, or introduce backdoors.
- This is why **the project specification mandates a domestic manufacturer** (Australian, NZ, UK, US, or Canadian — explicitly **not Chinese**). The supply chain is an integral part of the mitigation.
- Even with a domestic CM, supply-chain integrity assurance should include: incoming-component verification, in-line firmware checksum validation, signed build artefacts, and tamper-evident packaging.

### Firmware

- The firmware is open source and signed. Anyone can audit the source. Only signed images will be accepted by the bootloader.
- Update path is over USB-CDC only (no OTA), eliminating remote-attack surface.
- Long-term, the firmware should be reviewed by an independent assessor (e.g. Australian IRAP assessor, ASD-certified evaluator) for production deployment.

### USB-CDC interface

- The diagnostic interface exposes statistics and a small command set.
- It does not expose user-input fields beyond a constrained text command parser.
- It does not expose any operational data.
- Risk: an attacker with physical access could brick the unit by issuing `dfu` and aborting the update mid-stream.
- Mitigation: tactically minor (one unit lost, replaceable from depot); operationally negligible.

### Radio interface

- NULLWEAR transmits only in response to a received `00:25:DF`-prefixed packet, in microsecond bursts.
- It does not accept incoming connections.
- It does not advertise itself.
- An attacker could deliberately broadcast forged `00:25:DF` packets to provoke NULLWEAR transmissions, draining the battery faster than normal. This is a low-grade DoS against NULLWEAR. Mitigation: thermal/rate limiting at the firmware level (TX duty cycle cap).

### Asset register

- The asset register linking officer badge numbers to NULLWEAR serial numbers is **operationally sensitive**.
- An attacker with access to the asset register could combine it with field RF observations to identify which specific officer is at which specific location.
- Treat the asset register as PROTECTED data per agency information-classification policy.
- Implement role-based access; audit access; isolate the dock-management VLAN.

---

## Cryptographic properties

NULLWEAR uses cryptography only for:

- **Firmware signing.** MCUboot Ed25519 signature. Verification at boot.
- **No payload encryption.** The radio operations are link-layer; there is no application-layer payload to encrypt.

The firmware update key must be stored in a hardware security module (HSM) or equivalent at the agency. Key compromise would allow an attacker who modifies the firmware to make the device appear functional while not actually annihilating. This is the highest-impact attack on NULLWEAR's integrity. Treat the signing key as analogous in sensitivity to a code-signing certificate for OS kernel modules.

---

## Privacy considerations

NULLWEAR is privacy-protective by design — it eliminates a class of passive surveillance against police officers. But a NULLWEAR unit, by its function, transmits only when it detects a `00:25:DF` packet nearby. An adversary equipped to observe NULLWEAR's TX bursts (an SDR with packet-CRC-failure logging) could in principle infer the presence of an Axon device at the location of the NULLWEAR unit.

In other words: NULLWEAR removes the manufacturer-tied identifier but reveals "an Axon-OUI packet was being transmitted nearby" via its own emission. The leak is much weaker than the original signal — it carries no MAC, no serial, no device identity, just "something Axon-shaped happened here". But it is non-zero.

This is a fundamental property of reactive interference. The only stronger countermeasure would be silent shielding (e.g. Faraday holsters), which has the operational drawbacks discussed in the strategic Mitigation Report §3.

For the threat profile NULLWEAR is designed against (commodity, passive, third-party scanners running off-the-shelf BLE stacks), the residual leak is irrelevant — those scanners cannot detect CRC-failed packets at all, and even if they could, the information content is much lower.

For a sophisticated nation-state adversary with custom SDR-based receivers, the leak is observable, but they would also have other surveillance options against which NULLWEAR is not the primary mitigation.

---

## Compliance

NULLWEAR's design and operation must comply with:

- **Radiocommunications Act 1992 (Cth)** — operation of the device under appropriate class licence.
- **Telecommunications (Interception and Access) Act 1979 (Cth)** — NULLWEAR does not intercept communications and therefore should not engage Part 5-1A obligations, but legal review should confirm.
- **Privacy Act 1988 (Cth)** — the asset register is "personal information" under the Act when it links serial numbers to officer identities.
- **Surveillance Devices Act 2004 (Cth) and equivalent state Acts** — NULLWEAR does not record, observe or listen to communications; it should not engage as a "surveillance device" but legal review should confirm.
- **Customs and prohibited imports law** — components and finished units may have export-control implications under the Defence and Strategic Goods List; verify before international transfer.

---

## Disclosure of vulnerabilities in NULLWEAR

If you find a vulnerability in NULLWEAR (firmware bug, supply-chain risk, regulatory gap):

1. Open a private GitHub issue, or contact the author directly.
2. Do not publicise the vulnerability until the agency has had a reasonable opportunity to assess and mitigate.
3. Coordinated disclosure follows standard responsible-disclosure norms.

---

## Defence in depth

NULLWEAR is one layer of a five-layer defence (see strategic Mitigation Report §7):

1. **NULLWEAR (this layer)** — radio annihilation per officer.
2. Covert-vehicle decoy emission for plainclothes operations.
3. AFP cloud-side poisoning of identified attacker backends.
4. Telco-side fingerprinting and physical recovery of attacker scanner hardware.
5. Vendor pressure on Axon for default-RPA firmware.

NULLWEAR alone is the immediate, sovereign, deployable answer. The other four layers harden the defence against attacker adaptation and address the underlying problem at the protocol level over time.
