# Troubleshooting

Diagnostic guide for the depot technician dealing with reported faults. Cross-reference [`09-operations-manual.md`](09-operations-manual.md) §7 for the user-facing fault flow.

---

## 1. Diagnostic via USB-CDC

Every NULLWEAR unit exposes a USB-CDC serial interface when connected to a host. To use it:

1. Plug the unit into a depot diagnostic computer via USB-C.
2. The unit enumerates as a USB-CDC device. On Linux it appears as `/dev/ttyACMx`. On Windows, as a COM port. On macOS, as `/dev/tty.usbmodemxxxx`.
3. Open a terminal at **115200 baud, 8N1**:
   ```bash
   screen /dev/ttyACM0 115200
   ```
   or use `minicom`, `picocom`, PuTTY, etc.
4. Press Enter. The unit responds with a `>` prompt.

### Available diagnostic commands

| Command | Output |
|---|---|
| `?` or `help` | Lists available commands |
| `version` | Firmware revision, build SHA, build date |
| `stats` | Lifetime statistics: pkts received, pkts OUI-matched, pkts jammed, channel hops, errors |
| `bat` | Battery voltage, SoC, charge cycles, last charge time |
| `radio` | Radio peripheral state, current channel, last received MAC |
| `dump-faults` | Last 32 fault log entries from NVM |
| `clear-faults` | Erase the fault log (depot use only — preserves a snapshot) |
| `selftest` | Run an abbreviated self-test (~10 s) |
| `reset` | Soft reset |
| `dfu` | Enter DFU mode for firmware update |

---

## 2. Symptom → Diagnosis table

| Reported / observed symptom | Likely cause | Action |
|---|---|---|
| LED off entirely after charging | Battery deeply discharged below recovery voltage; OR fuel-gauge fault | Connect to bench USB; if charge current draws normally for 30 minutes and unit boots, monitor; if not, RMA. |
| LED rapid red blink | Battery temperature out of range, or fuel gauge reports fault | Allow to thermally stabilise (~30 min in ambient); if persists, RMA. |
| LED flickers red occasionally | Brown-out resets (battery weak under load) | Battery aged or PCB power filtering fault; RMA. |
| LED green but no annihilation observed in field test | Radio peripheral not initialised; OR antenna broken; OR firmware mismatch | Run `selftest`; check `stats` for non-zero `pkts_jammed` over 60 s with a known source nearby; if zero, RMA. |
| Annihilation works at very close range but PAR drops at 5 m | Antenna detuned (mechanical damage to PCB or chip antenna); OR insufficient TX power | Visually inspect antenna keepout area; check `radio` for TXPOWER setting; if hardware OK and software setting correct, RMA. |
| `stats` shows high `pkts_oui_matched` but low `pkts_jammed` | TX timing is missing the corruption window; OR DPPI link broken | Likely firmware or silicon issue — RMA for replacement, escalate to engineering team. |
| `stats` shows high `errors_tx_late` | RX→TX timing too tight; minimum-length packets slipping through | Firmware tuning required; report to engineering; not a unit-RMA condition unless field PAR < 0.95. |
| Unit boots but immediately resets every few seconds | Watchdog tripping; OR fuel gauge unreachable; OR low-battery brown-out | Read `dump-faults` to identify; usually battery — replace via refurbishment. |
| Unit fails IP67 retest after drop | Enclosure seal compromised | RMA. |
| USB-C connector loose / not charging | Mechanical damage to receptacle | RMA. |
| LED is amber but battery diagnosis says SoC > 80% | LED driver fault | RMA. |
| Multiple units in the same dock fail simultaneously | Dock fault, not unit fault | Test units on a different dock; if they pass, the original dock is faulty. |

---

## 3. Common engineering escalations

### "All units are reporting `errors_tx_late`"

The RX→TX transition timing has insufficient margin for the typical Axon advertisement length being seen in the field. Possible causes:

- DPPI link not configured at boot (boot-order bug in network-core init).
- Higher-priority interrupt is pre-empting the radio ISR.
- Specific BLE PHY mode mismatch (e.g. 2M PHY packets, which transmit twice as fast and halve the timing budget).

Engineering action: capture a logic-analyser trace of the RXMATCH event and the TXEN task on a scope. Confirm the gap is within Nordic's specified bounds. If the gap is consistently > 50 µs, the DPPI link is degraded.

### "Some units pass ATP but fail field test"

ATP runs in a controlled environment. Field test runs in real RF conditions. Two common causes:

1. **Antenna orientation sensitivity** — the chip antenna has a non-isotropic radiation pattern. If the unit is worn LED-down or in a particular pocket orientation, effective TX coverage drops in some directions. Mitigation: verify in field-test orientations §4 and report back to engineering for antenna placement review.

2. **High-RF environment desensitisation** — the receiver's LNA can be desensitised by strong adjacent-channel emissions (e.g. a Wi-Fi router in close range). NULLWEAR fails to detect packets it should be jamming. Mitigation: verify with `stats` whether `pkts_received_total` is significantly lower in the high-RF environment than in a quiet one.

### "ESP32 emulator not visible to the receiver"

If you can't see `00:25:DF:DE:AD:BE` from a 1-m baseline scan, the emulator is not transmitting. Confirm:

- ESP32 board is powered (LED on board lit).
- Serial console shows the heartbeat `[N] still advertising 00:25:DF:DE:AD:BE`.
- No firmware upload error.
- ESP32 BLE SDK version compatible with `esp_base_mac_addr_set()`.

If everything looks correct on the emulator side, the receiver may have a Bluetooth driver issue. Try a different BLE adapter or a different host.

---

## 4. Firmware update via DFU

If a unit needs a firmware update (e.g. bug fix or feature update from engineering):

1. Connect via USB-CDC.
2. Issue `dfu` command. The unit confirms and reboots into MCUboot.
3. From the depot host, run:
   ```bash
   mcumgr --conntype serial --connstring="dev=/dev/ttyACM0,baud=115200" \
          image upload nullwear_p_signed.bin
   mcumgr --conntype serial --connstring="dev=/dev/ttyACM0,baud=115200" \
          image confirm
   mcumgr --conntype serial --connstring="dev=/dev/ttyACM0,baud=115200" \
          reset
   ```
4. After reboot, verify the firmware version with `version`.
5. Re-run abbreviated ATP (Tests 2, 4) to confirm functional.

Only signed images will be accepted. If the bootloader rejects the image, check the signing key.

---

## 5. When to escalate beyond depot

Escalate to engineering for:

- More than 5% of units in a single batch failing ATP.
- Any unit failing in service in a way not on the table above.
- Repeatable failure that no firmware update or refurbishment fixes.
- Apparent attacker adaptation (e.g. systematic field-test failures in a particular geography or environment).

Engineering contact: see the supply contract.

---

## 6. Logging

All diagnostic actions taken at depot level should be recorded in the per-unit asset record:

- Date and time.
- Technician identifier.
- Symptom reported.
- Diagnostic steps taken.
- Outcome (resolved / RMA / refurbishment).

This record is used for warranty management with the contract manufacturer and for supply-side root-cause analysis.
