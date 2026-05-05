# Build Instructions

End-to-end instructions for compiling and flashing the NULLWEAR reference firmware. Assumes you are starting from a clean development workstation.

---

## What you'll have at the end

A built `.hex` (or `.zip`) image you can flash onto a Nordic nRF5340 dev kit, which immediately starts running the NULLWEAR radio jammer. From there you can run the field-testing protocol against it.

---

## Required hardware

- **Nordic nRF5340 DK** (`PCA10095`) — for development and pilot. Produced by Nordic Semiconductor; available from Mouser, Digi-Key, Element14. Cost ~AUD 80.
- **USB-C cable** (for the DK's onboard J-Link debugger).
- **(Optional)** A Nordic nRF52840 Dongle — useful as a reference receiver alternative.

For a production board (`nullwear_p`), use the manufactured PCB and skip the DK.

---

## Required software

| Component | Version | Source |
|---|---|---|
| **Operating system** | Ubuntu 22.04 LTS, macOS 13+, or Windows 11 | — |
| **nRF Connect SDK (NCS)** | v2.5.0 or newer | https://www.nordicsemi.com/Products/Development-software/nrf-connect-sdk |
| **Zephyr** | v3.4 or newer (bundled with NCS) | bundled |
| **GNU ARM Embedded Toolchain** | 12.2 or newer | https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads |
| **west** | latest | bundled |
| **CMake** | 3.20+ | system package manager |
| **nrfjprog / nrf-command-line-tools** | latest | https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools |
| **Python** | 3.10+ | system package manager |
| **VS Code + nRF Connect for VS Code extension** (recommended GUI) | latest | https://marketplace.visualstudio.com/items?itemName=nordic-semiconductor.nrf-connect |

---

## Setup (one-off)

The cleanest path is the **nRF Connect for Desktop** installer. It manages NCS, Zephyr, the toolchain, and `west` together. Recommended.

If you prefer the manual path:

```bash
# Install west and the bootstrapping tools (Linux/macOS)
pip install west

# Initialise an NCS workspace
west init -m https://github.com/nrfconnect/sdk-nrf --mr v2.5-branch ncs
cd ncs
west update
west zephyr-export
pip install -r zephyr/scripts/requirements.txt
pip install -r nrf/scripts/requirements.txt
pip install -r bootloader/mcuboot/scripts/requirements.txt

# Confirm
west boards | grep nrf5340dk
```

You should see `nrf5340dk_nrf5340_cpuapp` and `nrf5340dk_nrf5340_cpunet` listed.

---

## Building NULLWEAR/P

From a fresh terminal with the NCS environment sourced:

```bash
# Clone or copy this project into the NCS workspace
cd $NCS_WORKSPACE
cp -r /path/to/Project-Nullwear/firmware/nullwear-p ./

# Build for the nRF5340 DK
cd nullwear-p
west build -b nrf5340dk_nrf5340_cpuapp -p always
```

Expected output:

```
-- Configuring done
-- Generating done
-- Build files have been written to: .../build
[2/95] Building C object zephyr/CMakeFiles/zephyr.dir/...
...
[95/95] Linking C executable zephyr/zephyr.elf
Memory region    Used Size  Region Size  %age Used
        FLASH:        72 KB     1024 KB      7.0%
         RAM:        24 KB      512 KB      4.7%
```

The build also implicitly builds the **network-core** image because `CMakeLists.txt` declares it via `NRF5340_CHILD_IMAGE_PATH`.

The output is at `build/zephyr/merged.hex` — a combined app-core + network-core image ready to flash.

---

## Flashing

With the nRF5340 DK plugged in via USB:

```bash
west flash --erase
```

This:
1. Erases the entire flash on both cores.
2. Writes the merged hex to flash.
3. Resets the chip.
4. The radio jammer starts immediately.

You should see the dev kit's LED1 (driven by the Zephyr LED-blink behaviour wired in our build) flash green-amber on power-on, then settle to dim green (battery state OK if powered from USB).

To watch the firmware logs:

```bash
west attach          # connects via J-Link RTT
# or, if RTT is configured:
JLinkRTTViewer       # GUI viewer
```

You should see boot output:

```
[00:00:00.001] <inf> nullwear_app: ==================================
[00:00:00.001] <inf> nullwear_app:   NULLWEAR/P firmware Rev A
[00:00:00.001] <inf> nullwear_app:   Build: ...
[00:00:00.001] <inf> nullwear_app: ==================================
[00:00:00.005] <inf> battery: MAX17048 detected, version 0x0012
[00:00:00.010] <inf> nullwear_app: Net-core IPC bound
[00:00:00.011] <inf> nullwear_net: NULLWEAR network-core firmware starting
[00:00:00.011] <inf> nullwear_net: Starting radio jammer; OUI target = 00:25:DF (Axon Enterprise)
```

If you see those lines and no errors, **the firmware has booted correctly**. The radio is now scanning advertising channels for the target OUI.

---

## Verifying the radio is actually scanning

This step proves NULLWEAR is doing radio work, before you confirm it's doing the *right* radio work.

In a separate terminal, run the reference receiver near the dev kit, and have a known BLE source nearby (an ESP32 running the test emulator):

```bash
cd firmware/tools/reference-receiver
python ref_receiver.py --duration 30
```

You should see the test source's `00:25:DF:DE:AD:BE` MAC appear with **dramatically reduced** packet count compared to baseline (i.e. with NULLWEAR's antenna powered down or the dev kit removed).

