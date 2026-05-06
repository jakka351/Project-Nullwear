
<p align="center">
  <img src="https://github.com/jakka351/Project-Nullwear/blob/main/docs/img/logo.png" alt="Project NULLWEAR" width="800"/>
</p>
  
# Axon BLE Source Emulator (ESP32)

Test source for NULLWEAR verification. Turns any ESP32 dev board into a fake Axon BLE broadcaster, so you can verify NULLWEAR works without needing access to actual Axon equipment for lab testing.

The emulator emits the same advertisement shapes the v1.1 firmware and the reference receiver are looking for, across all three matching phases.

## Why this exists

To verify that NULLWEAR is annihilating the right thing, you need a controlled BLE source emitting Axon-shaped advertisements. There are two ways to get one:

1. Stand near a real Axon body camera and hope nobody minds.
2. Flash this sketch onto a $5 ESP32 dev board.

Option 2 is the right answer for lab testing.

## Modes

The emulator supports three modes, mirroring the receiver's three matching phases:

| Mode | Advertised MAC | FE6B service data | Receiver phase exercised |
|---|---|---|---|
| **`MODE_DEPLOYED`** | `00:25:DF:DE:AD:BE` (real Axon OUI prefix, distinctive suffix) | none | Phase 1 (OUI) only |
| **`MODE_DOCKED`**   | `00:00:00:00:00:00` (sanitised AdvA, with fallback to `02:00:00:00:00:00` if controller refuses the all-zero address) | 23-byte payload incl. synthetic 9-char serial `X60J0TST1` at bytes 14-22 | Phase 2 (UUID) + Phase 3 (sanitised MAC) |
| **`MODE_DUAL`** *(default)* | alternates between the above every 5 s (`DUAL_PERIOD_MS`) | toggled with the MAC | All three phases — single-flash end-to-end coverage |

**Default is `MODE_DUAL`** because it gives you complete coverage from one board flash. Override at compile time:

- *Arduino IDE:* edit the `#define NULLWEAR_TEST_MODE` line near the top of `axon_emulator.ino`.
- *PlatformIO:* add a build flag in `platformio.ini`:
  ```ini
  build_flags = -DNULLWEAR_TEST_MODE=MODE_DEPLOYED
  ```
  Other accepted values: `MODE_DOCKED`, `MODE_DUAL`. To change the alternation period in dual mode add `-DDUAL_PERIOD_MS=10000`.

The synthetic serial `X60J0TST1` is deliberately recognisable: any analyst inspecting a packet capture or the receiver's `axon_serials_recovered` list can immediately tell it came from the emulator and not a real Axon device.

## Hardware

Any ESP32 variant works. Tested suitable:

- ESP32 (original, 2016+)
- ESP32-S3
- ESP32-C3 (smallest, ~$3)

You'll need:

- ESP32 dev board with USB-C (or micro-USB)
- USB-C / micro-USB cable
- Any computer with the Arduino IDE or PlatformIO

## Build

### Arduino IDE 2.x

1. Install the **ESP32** board package:
   - File → Preferences → Additional Boards Manager URLs:
     `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - Tools → Board → Boards Manager → search "esp32" → Install (Espressif Systems)
2. Open `axon_emulator.ino`.
3. Tools → Board → ESP32 → select your board (e.g. *ESP32-C3 Dev Module*).
4. Tools → Port → select the COM port for your board.
5. Click **Upload**.

### PlatformIO (preferred)

In a fresh PlatformIO project:

```ini
[env:esp32-c3]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200
build_flags =
    -DNULLWEAR_TEST_MODE=MODE_DUAL
    ; -DDUAL_PERIOD_MS=5000
```

Drop `axon_emulator.ino` into `src/main.cpp` (rename), then:

```
pio run -t upload
pio device monitor
```

## What it does

- Sets the BLE base MAC to the configured value at boot, switching it live in `MODE_DUAL`.
- Starts BLE advertising in `ADV_NONCONN_IND` mode.
- Advertises every 30–40 ms (about 30 Hz), close to what real Axon body cameras do.
- TX power 0 dBm, giving you ~10–30 m range.
- In `MODE_DOCKED` / `MODE_DUAL`, attaches a 23-byte FE6B service-data payload containing the synthetic serial.
- Prints a heartbeat every 5 s on the serial port, including the current mode.

## Verifying it works (as a source)

Before testing NULLWEAR, confirm the emulator itself is broadcasting. From a separate device, run:

```bash
python ../reference-receiver/ref_receiver.py --duration 30
```

Expected output depending on mode:

| Mode | Expected receiver state |
|---|---|
| **`MODE_DEPLOYED`** | One target MAC `00:25:DF:DE:AD:BE`, kind = `OUI`, ~900 packets in 30 s. |
| **`MODE_DOCKED`**   | One target MAC `00:00:00:00:00:00` (or fallback `02:00:00:00:00:00`), kind = `UUID+SANITISED` (or `UUID` only with the fallback MAC), recovered serial `X60J0TST1`. |
| **`MODE_DUAL`**     | *Both* target MACs visible. Phase-hit counters all > 0. The `UUID-only` counter ≈ half of the UUID counter (every other 5 s window contributes). |

If you see fewer hits than expected — or only OUI hits in dual mode — re-flash with `MODE_DOCKED` to isolate the docked-mode path and check the receiver's CSV for `fe6b_payload_hex` populated rows.

## Reproducing v1.0 baseline

To verify against the v1.0 OUI-only firmware path (e.g. for regression baseline):

```bash
# Flash emulator with -DNULLWEAR_TEST_MODE=MODE_DEPLOYED
python ../reference-receiver/ref_receiver.py --no-phase2 --no-phase3 --duration 60
```

This restricts both source and receiver to Phase 1 only.

## Safety / legal note

Because this emulator broadcasts an OUI registered to Axon Enterprise *and/or* the FE6B Service UUID assigned to Axon Public Safety, in some jurisdictions running it outside a controlled environment could be construed as impersonating registered law-enforcement equipment. Restrict its use to:

- An RF-shielded room, or
- A physically isolated location well outside Axon equipment range, or
- A site for which you hold an experimental radio licence.

Always power it down between tests. Do **not** leave it broadcasting unattended in public.

License: MIT
