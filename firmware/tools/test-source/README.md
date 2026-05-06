<p align="center">
  <img src="https://github.com/jakka351/Project-Nullwear/blob/main/docs/img/logo.png" alt="Project NULLWEAR" width="800"/>
</p>


# Axon BLE Source Emulator (ESP32)

Test source for NULLWEAR verification. Turns any ESP32 dev board into a fake Axon BLE broadcaster, so you can verify NULLWEAR works without needing access to actual Axon equipment for lab testing.

## Why this exists

To verify that NULLWEAR is annihilating the right thing, you need a controlled BLE source emitting `00:25:DF`-prefixed advertising packets. There are two ways to get one:

1. Stand near a real Axon body camera and hope nobody minds.
2. Flash this sketch onto a $5 ESP32 dev board.

Option 2 is the right answer for lab testing. It has the additional benefit of using a fixed MAC (`00:25:DF:DE:AD:BE`) that you can pick out of the receiver's log unambiguously.

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
```

Drop `axon_emulator.ino` into `src/main.cpp` (rename), then:

```
pio run -t upload
pio device monitor
```

## What it does

- Sets the BLE MAC to `00:25:DF:DE:AD:BE` at boot.
- Starts BLE advertising in `ADV_NONCONN_IND` mode.
- Advertises every 30–40 ms (about 30 Hz), close to what real Axon body cameras do.
- TX power 0 dBm (default), giving you ~10–30 m range.
- Prints a heartbeat every 5 s on the serial port.

## Verifying it works (as a source)

Before testing NULLWEAR, confirm the emulator itself is broadcasting. From a separate device, run:

```bash
python ../reference-receiver/ref_receiver.py --duration 30
```

You should see `00:25:DF:DE:AD:BE` in the target list with a packet count of roughly 30 × 30 = 900 packets, ±50%.

## Safety / legal note

Because this emulator broadcasts an OUI registered to Axon Enterprise, in some jurisdictions running it outside a controlled environment could be construed as impersonating registered law-enforcement equipment. Restrict its use to:

- An RF-shielded room, or
- A physically isolated location well outside Axon equipment range, or
- A site for which you hold an experimental radio licence.

Always power it down between tests. Do **not** leave it broadcasting unattended in public.

License: MIT
