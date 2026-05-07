# Overview

Project NULLWEAR is a sovereign open-source counter-capability for one specific class of radio-frequency surveillance against law-enforcement officers — the passive third-party detection and tracking of officers via the Bluetooth Low Energy advertising packets that their issued Axon Enterprise equipment broadcasts in the open.

## In one paragraph

Every Axon body camera, smart holster, Taser, and in-car video unit broadcasts a Bluetooth Low Energy (BLE) advertising packet many times per second. Each of those packets contains the device's MAC address, the first three bytes of which are the IEEE-registered Organisationally Unique Identifier (OUI) `00:25:DF`, assigned to Taser International / Axon Enterprise. Anyone within Bluetooth range can passively listen for that signature and know that an officer wearing Axon equipment is nearby. NULLWEAR is a small, officer-issued radio device that detects each of those broadcasts in flight and corrupts them at the over-the-air protocol layer — destroying the packet's CRC trailer before any third-party scanner can validate the packet. To the third-party scanner, the officer becomes invisible.

## What this project provides

- **Three hardware variants**: NULLWEAR/P (personal-issue, body-worn), NULLWEAR/V (vehicle-mounted), NULLWEAR/S (station-mounted).
- **Reference firmware** for the Nordic nRF5340 system-on-chip, comprising network-core code that owns the radio peripheral and application-core code that handles battery, LED, USB and management.
- **Mechanical and PCB references** for production manufacture.
- **Verification tools**: a Python reference receiver and an ESP32-based test source emulator.
- **Operating documentation**: user manual, operations manual, field testing protocol, acceptance test procedure.
- **Programme documentation**: pilot deployment plan, security considerations, legal/regulatory analysis.
- **A companion strategic Mitigation Report and Engineering Specification PDF** (issued separately under the same confidentiality regime).

All of this is open source under the copyleft GPL licence. The intent is that any law-enforcement or national-security agency in any friendly jurisdiction can take this work, build the devices via a domestic contract manufacturer (not Chinese), and protect their officers, without legal encumbrance.

## What it is not

- It is not a fix for the underlying vulnerability. The fix is for Axon Enterprise to ship default-Resolvable-Private-Address firmware on all BLE-capable products. NULLWEAR is the bridge that protects officers in the years it will take Axon to do that.
- It is not a system that requires Axon's cooperation, agency network changes, or Bluetooth specification changes. It works on the existing fielded Axon equipment, today.
- It is not a jammer in the regulatory sense. It transmits only in microsecond bursts triggered by a specific OUI-matched BLE packet, and only on the same channel that packet is on.
- It is not a surveillance device, listening device, or interception capability. It does not record, decode, or relay any communication.
- It is not a substitute for any other officer-safety control — it is one layer of a defence-in-depth posture (see the strategic Mitigation Report §7).

## How it differs from the obvious alternatives

- **Decoys** add noise to an attacker's signal. NULLWEAR removes the signal from the attacker's input. Decoys can be filtered with sufficient observation; annihilation cannot.
- **Faraday-shielded equipment carriers** disable Axon equipment as well as protect it. NULLWEAR has zero impact on Axon equipment's own operation.
- **Waiting for vendor firmware** takes years; protection is needed now.
- **Broad-band jammers** are illegal, indiscriminate, and would brick the officer's own equipment along with the attacker's scanners.

## How it works (technical, one paragraph)

The Nordic nRF5340 system-on-chip has a dedicated network core with direct access to a 2.4 GHz radio peripheral. NULLWEAR's network-core firmware programs the radio to scan the three BLE primary advertising channels (37, 38, 39). On each received packet, it uses the radio's bit-counter event to fire an interrupt after the first 64 bits of the PDU (16 bits of header + 48 bits of MAC) have been received. It compares the OUI bytes against the target value `00:25:DF`. On a match, it uses Nordic's PPI/DPPI hardware-event chaining to switch the radio from receive to transmit within ~40 µs — without CPU mediation — and emit a colliding RF burst on the same channel during the original packet's CRC trailer. Every BLE receiver in radio range fails the CRC check and silently discards the packet. The Axon device's broadcast is destroyed in flight.

## Where to start, by reader type

| You are | Read first |
|---|---|
| An officer who has been issued the device | [`08-user-manual.md`](08-user-manual.md) |
| A depot quartermaster receiving units into stock | [`09-operations-manual.md`](09-operations-manual.md) |
| A field-test technician validating units before issue | [`10-field-testing-protocol.md`](10-field-testing-protocol.md) |
| An agency procurement officer | [`13-pilot-deployment-plan.md`](13-pilot-deployment-plan.md) + this overview |
| A contract manufacturer evaluating a build | [`07-build-instructions.md`](07-build-instructions.md), [`05-hardware-spec.md`](05-hardware-spec.md), [`12-acceptance-test-procedure.md`](12-acceptance-test-procedure.md) |
| An engineer reviewing the technique | [`03-bluetooth-le-primer.md`](03-bluetooth-le-primer.md) → [`04-ble-crc-corruption.md`](04-ble-crc-corruption.md) → [`06-firmware-architecture.md`](06-firmware-architecture.md) → source code |
| A security researcher | [`14-security-considerations.md`](14-security-considerations.md), then audit the firmware |
| Agency General Counsel or legal | [`15-legal-and-regulatory.md`](15-legal-and-regulatory.md) |
| An MP, a national-security analyst | the strategic Mitigation Report PDF (companion document), then this overview |

## A note on tone

This documentation is occasionally direct, occasionally informal. That is deliberate. The author's view is that the safety of law-enforcement officers is too serious for corporate hedging, and the engineering is too important to be obscured by jargon. If you find a passage that you cannot independently verify, or a claim that overstates what the device demonstrably does, file an issue. The goal is to produce a piece of work that an officer can stake their physical safety on, knowing what it does and what it does not.
