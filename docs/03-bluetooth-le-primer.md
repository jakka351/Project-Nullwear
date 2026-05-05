# Bluetooth Low Energy — A Primer for NULLWEAR Reviewers

This document is for engineers, security reviewers and informed officers who want to understand what NULLWEAR is doing at the radio-protocol level. It assumes general technical literacy but no prior BLE knowledge.

If you are an officer who just wants to wear the device, read [`08-user-manual.md`](08-user-manual.md) instead.

---

## What BLE is

Bluetooth Low Energy (BLE), introduced in Bluetooth 4.0 (2010), is a short-range wireless protocol designed for low-power devices. It shares the 2.4 GHz ISM band with classic Bluetooth, Wi-Fi, Zigbee, microwave ovens, baby monitors and a great deal else. It is now ubiquitous: every modern phone, smartwatch, fitness tracker, smart-home device, body-worn camera, smart holster and Taser includes a BLE radio.

### Two channel groups

The 2.4 GHz BLE band is divided into 40 channels, each 2 MHz wide. They split into two functional groups:

- **3 primary advertising channels — 37, 38, 39** at 2402 MHz, 2426 MHz and 2480 MHz respectively. These are deliberately spaced across the band to dodge Wi-Fi interference. Devices that want to be discovered, or that broadcast small data periodically, use these.
- **37 data channels — 0 through 36**, used for established connections after a device has been discovered and paired.

NULLWEAR cares **only** about the three advertising channels. Axon equipment broadcasts its identifying signature on these.

### Advertising packets — the structure

A BLE advertising packet looks like this on the air, at 1 Mbps PHY:

```
┌──────────┬──────────────┬────────┬──────────┬──────────┬──────┐
│ Preamble │ Access Addr  │ Header │ AdvA(MAC)│ AdvData  │ CRC  │
│   1 B    │     4 B      │  2 B   │   6 B    │ 0..31 B  │ 3 B  │
└──────────┴──────────────┴────────┴──────────┴──────────┴──────┘
   8 us       32 us         16 us     48 us     0..248 us  24 us
```

- **Preamble** — alternating bits (`10101010` or `01010101`) used by the receiver to lock onto bit timing.
- **Access Address** — for primary advertising, always the constant `0x8E89BED6`. The receiver discards every packet whose access address doesn't match what it expects.
- **Header** — 2 bytes describing the PDU type, the length of the payload, and a few flags.
- **AdvA** — a 6-byte (48-bit) MAC address. This is the device's unique identifier. The first 3 bytes (MSB end) are the IEEE-assigned OUI of the manufacturer.
- **AdvData** — an optional 0–31 byte payload. Manufacturer-specific advertising data, often containing service UUIDs, device names, etc.
- **CRC** — a 3-byte cyclic redundancy check covering the header, AdvA and AdvData. The receiver computes the CRC over the received bits and compares against the trailer. Any mismatch causes the packet to be silently discarded.

### Endianness — important for NULLWEAR

BLE transmits multi-byte fields little-endian on the air: the least-significant byte first.

A MAC written as `00:25:DF:AA:BB:CC` (the conventional notation, where `00:25:DF` is the OUI) is sent on the air in this byte order: `CC, BB, AA, DF, 25, 00`.

This means the OUI bytes (`00:25:DF`) are transmitted **last** in the AdvA field on the air. To detect the OUI, NULLWEAR must wait for the entire 48-bit AdvA to be received.

This single fact constrains the entire NULLWEAR timing budget — see [`04-ble-crc-corruption.md`](04-ble-crc-corruption.md).

### CRC — and why it matters

The CRC trailer is computed by the transmitter over (header || AdvA || AdvData) using the polynomial `x^24 + x^10 + x^9 + x^6 + x^4 + x^3 + x + 1` (i.e. `0x65B`), with initialisation `0x555555`. The receiver computes the same CRC over what it received and compares.

If the CRC fails, the receiver discards the packet **silently** — there is no log entry, no "almost received", no "we got most of it but the CRC failed". The packet simply does not exist as far as the receiver application layer is concerned.

This is the property NULLWEAR exploits.

