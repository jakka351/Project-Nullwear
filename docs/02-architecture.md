# Architecture

System-level architecture, signal flow, and component responsibilities.

---

## System view

NULLWEAR is a single-chip, single-purpose radio device. Every NULLWEAR variant (P, V, S) shares the same silicon, the same firmware, and the same operating principle. They differ only in form factor, antenna, and power source.

```
                       ┌────────────────────────────┐
                       │   Air interface (2.4 GHz)  │
                       │   BLE adv channels 37/38/39│
                       └─────┬───────────────┬──────┘
                             │ RX            │ TX (jam pulse)
                             ▼               ▼
┌──────────────────────────────────────────────────────────┐
│                    nRF5340 SoC                            │
│  ┌─────────────────────────┐  ┌─────────────────────────┐│
│  │ NETWORK CORE (M33 64MHz)│  │ APP CORE (M33 128 MHz)  ││
│  │  - RADIO peripheral      │  │  - Battery mgmt         ││
│  │  - PPI/DPPI hardware    │  │  - LED driver           ││
│  │    event chaining        │  │  - State machine        ││
│  │  - OUI matcher           │  │  - USB-CDC diag         ││
│  │  - Jam pulse generator   │  │  - Stats aggregation    ││
│  └────────────┬─────────────┘  └────────────┬────────────┘│
│               │ IPC service (mbox + shared RAM)           │
└───────────────┼──────────────────────────────────────────┘
                │
        ┌───────▼────────┐    ┌──────────┐    ┌──────────┐
        │ MAX17048       │    │ MCP73831 │    │ USB-C    │
        │ Fuel gauge     │    │ Charger  │◄───┤ port     │
        └───────┬────────┘    └────┬─────┘    └──────────┘
                │ I2C              │ VBAT
                ▼                  ▼
        ┌───────────────────────────────┐
        │        LiPo cell (200 mAh)     │
        └───────────────────────────────┘
```

## Layered responsibilities

### Layer 1 — Air interface

Radio waves at 2.4 GHz. NULLWEAR is one of many devices in this band. Its only RF interaction is to receive BLE advertising packets and emit short jam pulses on the same channels.

### Layer 2 — Network core (nRF5340 cpunet)

Owns the RADIO peripheral. Responsible for the entire RX-detection-and-TX-corruption pipeline. Has no operating system in the conventional sense — uses Zephyr only as a thin scaffolding for IPC and minimal startup; the radio operations are bare-metal nrfx HAL.

Files:
- `firmware/nullwear-p/network_core/src/main.c` — minimal entry point, IPC server.
- `firmware/nullwear-p/network_core/src/radio_jammer.c` — the actual annihilation engine.
- `firmware/nullwear-p/network_core/src/radio_jammer.h` — public interface.

### Layer 3 — Application core (nRF5340 cpuapp)

Handles everything not RF-related. Standard Zephyr-based application code.

Files:
- `firmware/nullwear-p/src/main.c` — entry point, IPC client.
- `firmware/nullwear-p/src/battery_mgmt.c` — fuel gauge polling, charge state.
- `firmware/nullwear-p/src/led_driver.c` — RGB LED status indication.
- `firmware/nullwear-p/src/state_machine.c` — boot, run, idle, deep-sleep transitions.

### Layer 4 — Inter-core communication

Zephyr `ipc_service` over the Nordic IPC peripheral. App core polls network core for stats once per minute. Network core has no need to call into app core.

### Layer 5 — Power subsystem

Lithium-polymer battery, USB-C charge input, fuel gauge. The radio must run continuously off battery — power budget is the dominant engineering constraint.

### Layer 6 — User interface

A single button (long-press to deep-sleep, short-press to acknowledge), an RGB LED (battery status + occasional jam-event blue flicker), a USB-C port (charge + diagnostic).

### Layer 7 — Mechanical

Sealed PC-ABS enclosure, ultrasonic-welded, internally potted in polyurethane for shock isolation and IP67 rating. Belt clip / MOLLE-mount on rear.

---

## Data flow during a single annihilation event

Event: an Axon body camera ~5 m away broadcasts a BLE advertising packet on channel 37.