If baseline scan with no NULLWEAR shows hundreds of packets per minute and NULLWEAR-active scan shows < 10 packets per minute, **NULLWEAR is working**.

For a more rigorous measurement, follow [`10-field-testing-protocol.md`](10-field-testing-protocol.md).

---

## Building for the production board

Once a `nullwear_p` board definition exists in `boards/arm/nullwear_p/`:

```bash
west build -b nullwear_p_cpuapp -p always
```

The board definition encodes the production pinout — different from the DK — so the same firmware sources produce a different binary. The PCB pinout is documented in `pcb/nullwear-p/README.md`.

---

## Building a release image

For production-signed images:

1. Generate signing keys (one-time, store securely):

   ```bash
   west sign --tool imgtool -- --key root-ed25519.pem
   ```

2. Build with release configuration:

   ```bash
   west build -b nullwear_p_cpuapp -p always -- -DEXTRA_CONF_FILE=overlay-release.conf
   ```

3. The signed image at `build/zephyr/app_signed.hex` is the artefact you ship to the contract manufacturer for production flashing.

---

## Troubleshooting build failures

| Symptom | Likely cause | Fix |
|---|---|---|
| `error: 'NRF_RADIO' undeclared` | nrfx HAL missing in includes | Add `CONFIG_NRFX_DPPI=y`, `CONFIG_NRFX_TIMER0=y` to `network_core/prj.conf` |
| `cannot find -lnrfxlib` | NCS install incomplete | `west update` + `pip install -r requirements.txt` |
| `J-Link warning: Stuck in reset` | DK power cycle needed | Unplug USB, wait 5 s, replug |
| `Permission denied: /dev/ttyACMx` | Linux dialout group | `sudo usermod -aG dialout $USER` and re-login |
| Network core fails to boot | App-core image not provisioning the network core's image header | Use `west flash --erase` not just `--reset` |

For deeper failures: enable verbose builds with `west build -- -v`, check `build/CMakeFiles/CMakeOutput.log`.

---

## Cross-referencing

- Architecture: [`02-architecture.md`](02-architecture.md)
- BLE protocol fundamentals: [`03-bluetooth-le-primer.md`](03-bluetooth-le-primer.md)
- The technique itself: [`04-ble-crc-corruption.md`](04-ble-crc-corruption.md)
- Hardware reference: [`05-hardware-spec.md`](05-hardware-spec.md)
- Firmware structure: [`06-firmware-architecture.md`](06-firmware-architecture.md)
- Acceptance test: [`12-acceptance-test-procedure.md`](12-acceptance-test-procedure.md)
- Field test: [`10-field-testing-protocol.md`](10-field-testing-protocol.md)