### Whitening

BLE applies a data-whitening function (a 7-bit LFSR seeded by the channel index plus bit 6 set) to the entire packet content (header onward) to reduce DC bias and improve clock recovery. The receiver descrambles automatically on a successful access-address match.

The whitening seed is therefore deterministic from the channel: ch37 → `0x65`, ch38 → `0x66`, ch39 → `0x67` (using Nordic's register encoding convention).

This is relevant because if you want to fake or analyse a BLE packet at the radio layer, you must apply the correct whitening for the channel it's on. NULLWEAR's jam pulse does not need to be a valid BLE packet, but it must be transmitted on the correct channel — the channel of the original advertisement.

### Random vs public addresses

BLE provides several MAC address modes:

- **Public address** — a permanent IEEE-allocated MAC. Globally unique. Manufacturer-tied.
- **Random static address** — a random MAC generated at manufacture and persistent across power cycles. The top two bits identify it as random.
- **Random private resolvable address (RPA)** — a rotating address generated periodically (typically every 15 minutes or so) using a secret key that the device's authorised peer can resolve. Provides privacy against passive observers.
- **Random private non-resolvable address** — a rotating address with no key, used by devices that don't need to be re-identified.

Modern privacy guidance (e.g. Apple's published BLE recommendations, the BLE 5.x specification's privacy normative text) strongly recommends RPA for any device worn on a person.

**Axon equipment broadcasts public addresses** with the `00:25:DF` OUI. This is the choice that creates the vulnerability NULLWEAR mitigates.

If Axon ships a firmware update that switches to RPA — even random static would be a partial improvement — the BLE-OUI fingerprint disappears at the protocol layer, NULLWEAR becomes redundant against new firmware, and the world is in a better place. Until then, NULLWEAR fills the gap.

### Advertising types

Several advertising-PDU types exist:

- `ADV_IND` — connectable, scannable advertising. Used by devices that accept connections.
- `ADV_DIRECT_IND` — connectable, directed at a specific peer.
- `ADV_NONCONN_IND` — non-connectable, non-scannable. Used by beacons.
- `ADV_SCAN_IND` — scannable but not connectable.
- `SCAN_REQ` / `SCAN_RSP` / `CONNECT_REQ` — request/response packets exchanged between scanners and advertisers.

NULLWEAR's OUI matcher fires on **any** advertising PDU type whose AdvA carries the target OUI. The PDU type is irrelevant to the threat — what matters is that the manufacturer-tied address is on the air.

### Advertising interval

The Bluetooth specification permits advertising intervals from 20 ms to 10.24 s. Devices choose intervals based on their power budget and discoverability needs. Axon body cameras, by observation, advertise at intervals of approximately 30 ms when actively discoverable, dropping to lower rates when in low-power state.

This means a typical Axon body camera produces ~30 advertising packets per second per channel, ~90 packets per second across the 3 advertising channels combined. NULLWEAR's hop schedule of 80 ms per channel is fast enough to catch most of them — over a 240 ms cycle it visits each channel once, and at 30 ms advertising interval roughly 8 packets land in each cycle, of which the probability that NULLWEAR is on the right channel at the right moment is ~1/3 ≈ 33%. Over time, on average, **one in three packets** is captured and annihilated per single-pass through the channel cycle.

That's not enough — we want closer to 100%. To get there, the channel-hop timer can be reduced (e.g. 20 ms per channel = full cycle in 60 ms ≈ 100% packet capture rate at the cost of higher CPU load) or multiple NULLWEAR units can be carried (each on a different channel-hop offset). Engineering trade-offs to be tuned during pilot.

### Where to read more

- Bluetooth Core Specification 5.4 (or later), Volume 6 ("Low Energy Controller"), Part B ("Link Layer Specification") — the canonical reference.
- Nordic Semiconductor nRF5340 Product Specification, sections on RADIO peripheral, PPI/DPPI, and packet handling.
- Heinrich, Geier, et al., "BlueShift" — practical demonstration of BLE selective denial-of-service via reactive transmission. (See `REFERENCES.md`.)

That is the BLE foundation NULLWEAR is built on.
