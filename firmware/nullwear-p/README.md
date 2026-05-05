# NULLWEAR/P firmware

Personal-issue device firmware. Two-image build: app core + network core.

## Files

| Path | Purpose |
|---|---|
| `CMakeLists.txt` | App-core build. Declares the network-core sub-image. |
| `prj.conf` | App-core Kconfig. |
| `boards/nrf5340dk_nrf5340_cpuapp.overlay` | Devicetree overlay for the Nordic nRF5340 DK. |
| `src/main.c` | App-core entry point. IPC client. Stats polling. |
| `src/nullwear.h` | Shared definitions. |
| `src/battery_mgmt.c` | MAX17048 fuel-gauge polling, charge-state detection. |
| `src/led_driver.c` | RGB-LED PWM driver, state-driven LED patterns. |
| `src/state_machine.c` | Top-level lifecycle (boot / run / idle / deep-sleep / fault). |
| `network_core/CMakeLists.txt` | Network-core build. |
| `network_core/prj.conf` | Network-core Kconfig. |
| `network_core/src/main.c` | Network-core entry point. IPC server. |
| `network_core/src/radio_jammer.h` | Public interface for the jammer module. |
| `network_core/src/radio_jammer.c` | **The actual annihilation engine.** |

## Architecture

See `../../docs/06-firmware-architecture.md` for full detail. Summary:

- App core (Cortex-M33 @ 128 MHz) runs Zephyr; handles battery, LED, button, USB-CDC, IPC.
- Network core (Cortex-M33 @ 64 MHz) owns the RADIO peripheral; runs an interrupt-driven jammer loop with no scheduler in the critical path.
- The two cores communicate via Zephyr `ipc_service` over Nordic's IPC peripheral.

## Build

```bash
west build -b nrf5340dk_nrf5340_cpuapp -p always
west flash --erase
```

For the production board, substitute `nrf5340dk_nrf5340_cpuapp` with `nullwear_p_cpuapp` (board definition pending pilot CM).

## Status

| Component | Status |
|---|---|
| Network-core radio jammer | Reference implementation written from first principles per Nordic nrfx HAL and BLE 5.x specification. **Awaiting test-compile and bench-test verification.** |
| App-core firmware | Reference implementation written against Zephyr v3.4 patterns. **Awaiting test-compile.** |
| Build files | Reference build files for NCS v2.5+. **Awaiting test-build.** |
| Devicetree overlays | DK overlay only; production board pending. |

The author has not personally test-compiled this firmware against a current NCS install. Treat it as engineering reference rather than tested binary. The first task of the pilot CM is to:

1. Stand up an NCS v2.5+ workspace.
2. Test-build per `../../docs/07-build-instructions.md`.
3. Resolve any API drift (Zephyr APIs evolve; minor adjustments for newer NCS releases may be required).
4. Flash to a Nordic nRF5340 DK.
5. Verify the basic boot and IPC chain works.
6. Run lab acceptance per `../../docs/12-acceptance-test-procedure.md` against an ESP32-based test source.
7. Report findings — both successes and any gaps — back to the maintainer.

This honesty matters: every claim in the reference implementation is technically grounded but not yet field-validated. The verification protocol exists precisely so that no agency takes the firmware on faith.
