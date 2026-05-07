# Project-Nullwear — Feasibility Analysis

**Repository:** [jakka351/Project-Nullwear](https://github.com/jakka351/Project-Nullwear)
**Subject:** An open-source mitigation device for the Axon Bluetooth OUI broadcast vulnerability
**Author of repo:** Benjamin Jack Leighton (Tester Present, [testerpresent.com.au](https://testerpresent.com.au))

---

## 1. What the repo actually proposes

A wearable, vehicle-mount, and station-mount selective BLE jammer built around an **nRF5340 SoC**. It listens on the three BLE advertising channels, watches for any incoming packet whose first three MAC bytes equal `00:25:DF` (the IEEE-registered OUI for Taser/Axon), and — using the network core's `BCMATCH`/`RXMATCH` events plus PPI/DPPI hardware event chaining — fires a corrupting RF pulse during the packet's 24-bit CRC field so that every receiver in range fails CRC and silently drops the packet.

| Aspect | Detail |
|---|---|
| Form factors | Personal (P), Vehicle (V), Station (S) — shared firmware |
| SoC | Nordic nRF5340 (dual Cortex-M33; dedicated network-core radio) |
| RF front-end | Skyworks SKY66112-11 |
| Antenna | Johanson 2.4 GHz chip antenna (P); external roof (V); distributed indoor (S) |
| Power | 200 mAh LiPo, USB-C, ≥18 h target battery life |
| SDK | Nordic nRF Connect SDK v2.5+ / Zephyr v3.4+ |
| Verification | Python `bleak`-based reference receiver; ESP32 Axon-emulator test source |
| Author profile | Real person; established Australian Ford reverse-engineering portfolio (J2534, UDS, PCM flashing); **no public RF/BLE firmware track record** |

---

## 2. What holds up

### 2.1 The vulnerability is genuine and uncontested

A Python script (`lookout.py`) exists in the wild specifically for detecting nearby Axon equipment by scanning for the `00:25:DF` OUI. Axon (formerly TASER International) is publicly assigned the MA-L `00:25:DF`, so any BLE/Wi-Fi MAC starting with those three bytes is almost certainly Axon kit. The 24-bit OUI is broadcast in plaintext multiple times per second by every Axon BLE-emitting device in normal operation. This has been demonstrated publicly since DEF CON 31 (2023). It is not a hypothetical attack — it is already a real OSINT primitive being used in projects like Vertex's WiGLE work to map police vehicles and stations.

### 2.2 The mitigation technique is real and well-published

Selective reactive narrow-band jamming of BLE advertising — using commercially available BLE-capable hardware to corrupt packets matching a target address so receivers detect the corruption via CRC — is a documented academic technique. The VaktBLE work goes further, demonstrating a non-compliant peripheral that jams the CRC of specific BLE packets while extracting connection parameters in real time. Damien Cauquil's BTLEjack and similar projects have shown the timing budgets are achievable on Nordic silicon.

### 2.3 The hardware platform is appropriate

The nRF5340 specifically — dual Cortex-M33 with a dedicated network-core radio, `BCMATCH` event firing on a configurable bit position, sub-microsecond PPI/DPPI event chaining, hardware RX→TX ramp — is a sensible platform choice. The ~30 µs CRC window after ~80 µs of preamble + access address + header is tight but tractable.

### 2.4 The repository is unusually honest about its own limitations

The "Validation Status" table explicitly buckets every claim into **Verified / Awaiting verification / Reasonable estimate**, and openly states the firmware has never been test-compiled, the PCB CAD doesn't exist yet, range numbers are projections from antenna specs not measurements, and the legal pathway is the author's interpretation. That self-discipline raises the document above most "I built a thing on GitHub" projects.

---

## 3. Where it falls down

### 3.1 Legal status — the single biggest problem

The repo describes this as *"Selective passive-emission device; legal under LIPD 2015 (AU) class licence with a single ACMA Ministerial determination."* This is essentially wrong.

- **Section 197** of the *Radiocommunications Act 1992* makes it an offence to cause interference to radiocommunications.
- The **LIPD class licence** *requires* devices not to cause interference to radiocommunication services. The whole point of NULLWEAR is to deliberately cause interference to a specific radiocommunication service. There is no possible reading of LIPD that admits a device whose entire function is targeted in-band emission timed to destroy other devices' packets.
- The **defence/national-security carve-out** in Sections 24–27 is for military and intelligence use, not general policing.
- A "single Ministerial determination" is **not a real regulatory pathway** anyone has walked for police jammers in Australia; it would require fresh primary regulation, not a tweak to an existing class licence.
- Same picture in the **US** (FCC's anti-jammer enforcement is aggressive and unequivocal), **UK** (Ofcom), and **Canada** (ISED).

> **Realistically this device cannot lawfully be deployed without statutory change in any of the named jurisdictions.**

### 3.2 The "annihilation" framing oversells what the technique does

Per-packet selective CRC corruption stops a *standards-compliant BLE receiver* from accepting the packet. It does **not** stop a determined adversary.

#### a) The OUI is already on the air before the corruption pulse

The OUI bytes hit the air before NULLWEAR's corruption pulse fires — the technique by design lets the address transmit in the clear and only corrupts the trailing CRC. Any attacker using a software-defined radio or a modified BLE chipset that captures raw bits without enforcing CRC will still recover the OUI. This is exactly how BLE sniffers like the nRF52840 dongle in promiscuous mode work — CRC-failed packets are routinely captured and viewable in Wireshark, just flagged as bad-CRC. The architecture only protects against attackers using stock BLE stacks. **Anyone who has read this README will have already reconfigured their scanner.**

#### b) The corruption pulse is itself a beacon

NULLWEAR's burst pattern — periodic wideband emissions on channels 37/38/39 timed to whenever an Axon device speaks — is itself a fingerprint. An adversary can scan for *NULLWEAR's* transmissions and locate the officer that way. You've swapped one fingerprint for a more interesting one.

#### c) Single-radio coverage of three hopping channels

The device only has one radio. BLE adverts hop across three channels with random offsets, so a single-radio jammer must channel-hop. It will inevitably miss the first packet on each channel transition. Real Axon devices send adverts many times per second; missing one in twenty still gives a city-scale scanner mesh enough samples to track. The repo's own table claims "Total — no clean Axon advertisement crosses the protection bubble" but that claim isn't supported by the architecture as drawn.

### 3.3 Self-interference is hand-waved

"Impact on Wi-Fi, Zigbee, other BLE: None" is asserted, not analysed. A 30 µs burst with enough TX power to corrupt CRC at 30 m has non-trivial spectral skirts.

| BLE adv channel | Frequency | Wi-Fi channel it sits in |
|---|---|---|
| 37 | 2402 MHz | bottom of Wi-Fi ch. 1 |
| 38 | 2426 MHz | Wi-Fi ch. 6 |
| 39 | 2480 MHz | Wi-Fi ch. 13/14 |

The officer's own radios — body-cam Wi-Fi backhaul, Bluetooth earpiece, Taser pairing, dock-station handshake at end of shift — are all in this band. Whether the device degrades the officer's *own* equipment is a measurement question that has not been done.

### 3.4 Timing claim needs measurement, not assertion

The 40 µs RX→TX turnaround is right at the edge of nRF5340 capability. The Nordic radio's `RXEN`→`READY`→`TXEN`→`READY` chain and `DISABLE` state has documented timing in the product specification, and a hard-real-time PPI sequence can hit it, but the author's own status table flags this as *"written from first principles, not yet test-compiled."* Until someone has flashed this onto silicon and put it on a scope, the timing budget is **plausible rather than proven**.

### 3.5 Maturity is honestly described but very early

| Item | Status |
|---|---|
| PCB CAD | Does not exist |
| Enclosure CAD | Does not exist |
| Compiled firmware | Has never been built |
| Bench measurement | None |
| EMC test | None |
| Field test | None |
| GitHub stars / forks | 2 / 0 |

The "complete reference implementation" is reference C code that has never run. That's fine for a feasibility-study deliverable, but the README simultaneously markets it as a turnkey package agencies can hand to a contract manufacturer for a 200-unit pilot — **which it is not**.

### 3.6 The political framing damages the deliverable

- Calling out Axon directly in the README
- Instructing the reader not to use Chinese contract manufacturers
- The Australian flag SVG in the repo root
- The `ATTN: MICROSOFT` file in the root
- The salute emoji at the end

None of this belongs in a document being sent to ASIO, ACSC, or parliamentary committees. The technical core is defensible; the editorial wrapper is going to make procurement and counsel tune out before they reach it.

### 3.7 There is a far cheaper fix the README itself acknowledges

The proper mitigation, called out in the repo's own closing note: **Axon ships a firmware update that rotates Resolvable Private Addresses (RPAs) on its BLE peripherals.** That is a few weeks of vendor work and zero hardware deployed. Every dollar spent on NULLWEAR is a dollar that doesn't go into pressuring the vendor to do the actual fix.

---

## 4. Bottom line

As an **engineering thought experiment** and a **forcing function on Axon**, the project is intellectually serious and the underlying technique is real.

As a **deployable product**, it has three blocking problems in roughly this order:

1. **It is illegal to use as described in every jurisdiction it names**, with no realistic short-term legal pathway.
2. **The "invisibility" claim doesn't survive contact with an attacker who has read the spec**, because the OUI hits the air before the CRC corruption pulse and the corruption pulse is itself a fingerprint.
3. **Nothing in the BoM, mechanical, or firmware path has been physically built or measured** — the whole stack is paper.

### Recommendation

If reviewing this as an agency technical advisor: **don't fund a pilot.** Use the disclosure leverage to push Axon for a firmware update implementing RPA rotation on all BLE-emitting products, with a hard public deadline. If Axon won't move, the right pressure is **procurement and regulatory**, not a custom jammer.

The repo is most useful as the **threat-validation half** of that conversation — the documented existence of a credible mitigation an agency *could* be forced to deploy is itself the lever.

---

## 5. Summary scorecard

| Dimension | Verdict |
|---|---|
| Underlying vulnerability is real | ✅ Confirmed in public literature and tooling |
| Selective CRC-corruption technique is real | ✅ Published academic technique |
| nRF5340 platform choice is sound | ✅ Appropriate silicon |
| Reference firmware is buildable as written | ⚠️ Unverified — never compiled |
| Timing budget achievable on hardware | ⚠️ Plausible, not measured |
| Defeats a knowledgeable adversary | ❌ No — OUI transmits before CRC corruption |
| Legal under AU LIPD 2015 | ❌ No — directly contradicts Section 197 |
| Legal in US / UK / CA | ❌ No |
| PCB / mechanical CAD ready for CM | ❌ Does not exist |
| Lab + field test data | ❌ None |
| Self-interference with officer's own gear analysed | ❌ Asserted away, not measured |
| Tone appropriate for parliamentary / agency audience | ❌ No |

---

*Analysis prepared 8 May 2026.*
