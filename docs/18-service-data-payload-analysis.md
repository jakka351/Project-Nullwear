# Axon FE6B Service-Data Payload — Empirical Analysis

**Status:** preliminary findings from a 29-device sample. Conclusions are structurally well-supported but require corroboration with multi-observation data per device before being treated as ground truth.

**Operational classification:** PROTECTED. Specific per-device identifier bytes are deliberately omitted from this public document. The corresponding analysis CSV with full payload bytes is held by the maintainer under the same regime as the asset register.

---

## What this document is

The companion *Threat Validation Report* established that every Axon BLE advertisement observed in 3.4 years of telemetry carries a Service Data 16-bit AD structure on UUID `0xFE6B` (registered to Axon Public Safety with the Bluetooth SIG). The data bytes following the UUID — the **payload** — are 18 bytes (in docked-mode broadcasts) or 23 bytes (in deployed-mode broadcasts) wide.

This document analyses what those payload bytes encode.

It matters because:

- It validates that NULLWEAR v1.1's dual-signature matcher (OUI + FE6B) catches every observed Axon broadcast variant.
- It surfaces a **second-order privacy concern**: Axon's apparent "docked / privacy-mode" broadcast (where the MAC is sanitised to `00:00:00:00:00:00`) still leaks a stable per-device identifier, defeating the apparent intent of zeroing the MAC.

## Sample

- 29 Axon devices broadcasting in deployed mode (MAC starts with `00:25:DF`, payload length 23 bytes).
- 1 Axon device broadcasting in docked / privacy mode (MAC = `00:00:00:00:00:00`, payload length 18 bytes).
- All observations from the same Greater Melbourne dataset described in the *Threat Validation Report*.

## Inferred payload structure

### Deployed-mode payload — 23 bytes

| Byte(s) | Observed pattern | Interpretation |
|---|---|---|
| `0` | Constant `0x02` across all 29 devices | **Format / broadcast-mode identifier** — `0x02` = deployed |
| `1` | Two values, `0x01` (62%) and `0x02` (38%) | **Device sub-state flag** — possibly recording-on / recording-off, or holster-paired / not-paired |
| `2`–`11` | 10 bytes, ~5 bits of entropy each, near-unique per device | **Stable per-device identifier (80 bits)** — likely a hash / salted serial / device-key fingerprint |
| `12`–`13` | Constant `0x00 0x00` | Padding or reserved |
| `14`–`18` | Constant ASCII string `X60J0` across all 29 devices | **Hardware model code** — strongly consistent with the Axon Body 3 family ("AB3", model series "X60") |
| `19`–`22` | 4 ASCII characters in the range `0`–`9` and `K`–`V` | **Serial-number fragment** — last 4 characters of the unit's printed serial number, broadcast in cleartext |

### Docked / privacy-mode payload — 18 bytes (1 device sample)

| Byte(s) | Value (this device) | Interpretation |
|---|---|---|
| `0` | `0x01` | **Format identifier** — `0x01` = docked / non-deployed |
| `1` | `0x01` | Sub-state flag (same family as deployed format) |
| `2`–`11` | 10 unique bytes | **Stable per-device identifier — same field as deployed format** |
| `12`–`13` | `0x00 0x00` | Padding (same as deployed) |
| `14`–`17` | 4 bytes of state / dock-pairing data | **Short trailing region** — likely dock-slot / battery / status |

The trailing region of the docked payload is shorter than deployed (4 bytes vs 9 bytes) — it omits the cleartext model + serial-fragment string, which makes sense: a device sitting in its dock doesn't need to announce its hardware model because the dock already knows.

## The privacy-leak finding

**The 80-bit identifier in bytes 2–11 appears in BOTH the deployed and docked payload formats.** It is structurally a per-device fingerprint — high entropy across devices, but presumably stable within any given device.

If this stability holds (must be confirmed with multi-observation data per device — see "What we have not yet verified" below), then:

> **Axon's apparent privacy-mode broadcast — sanitising the MAC to all-zero — does not achieve privacy. The same physical device is re-identifiable across observations via the 80-bit FE6B payload identifier, with or without a meaningful MAC.**

This is the kind of finding that constitutes a follow-on disclosure to the original BLE-OUI vulnerability. The mitigation is the same as for the OUI: ship a firmware that uses a Resolvable Private Address mechanism for the per-device identifier byte too. Until then, every Axon device leaks two independent stable signatures (the public MAC, and the FE6B payload ID), and obscuring one without the other achieves nothing.

## Implications for NULLWEAR

### v1.1 firmware

The dual-signature matcher (see [`docs/17-firmware-v1.1-dual-signature-matcher.md`](17-firmware-v1.1-dual-signature-matcher.md)) catches both broadcast modes by matching on the 16-bit `0xFE6B` UUID, which is present in both the 23-byte deployed payload and the 18-byte docked payload. The payload-structure analysis here confirms that v1.1's design choice to match on the UUID rather than on payload bytes is correct — the UUID is the only field guaranteed stable across both modes.

### Future v1.2 (if ever needed)

