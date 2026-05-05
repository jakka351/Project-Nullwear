# BLE CRC Corruption — How NULLWEAR Annihilates a Packet

This document describes the mechanism NULLWEAR uses to make a BLE advertising packet undetectable. It is intended for engineers reviewing the firmware, security researchers seconding an opinion, and anyone evaluating whether the technique is sound.

---

## The fundamental observation

Bluetooth Low Energy receivers silently discard any received packet whose computed CRC does not match the trailing CRC field on the packet. The receiver does not log a CRC failure as "almost a packet" — it logs nothing. The packet, from the application's point of view, did not exist.

Therefore: **if we can guarantee that the receiver computes the wrong CRC for a target packet, that packet vanishes from every system the receiver feeds.**

The CRC is computed over the header, AdvA and AdvData. To force a CRC mismatch, we need to corrupt **any** bit anywhere in those fields, OR corrupt the CRC trailer itself, OR both. Any single-bit error anywhere in the protected region of the packet produces a CRC failure with probability ≈ 1 (the BLE CRC is 24-bit; the false-pass rate of an arbitrary perturbation is ~2⁻²⁴ ≈ 6 × 10⁻⁸).

---

## The technique

NULLWEAR transmits a colliding RF burst, on the same channel as a target packet, while the target packet's trailing bytes are still on the air. The collision produces decode errors at the receiver, which propagate into the CRC check, which fails. The target packet is silently discarded.

This is **selective interference**, not jamming:

- It is not continuous — the radio transmits only when triggered by detection of a target packet.
- It is not band-wide — it transmits only on the specific channel the target packet is on (37, 38, or 39).
- It is not protocol-agnostic — it triggers only on detection of the target OUI in the target packet's MAC field.
- It is not affecting non-target packets — Wi-Fi, Zigbee, other BLE devices, and other manufacturers' BLE traffic continue normally.

The technique is documented in academic literature (Heinrich et al., Brauer et al. — see [`REFERENCES.md`](REFERENCES.md)).

---

## Why a colliding pulse causes CRC failure

A BLE 1 Mbps PHY uses GFSK modulation: zero and one are transmitted as different frequency offsets from the channel centre frequency. When two transmitters radiate simultaneously on the same channel, the received signal is the vector sum of the two transmissions at the antenna of the receiver. For receivers that operate at signal-to-interference ratios much less than 10 dB (which is true for most short-range receivers), the bit decisions become essentially random in the overlap region.

Random bits in any of (header || AdvA || AdvData || CRC) cause CRC failure with probability ≈ 1 — 2⁻²⁴, i.e. effectively certainty.

It is not necessary that the colliding pulse be a valid BLE packet, or even a meaningful waveform. Any energy on the same channel during the target packet's bit-stream is sufficient. A blank carrier, a noisy waveform, an unrelated BLE packet — all work.

---

## The timing budget

The hard part is timing. The target packet is on the air for between ~80 µs (minimum: header + 6-byte AdvA + 3-byte CRC, no AdvData) and ~376 µs (maximum: + 31 bytes of AdvData). NULLWEAR's transmission must overlap with the protected region.

```
Time (µs) →
0       8       40       56        104       104+L     128+L
│───────│───────│────────│─────────│─────────│─────────│
 Preamble  AA    Header   AdvA      AdvData    CRC
                                    (L bytes)
                          ←─────────────────────→ NEEDED OVERLAP
                                    └─→ NULLWEAR pulse should land here
```

Where `L` is the AdvData length in bytes (each byte = 8 µs at 1 Mbps).

For the minimum-length advertisement (`L = 0`):
- CRC region: 104–128 µs from packet start.
- NULLWEAR must transmit before 128 µs from packet start.

For a typical Axon advertisement (`L ≈ 10` bytes):
- CRC region: 184–208 µs from packet start.
- NULLWEAR must transmit before 208 µs from packet start.

NULLWEAR's detection-to-transmission pipeline:

| Phase | Duration (µs) | Cumulative |
|---|---|---|
| Packet preamble + access address | 40 | 40 |
| Address-match event fires; RX continues | 0 | 40 |
| 16 bits of header received | 16 | 56 |
| 48 bits of AdvA received → BCMATCH event fires | 48 | 104 |
| ISR latency (radio core, no OS interrupt mediation) | < 5 | 109 |
| OUI compare (3-byte equality) | < 1 | 110 |
| TASKS_DISABLE issued; radio ramps down | ~6 | 116 |
| DPPI link fires DISABLED → TXEN; radio ramps up | ~40 | 156 |
| Jam pulse on the air | (continues) | 156+ |

