# Firmware Architecture

How the firmware is structured. Read [`02-architecture.md`](02-architecture.md) first for the system view.

---

## Two-image, two-core layout

The nRF5340 has two ARM Cortex-M33 cores. NULLWEAR uses both:

```
┌──────────────────────────────────────────────────────────┐
│   APPLICATION CORE (cpuapp) — 128 MHz, 1 MB Flash, 512 kB RAM
│
│   image: nullwear_p_app_signed.hex
│   role:  battery, LED, button, USB-CDC diagnostic, IPC client
│
│   FLASH:
│     0x00000000 - 0x0000C000  MCUboot bootloader (signed)
│     0x0000C000 - 0x000F0000  Application image slot 0 (active)
│     0x000F0000 - 0x000F8000  Application image slot 1 (OTA staging)
│     0x000F8000 - 0x00100000  Settings + non-volatile event log
│
│   USES Zephyr RTOS for: thread scheduling, drivers, I²C, PWM, USB,
│   logging, IPC service, MCUboot.
│
└──────────────────────────────────────────────────────────┘
                              │
                  IPC service │ shared memory + mailbox
                              │
┌──────────────────────────────────────────────────────────┐
│   NETWORK CORE (cpunet) — 64 MHz, 256 kB Flash, 64 kB RAM
│
│   image: nullwear_p_net_signed.hex
│   role:  RADIO peripheral, OUI matcher, jam pulse, channel hop
│
│   FLASH:
│     0x01000000 - 0x01040000  Radio firmware
│
│   USES Zephyr only as a thin scaffold for IPC and startup.
│   The actual radio operations are bare-metal nrfx HAL with direct
│   PPI/DPPI hardware-event chaining. The scheduler is not in the
│   critical path of any radio event.
└──────────────────────────────────────────────────────────┘
```

This separation is by design. The radio core is safety-critical: it is the load-bearing element of NULLWEAR. The application core is comparatively complex and updateable. Putting them on separate cores means a fault in the application code cannot bring down the radio.

---

## Source tree

```
firmware/nullwear-p/
├── CMakeLists.txt                       # app-core build
├── prj.conf                             # app-core Kconfig
├── boards/
│   └── nrf5340dk_nrf5340_cpuapp.overlay # devicetree overlay (DK)
├── src/                                 # app core sources
│   ├── nullwear.h                       # shared definitions
│   ├── main.c                           # app entry point + IPC client
│   ├── battery_mgmt.c                   # MAX17048, charge state
│   ├── led_driver.c                     # RGB PWM LED + status
│   └── state_machine.c                  # boot/run/idle/sleep
└── network_core/                        # net core sub-image
    ├── CMakeLists.txt
    ├── prj.conf
    └── src/
        ├── main.c                       # net entry + IPC server
        ├── radio_jammer.h               # public interface
        └── radio_jammer.c               # the actual annihilation engine
```

---

## Network-core firmware in detail

`firmware/nullwear-p/network_core/src/radio_jammer.c` is the load-bearing file. It contains:

### Hardware initialisation

- Configures the RADIO peripheral for BLE 1 Mbps PHY.
- Sets the access address to `0x8E89BED6` (BLE primary advertising).
- Configures the BLE CRC parameters: 24-bit, init `0x555555`, polynomial `0x0000065B`.
- Configures the bit counter (`BCC = 64`) to fire after the full 64-bit (header + AdvA) prefix has been received.
- Sets up DPPI channels for hardware event chaining:
  - `EVENTS_DISABLED → TASKS_TXEN` (zero-CPU-mediation RX→TX transition).

### Channel hopping

- TIMER0 fires every 80 ms.
- ISR cleanly disables radio, advances channel index, re-arms RX on next channel.

### Detection and corruption pipeline

```c
if (NRF_RADIO->EVENTS_BCMATCH) {
    // 64 bits of PDU received. Check the OUI.
    if (oui_matches_axon(s_rx_buf)) {
        launch_jam_pulse();   // disables RX, triggers TX via DPPI
    }
}
```

### Statistics

A small struct of `pkts_received_total`, `pkts_oui_matched`, `pkts_jammed`, `errors_tx_late`, `channel_hops` is updated atomically and exposed via IPC to the app core.

---

## Application-core firmware in detail

### `main.c`

- Initialises LED, battery management, IPC.
- Once a minute, requests a stats snapshot from the network core and logs it.
- Sleeps the rest of the time.

