
<p align="center">
  <img src="https://github.com/jakka351/Project-Nullwear/blob/main/docs/img/logo.png" alt="Project NULLWEAR" width="800"/>
</p>
  
# NULLWEAR Firmware

Firmware sources and verification tools.

## Layout

```
firmware/
├── README.md                  # this file
├── nullwear-p/                # personal-issue device firmware
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── boards/                # devicetree overlays
│   ├── src/                   # application core
│   └── network_core/          # radio / network core sub-image
└── tools/
    ├── reference-receiver/    # Python verification tool
    └── test-source/           # ESP32-based Axon BLE source emulator
```

## Build

See `../docs/07-build-instructions.md` for end-to-end build steps. Quick version:

```bash
# In an NCS workspace:
cd nullwear-p
west build -b nrf5340dk_nrf5340_cpuapp -p always
west flash --erase
```

## Verify

See `../docs/10-field-testing-protocol.md` for the full verification protocol. Quick version:

1. Flash a NULLWEAR/P unit (or DK).
2. Flash the ESP32 emulator (`tools/test-source/axon_emulator.ino`).
3. Run the reference receiver:
   ```bash
   cd tools/reference-receiver
   pip install bleak
   python ref_receiver.py --duration 60
   ```
4. Place the NULLWEAR unit between the ESP32 source and the receiver. Re-run with the baseline reception count. Expect PAR ≥ 0.99.

## Variants

The firmware tree currently contains only NULLWEAR/P (personal-issue) sources. NULLWEAR/V and NULLWEAR/S share the same source core; their board definitions and minor variant-specific files will be added under `nullwear-v/` and `nullwear-s/` once the pilot board designs are finalised. The radio jammer code is identical across all variants.

## Source code rights

MIT licensed (see `../LICENSE`). Use freely. Build with whichever toolchain you prefer. Ship to whichever agency needs it.