For the typical Axon case (CRC region 184–208 µs), this leaves a margin of ~28 µs before the CRC region starts — ample headroom.

For the minimum-length case (CRC region 104–128 µs), NULLWEAR is **late** by ~28 µs. The CRC region has been fully transmitted before NULLWEAR begins. In this scenario the technique would fail — except that minimum-length BLE advertisements are extremely rare in practice, and Axon advertisements always include service data.

To handle the absolute worst case, NULLWEAR's jam pulse extends 60 µs past the latest expected CRC region. For a packet with `L = 0`, this means the jam continues into the next packet's preamble window (which is harmless — preamble bits are just bit-sync, the access-address match check on the next packet is unaffected). For typical packets, the jam covers the CRC region cleanly.

---

## Why the OUI is at the *end* of AdvA

This is the timing constraint that drove the entire design. BLE transmits MAC addresses little-endian on the air: LSByte first. The OUI bytes are the **most significant** three bytes of a MAC, so they are sent **last** in the AdvA field.

Detection of the OUI therefore requires receiving the entire 48-bit AdvA — not just the first 24 bits. This adds 24 µs of waiting compared with a hypothetical "OUI first" encoding, which would have been the more privacy-friendly choice but is not what the spec mandates.

(Note: this is not an Axon design choice. It is the BLE specification's choice. Axon could mitigate by using Resolvable Private Addresses, which would eliminate the manufacturer fingerprint entirely.)

---

## Why DPPI/PPI is essential

The Nordic nRF5340 (and the older nRF52840) provides a hardware event-task chaining mechanism called **PPI** (Programmable Peripheral Interconnect) and its newer dist-form **DPPI**. This allows a peripheral event (e.g. RADIO `EVENTS_DISABLED`) to directly trigger a peripheral task (e.g. RADIO `TASKS_TXEN`) without any CPU mediation.

Without DPPI, the RX→TX transition would require:
1. CPU receives DISABLED interrupt (~5 µs of NVIC + ISR entry latency).
2. CPU writes 1 to `TASKS_TXEN`.
3. Radio begins TX ramp.

With DPPI, the link is direct: as soon as the radio's internal state machine raises DISABLED, the silicon initiates the TXEN task in the next clock cycle. The interrupt-mediated path saves no time but adds jitter — and in this application, jitter is fatal because we have to land in a bounded window.

NULLWEAR uses DPPI to chain DISABLED → TXEN, ensuring the silicon-minimum RX→TX transition (~40 µs).

---

## What can go wrong

| Failure mode | Likelihood | Mitigation |
|---|---|---|
| Minimum-length advertisement: jam misses the CRC window | Rare (Axon advertisements always carry service data) | Jam pulse duration extended to absorb worst case |
| Packet on a channel NULLWEAR is not currently on | ~67% per individual packet, but reduces to near-zero over multi-second observation due to channel hopping | Reduce channel-hop interval; or use multiple NULLWEAR units with offset hop schedules |
| Receiver is much closer to source than NULLWEAR | NULLWEAR's TX must be loud enough at the receiver to overpower the source. NULLWEAR at +8 dBm in a duty-cycle-bounded burst is comparable to typical Axon TX power; effective at distances where source power is similar. | Place NULLWEAR on the same person as the source — guaranteed close-range. Vehicle and station variants use higher TX power and/or external antennas. |
| Source uses a non-standard PDU type | Unlikely — Axon equipment uses standard advertising types | NULLWEAR matches on AdvA OUI regardless of PDU type |
| Source uses a randomised MAC | Would defeat NULLWEAR — but is also the engineering fix Axon should be making | This would mean Axon has fixed the underlying vulnerability; NULLWEAR becomes redundant. Win condition. |

---

## Verification

The only meaningful verification is empirical, not analytical:

1. Build a NULLWEAR unit and a known BLE source (the ESP32 emulator).
2. Run the reference receiver and measure baseline detections.
3. Activate NULLWEAR and re-measure.
4. Compute Packet Annihilation Ratio (PAR).

The acceptance threshold (PAR ≥ 0.99 at 5 m) is set in [`12-acceptance-test-procedure.md`](12-acceptance-test-procedure.md). The field-realistic threshold is set in [`10-field-testing-protocol.md`](10-field-testing-protocol.md). Both must pass before any operational deployment.

Until pilot units are built, the technique remains a high-confidence engineering prediction, not a measured field result. Be honest about that.
