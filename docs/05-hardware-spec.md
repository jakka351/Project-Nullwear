# Hardware Specification (short form)

A condensed reference. The full engineering specification, including mechanical drawings, BoM with line costs, manufacturing process, and acceptance criteria, is in the **Engineering Specification PDF** (companion document).

This file gives the contract manufacturer enough information to scope, the agency engineer enough to understand, and the procurement officer enough to budget.

---

## NULLWEAR/P (Personal)

| Parameter | Value |
|---|---|
| Form factor | 50 × 35 × 11 mm, ~28 g |
| Enclosure | Polycarbonate / ABS blend, ultrasonically welded, internally potted |
| Ingress protection | IP67 (1 m / 30 min) |
| Drop resistance | 1.5 m onto concrete, 6 faces |
| Operating temperature | –10 to +55 °C |
| Storage temperature | –20 to +60 °C |
| SoC | Nordic Semiconductor nRF5340 (QFN-94) |
| RF front-end | Skyworks SKY66112-11 |
| Antenna | Johanson 2450AT43A100 chip antenna |
| TX power | +8 dBm peak, ~30 µs duty per detected target |
| RX sensitivity | –96 dBm @ 1 Mbps PHY (per Nordic datasheet) |
| Effective range | 10–30 m line-of-sight (typical) |
| Battery | 200 mAh LiPo cell with PCM |
| Charge | USB-C, 5 V / 500 mA, ~50 min full charge |
| Charge controller | Microchip MCP73831 |
| Fuel gauge | Maxim MAX17048 (I²C) |
| Endurance | ≥ 18 hours per charge (typical 21–24 hours) |
| Indicators | Single 0805 RGB LED |
| Controls | Single tactile switch |
| Diagnostic | USB-CDC at 115200 baud |
| Bootloader | MCUboot, signed images |
| Mounting | Two M3 brass inserts (rear); belt clip, MOLLE mount, body-cam mount |

### Reference BoM (per unit, AUD, 50k volume)

| Class | Component | MPN | Unit | Ext |
|---|---|---|---|---|
| SoC | Dual-core M33 + 2.4 GHz radio | Nordic nRF5340 | $8.40 | $8.40 |
| RF FE | LNA + T/R sw | Skyworks SKY66112-11 | $2.10 | $2.10 |
| Charger | 1S LiPo charger | Microchip MCP73831T | $0.70 | $0.70 |
| Fuel gauge | I²C battery monitor | Maxim MAX17048 | $1.30 | $1.30 |
| LDO | 3V3 100mA low-Iq | TI TLV75533 | $0.45 | $0.45 |
| Load switch | Low-Rdson | TI TPS22918 | $0.55 | $0.55 |
| ESD/TVS | USB-C ESD | ST USBLC6-2SC6Y | $0.40 | $0.40 |
| Reverse-pol | P-MOSFET | Diodes DMP3098L | $0.25 | $0.25 |
| Crystal | 32 MHz, ±20 ppm | Abracon ABM3B | $0.55 | $0.55 |
| Crystal | 32.768 kHz | Abracon ABS07 | $0.40 | $0.40 |
| Antenna | 2.4 GHz chip | Johanson 2450AT43A100 | $0.95 | $0.95 |
| Battery | 200 mAh LiPo + PCM | PowerStream PGEB0042530 | $3.80 | $3.80 |
| USB-C | Right-angle SMD | Amphenol 12401610E4#2A | $0.85 | $0.85 |
| LED | 0805 RGB | Cree CLV1A-FKB | $0.30 | $0.30 |
| Switch | Tactile, sealed | C&K KMR2 | $0.35 | $0.35 |
| Passives | ~35 caps, resistors, inductors | Various | $0.04 | $1.40 |
| PCB | 4-layer FR-4, ENIG | (CM-sourced) | $2.40 | $2.40 |
| Enclosure | PC-ABS injection-moulded, 2-piece | Custom-tooled | $3.80 | $3.80 |
| Belt clip | Stainless 304 | Custom | $1.20 | $1.20 |
| Light pipe | 3 mm acrylic | Custom | $0.30 | $0.30 |
| Potting | Polyurethane | Wevolt PU-7000 | $1.10 | $1.10 |
| Misc | Gasket, brass inserts, hardware | — | — | $0.40 |
| **Components subtotal** |  |  |  | **$31.95** |
| Assembly + test + program + pack |  |  |  | **$12.70** |
| Margin / OH / contingency |  |  |  | **$11.20** |
| **All-in target unit cost** |  |  |  | **~$55.85** |

---

## NULLWEAR/V (Vehicle)

Same SoC, firmware and antenna chain as /P. Differences:

| Parameter | Value |
|---|---|
| Form factor | 75 × 50 × 22 mm, ~110 g |
| Enclosure | Polycarbonate, gasketed, IP65 |
| Power input | 12 V switched (with ignition-sense lead), 1 A fast-blow inline fuse |
| Power buck | TI LMR33630ADDAR (12 V → 5 V, 90% η) |
| Backup battery | 500 mAh LiPo (~6 h ride-through with engine off) |
| Antenna | External, magnetic-mount, Linx ANT-2.4-CW-RH or equivalent (3 m RG-316) |
| Connector | Female SMA panel jack |
| Effective range | 30–50 m (with external roof antenna) |
| Indicators | 2× LEDs (power, RF activity) |
| Mounting | 4× M4 stainless on flange |
| Vibration | ISO 16750-3 rigid-mount profile |
| Indicative cost (25k volume) | AUD 82–95 per unit |

---

## NULLWEAR/S (Station)

Centralised distributed-antenna unit.

| Parameter | Value |
|---|---|
| Form factor | 200 × 150 × 60 mm, ~1.6 kg |
| Enclosure | Aluminium 6061-T6 extrusion, anodised |
| Power input | 100–240 V AC (IEC C14), 30 W |
| AC-DC | Mean Well IRM-30-5 |
| UPS | 2200 mAh LiFePO₄ pack, ~4 h ride-through |
| Mounting | DIN rail (35 mm) + M5 wall mount holes |
| Antenna ports | 8× female N-type panel connectors |
| Antennas | 8× indoor (Taoglas TG.30.8113 or equivalent), ceiling-mount |
| Antenna feed | 8× pre-terminated RG-316 coax, 8 m each |
| Per-port effective range | 5–15 m indoor (depends on building structure) |
| Cooling | Convection only (no fans) |
| Network | RJ-45 Ethernet, monitoring only (no command surface) |
| Indicative cost (1k volume) | AUD 600 per unit |

---

## What's the same across variants

Every variant uses:

- The same nRF5340 SoC.
- The same firmware sources (with variant-specific board definitions).
- The same OUI matcher logic.
- The same selective-CRC-corruption technique.
- The same diagnostic interface (USB-CDC).
- The same MCUboot signing keys.
- The same operations and acceptance procedures (with variant-specific mechanical and power test additions).

This shared architecture means three production lines feed one firmware repo. A bug fix or feature addition in one variant ships to all three with one rebuild.

---

## What's not in this file

- Mechanical drawings (top, side, exploded views) — see Engineering Specification PDF.
- PCB layout, schematic, gerbers — see `pcb/` directory of this repository (placeholder until pilot CM produces them).
- Enclosure CAD (STEP / STL) — see `enclosure/` directory (placeholder).
- Detailed RF chain, matching network, antenna keepout — see Engineering Specification PDF Part III.
- Production process, SMT reflow profile, conformal coating — see Engineering Specification PDF Part VII.