If Axon ships a future firmware that suppresses the `0xFE6B` UUID but still carries the 80-bit identifier elsewhere, NULLWEAR could be extended to scan for the per-device identifier directly. This would be much more complex (the identifier is per-device, so the matcher would need a learned database — a significant architectural change). Not needed now.

### What this means for officer privacy posture

If your agency is relying on Axon's docked / sanitised-MAC behaviour as a privacy feature for stored equipment, **stop**. The 80-bit identifier still leaks. An adversary with a passive BLE listener inside or near a police station can build a stable fingerprint of every body camera in inventory, then track those cameras as they leave the station and are issued to officers (because the same identifier persists into the deployed-mode broadcasts). This means even **off-duty equipment is a tracking surface**.

The recommendation in the strategic Mitigation Report (§9.1: Axon to ship default RPA firmware) should be updated to include: **the per-device identifier in the FE6B payload must rotate with the same cadence as the MAC**, otherwise the fix is half-done.

## Cleartext model / serial broadcast

Bytes 14–22 of the deployed payload contain ASCII characters that strongly resemble the Axon hardware model code (`X60J0` — consistent with the Axon Body 3 product family) plus a 4-character serial-number fragment.

This means a passive observer can also extract:

- **The Axon hardware model in service** at any observed location.
- **Potentially the last 4 characters of every body camera's serial number**, which when combined with the agency's procurement records (Axon Body 3 fleet, serial range) might allow an adversary to map specific serials to specific officers.

**This is a third independent piece of identifying information leaked per advertisement** (after the MAC and the 80-bit identifier).

## What this means for the strategic threat picture

The original 2022 disclosure framed the vulnerability around the OUI. The 2026 Mitigation Report broadened it to the protocol-layer fact of broadcasting a manufacturer-tied MAC. This deep-dive shows the threat surface is **broader still**:

| Identifier | Length | Mode | Re-identifiable? | Mitigated by NULLWEAR v1.0 | Mitigated by NULLWEAR v1.1 |
|---|---|---|---|---|---|
| MAC OUI prefix | 24 bits | Deployed | yes (manufacturer attribution) | yes | yes |
| Full MAC (per-device suffix) | 48 bits | Deployed | yes (per-device) | yes | yes |
| FE6B payload ID | 80 bits | Deployed AND docked | yes (per-device, even with zeroed MAC) | no (deployed-mode only by OUI side-effect) | yes (matched via UUID, both modes) |
| Hardware model ASCII | 5 bytes | Deployed | yes (model attribution) | yes (annihilated as part of packet) | yes |
| Serial fragment ASCII | 4 bytes | Deployed | yes (per-device) | yes | yes |

NULLWEAR v1.1 annihilates the entire packet, so all of the above identifiers are suppressed within the protection bubble. The privacy gap is OUTSIDE the bubble — i.e. for fielded equipment that a NULLWEAR/P is not currently protecting (e.g. devices in a dock at a station with no NULLWEAR/S installed, or devices in a vehicle with no NULLWEAR/V).

## What we have not yet verified

This analysis rests on a 29-device sample of single observations per device. The structural claims are well-supported, but the following points require follow-up:

1. **Is the 80-bit identifier stable for the same device across multiple observations?** The strongest privacy claim depends on this. The dataset's repeated detections of the same device only have RSSI / GPS recorded per-detection, not raw advertisement payloads — so we can't directly verify intra-device payload stability. The structural argument (one stable value per device row in the dataset) is suggestive but not conclusive.
2. **Does the byte-1 sub-state flag toggle predictably?** Two values are observed (`0x01` and `0x02`). Multi-observation data of the same device would tell us whether this changes with operational state (recording on/off, holster paired/unpaired).
3. **Is the model code `X60J0` actually Axon Body 3?** Strongly suggested by Axon's published model nomenclature, but a definitive mapping would come from Axon's own product documentation or a known-model side-channel.
4. **What does the docked-mode trailing region (bytes 14–17 of the 18-byte payload) actually encode?** A single sample is insufficient to characterise. More docked-state captures would resolve this — your Sydney police-station log would be the natural source.

Sending more data to the maintainer (per [`CONTACT.md`](../CONTACT.md)) lets these be answered properly.

## Reproducibility

Analysis is in the maintainer's local Tracking Data analysis directory:
`08_service_data_deep_dive.py` reads a RaMBLE SQLite export, extracts every FE6B payload, runs per-byte entropy analysis, and emits anonymised CSV + summary JSON + this findings document template.

The CSV maps anonymised pseudonyms (`AX-####`) to per-byte values; **the real MAC mapping is held by the maintainer only**, per the project's secrets-and-publishing policy.

## Cross-references

- v1.1 firmware design: [`docs/17-firmware-v1.1-dual-signature-matcher.md`](17-firmware-v1.1-dual-signature-matcher.md)
- BLE primer: [`docs/03-bluetooth-le-primer.md`](03-bluetooth-le-primer.md)
- Threat Validation Report (companion PDF, held by maintainer)
- Bluetooth SIG 16-bit UUID registry: <https://www.bluetooth.com/specifications/assigned-numbers/>
- Axon Body 3 product page: <https://www.axon.com/products/axon-body-3>