### `battery_mgmt.c`

- I²C-poll the MAX17048 fuel gauge every 60 s.
- Compute battery state tier (`BAT_OK` / `BAT_LOW` / `BAT_CRIT` / `BAT_CHG` / `BAT_FAULT`).
- Detect critical undervoltage; trigger ordered shutdown.
- Detect charging via the MCP73831 `STAT` GPIO.

### `led_driver.c`

- PWM-driven RGB LED.
- Background thread reads battery state and updates LED pattern accordingly.
- LED breathing pattern when charging.
- LED rapid-blink when fault.

### `state_machine.c`

- Top-level lifecycle: `BOOT` → `RUN` → `IDLE` (low-power) → `DEEP_SLEEP` (USB-detached, manual) → `FAULT`.
- Button handler: short-press = acknowledge; long-press = enter deep sleep.

### USB-CDC diagnostic (TBD)

A small command parser exposes the diagnostic commands listed in [`11-troubleshooting.md`](11-troubleshooting.md). To be added in a follow-up commit; placeholder in `state_machine.c`.

---

## Concurrency model

Network core: pure interrupt-driven. No threads. The radio ISR and the timer ISR are the only execution contexts. They cannot pre-empt each other (same priority).

Application core: Zephyr-managed cooperative threads:

| Thread | Priority | Stack | Purpose |
|---|---|---|---|
| `main` | 0 | 4 kB | Init + IPC stats poll |
| `battery_poll_tid` | 14 | 1 kB | Periodic fuel-gauge read |
| `led_tid` | 12 | 1 kB | LED state update |
| `state_tid` | 10 | 2 kB | Top-level state machine |

Lower priority number = higher priority (Zephyr convention). The sleep paths are well-tested; total CPU utilisation under typical load is < 1%.

---

## Build configuration

The app-core image enables:

- `CONFIG_LOG=y` with deferred mode and RTT backend.
- `CONFIG_I2C=y` for the fuel gauge.
- `CONFIG_PWM=y` for the LED.
- `CONFIG_GPIO=y` for the button and charger STAT.
- `CONFIG_IPC_SERVICE=y` for inter-core communication.
- `CONFIG_USB_DEVICE_STACK=y` for USB-CDC diagnostic.
- `CONFIG_BOOTLOADER_MCUBOOT=y` for signed updates.
- `CONFIG_BT=n` — Bluetooth host stack not used; we use the radio raw.
- `CONFIG_PM=y` for power management.

The net-core image enables:

- `CONFIG_NRFX_DPPI=y` and `CONFIG_NRFX_TIMER0=y` for raw HAL access.
- `CONFIG_BT=n` and `CONFIG_BT_CTLR=n` — no Bluetooth Controller; we own the radio directly.
- Deferred logging via RTT only.

---

## Memory budget

| Item | App core | Net core |
|---|---|---|
| MCUboot | 48 kB Flash | n/a |
| Application | ~72 kB Flash, ~24 kB RAM | ~32 kB Flash, ~12 kB RAM |
| Settings + log | 32 kB Flash | n/a |
| Free | ~872 kB Flash, ~488 kB RAM | ~224 kB Flash, ~52 kB RAM |

Plenty of headroom on both cores. The current design uses a small fraction of the silicon's capability — there is room to add features (e.g. multi-OUI support, USB-CDC command parser, DFU-over-CDC) without architectural change.

---

## Updateability

Firmware updates are over USB-CDC only — there is no over-the-air update path. This is by design: it eliminates a remote-attack surface against a security-relevant device.

The MCUboot bootloader on the app core verifies signed images before booting. The net-core image is provisioned during the same flash cycle as the app-core image; both are atomically updated.

Updating in the field requires the depot to physically connect each unit via USB-C. The expected lifetime cadence of firmware updates is approximately one per year.

---

## Cross-references

- BLE protocol fundamentals: [`03-bluetooth-le-primer.md`](03-bluetooth-le-primer.md)
- The annihilation technique: [`04-ble-crc-corruption.md`](04-ble-crc-corruption.md)
- Build instructions: [`07-build-instructions.md`](07-build-instructions.md)
- Hardware reference: [`05-hardware-spec.md`](05-hardware-spec.md)
- Acceptance procedure: [`12-acceptance-test-procedure.md`](12-acceptance-test-procedure.md)