```
T = 0 µs:    Packet preamble starts on air.
T = 8:       Access address starts.
T = 40:      Access address complete. nRF5340 RADIO matches it.
              ADDRESS event fires.
              SHORTS chain: ADDRESS → BCSTART (bit counter starts).
T = 56:      16 bits of PDU header received.
T = 104:     64 bits of PDU received (header + full MAC AdvA).
              BCMATCH event fires.
              ISR runs: oui_matches_axon(s_rx_buf) returns true.
              launch_jam_pulse() executes:
                - TASKS_DISABLE issued
                - PACKETPTR set to s_jam_buf
                - SHORTS reconfigured for TX
T = 110:     Radio enters DISABLED state (~6 µs ramp-down).
              DPPI link s_dppi_dis_to_txen fires:
                  EVENTS_DISABLED → TASKS_TXEN
T = 150:     Radio in TXIDLE → TXSTART (~40 µs ramp-up).
              Jam pulse begins on the air.
T = 184:     Original packet's CRC region begins
              (assuming ~10 bytes AdvData).
T = 184–208: Both the original CRC and the jam pulse are on the air
              simultaneously. The receiver's bit decisions are random
              in this overlap window.
T = 208:     Original packet ends. Receiver computes CRC over the
              corrupted received bits. CRC FAILS.
              Receiver silently discards. Application layer (the
              attacker's scanner) sees no packet.
T = 230:     Jam pulse ends.
              Radio cycles back to RX.
              SHORTS re-armed.
              Resume scanning.
```

The entire event takes ~230 µs from packet preamble to next-packet-ready. Total energy expended: ~3 mA × 230 µs × 3.7 V = ~2.6 µJ per annihilation event.

A typical Axon device producing 30 packets/sec, 1/3 of which arrive while NULLWEAR is on the matching channel, generates ~10 annihilation events/sec → ~26 µJ/sec, negligible compared to the ~30 mW continuous draw of the rest of the device.

---

## Channel scanning strategy

The radio cannot listen on more than one channel at a time. NULLWEAR rotates through all three primary advertising channels (37, 38, 39), spending 80 ms on each.

```
t = 0 ms      ──► channel 37 ────────────► 80 ms
                                          │
t = 80        ──► channel 38 ────────────► 160 ms
                                          │
t = 160       ──► channel 39 ────────────► 240 ms
                                          │
t = 240       ──► (loop back to 37)
```

For a target that broadcasts every ~30 ms across all 3 channels (typical Axon behaviour), the per-channel hit rate during a single 80-ms dwell is approximately 80/30 × 1/3 ≈ 0.89 packets per dwell on the target channel. Across the full 240-ms cycle, expected packet capture rate is ~2.7 packets per second per Axon device.

This is sufficient to keep an attacker's dashboard cleared — losing 90% of an attacker's input over time renders even surviving detections uncorrelated and useless. For applications requiring near-100% capture, the dwell time can be reduced (at higher CPU load) or multiple NULLWEAR units worn with phase offsets.

---

## Failure tolerance

| If this fails | What happens |
|---|---|
| App core firmware crash | Radio keeps jamming (network core is independent). LED frozen. USB-CDC unresponsive. |
| Network core firmware crash | Radio stops. App core LED detects (no stats updates) and signals fault state. |
| Battery depleted | Both cores power off cleanly. Unit no longer operational. |
| USB-C disconnected during charge | Battery powers the device normally. No interruption. |
| Antenna damaged | RX sensitivity degrades. Field-test protocol detects via reduced PAR. |
| OUI matcher firmware bug | Either over-jams (would corrupt non-target traffic — caught by ATP Test 5) or under-jams (caught by ATP Test 4 and field testing). |

The decision to put the radio on a separate core from the app firmware is by design: it isolates the safety-critical function from the comparatively complex management logic. A bug in `battery_mgmt.c` cannot bring down the radio.

---

## Cross-references

- BLE protocol details: [`03-bluetooth-le-primer.md`](03-bluetooth-le-primer.md)
- The annihilation technique: [`04-ble-crc-corruption.md`](04-ble-crc-corruption.md)
- Hardware reference: [`05-hardware-spec.md`](05-hardware-spec.md)
- Firmware architecture detail: [`06-firmware-architecture.md`](06-firmware-architecture.md)
- Power subsystem: see Engineering Specification PDF, Part V.
- Mechanical: see Engineering Specification PDF, Part VI.
