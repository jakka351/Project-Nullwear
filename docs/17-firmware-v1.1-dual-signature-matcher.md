# Firmware v1.1 — Dual-Signature Matcher

**Status:** specification + reference implementation merged into `firmware/nullwear-p/network_core/src/`. Hostile-reviewer audit performed; current revision is **v1.1.1** (see "Audit fixes" below). Awaiting test-compile and lab validation per the existing ATP and field-test protocols.

**Change category:** functional enhancement, backward-compatible, build-time toggle to revert to v1.0 behaviour.

## Audit fixes applied in v1.1.1

A first-pass code review by the maintainer (acting as hostile reviewer against the maintainer's own work) caught the following defects in the initial v1.1 firmware. All were fixed before publication:

1. **`RADIO_0_IRQHandler` / `TIMER0_IRQHandler` would never fire.** Bare ISR symbols are not picked up by Zephyr — the kernel installs its own vector table. Fixed by using `IRQ_DIRECT_CONNECT(RADIO_IRQn, …)` for the latency-critical RADIO ISR and `IRQ_CONNECT(TIMER1_IRQn, …)` for the slow channel-hop timer.
2. **Wrong IRQ name.** `RADIO_0_IRQn` is for chips with multiple radios; nRF5340 cpunet has a single `RADIO_IRQn`.
3. **TIMER0 collides with Zephyr's tickless kernel.** Hop timer moved to TIMER1.
4. **DPPI link `DISABLED → TXEN` fires on every `DISABLED`** including normal RX-end and channel-hop forced disable, which would cause accidental TX bursts. DPPI dropped entirely. Replaced with synchronous spin-wait inside `launch_jam_pulse()` (~10 µs added latency, still well inside the empirically-validated CRC-corruption window).
5. **`nrfx_dppi_channel_alloc()` API takes an instance pointer in nrfx ≥ 3.0** (NCS 2.5+). Moot now that DPPI is dropped.
6. **`radio_arm_rx()` did not restore RX-mode SHORTS** after `launch_jam_pulse()` overwrote them with TX-mode SHORTS — the next RX would not trigger BCMATCH. Fixed.
7. **`radio_arm_rx()` did not clear stale events** that could fire spuriously. Fixed.
8. **Channel-hop ISR raced the DISABLED ISR** with both trying to re-arm. Fixed by making the DISABLED ISR the single point of re-arm; the hop ISR just sets the new channel and triggers `TASKS_DISABLE`.

The v1.1 design intent (dual-signature matching, two-phase BCMATCH) is unchanged. Only the implementation hygiene improved.

---

## Why v1.1 exists

Empirical analysis of 3.4 years of in-the-wild Axon BLE telemetry (see *Axon BLE — Threat Validation Report*, companion PDF) surfaced a class of Axon advertisement that the v1.0 OUI matcher does not catch:

> Some Axon devices broadcast with a **sanitised AdvA** (typically `00:00:00:00:00:00`) while still carrying Axon-identifying data in the BLE Service Data field. This is consistent with the device being in a **non-deployed / docked / charging state** — characteristic of body cameras and Tasers sitting in dock at a police station.

For such packets the on-air MAC is not `00:25:DF:XX:YY:ZZ`, so v1.0's OUI-based matcher (`pdu[5..7] == { 0xDF, 0x25, 0x00 }`) returns false and the packet is not jammed.

This is a small operational gap — docked devices are inside police stations, not on patrolling officers, and the strategic threat model is officer tracking by external attackers rather than dock-side fingerprinting. **But the gap is fixable cheaply, and closing it makes NULLWEAR coverage complete across all observed Axon broadcast modes.**

## What v1.1 adds

A **second matching path** that runs only when the v1.0 OUI check misses. The second path looks for the BLE 16-bit Service Data UUID `0xFE6B`, which is registered with the Bluetooth SIG to **Axon Public Safety** and is present in every Axon advertisement observed to date — both deployed-mode and docked-mode.

Match logic, pseudocode:

```
on every received BLE advertising packet:
    if MAC_OUI == 0x00:0x25:0xDF:               // Phase 1 (v1.0 behaviour)
        jam()
    else if AdvData contains AD-type 0x16 with UUID 0x6B 0xFE:   // Phase 2 (NEW)
        jam()
    else:
        do nothing (packet is not Axon)
```

If the OUI matches, the device is unambiguously Axon and we jam at the v1.0 timing. If the OUI doesn't match, we wait a further 64 bits and inspect the start of AdvData for the Axon service-UUID signature.

## Implementation

### Hardware events

The Nordic nRF5340 RADIO peripheral's bit counter (BCC / BCMATCH) is re-armable — after a BCMATCH fires for the first phase, software can write a new BCC value and wait for the second BCMATCH on the same packet. This lets the matcher run in two stages on one received PDU without missing the on-air timing budget.

| Phase | BCC | Time-on-air | What it inspects |
|---|---|---|---|
| 1 (OUI) | 64 bits | T+104 µs from preamble | `pdu[5..7]` against `0x00 0x25 0xDF` |
| 2 (UUID) | 128 bits | T+168 µs | `pdu[8..14]` for `0x16 0x6B 0xFE` 3-byte sequence |

### Timing budget

Empirical PDU lengths from the Threat Validation Report:

| Percentile | Total PDU | CRC region |
|---|---|---|
| p10 | 352 µs | 328–352 µs |
| p50 | 352 µs | 328–352 µs |
| p90 | 368 µs | 344–368 µs |

Phase-1 jam start: ~115 µs after preamble. Phase-2 jam start: ~180 µs.
Jam-pulse duration (38-byte payload): 304 µs.

Both jam-start times leave the jam pulse comfortably overlapping the CRC region of every observed Axon advertisement. **Verified for both phases against the same 46-device empirical dataset that validated v1.0.**

### Code layout

Single file changed: `firmware/nullwear-p/network_core/src/radio_jammer.c`. Header `radio_jammer.h` now defines:

```c
#define NULLWEAR_AXON_SERVICE_UUID_LO    0x6BU
#define NULLWEAR_AXON_SERVICE_UUID_HI    0xFEU
#define NULLWEAR_AD_TYPE_SERVICE_DATA_16BIT  0x16U
#define NULLWEAR_BCC_PHASE_OUI            64U
#define NULLWEAR_BCC_PHASE_UUID          128U
#define NULLWEAR_ENABLE_PHASE_2            1   /* set to 0 to revert to v1.0 */
```

The `nullwear_radio_stats_t` struct gains a new counter:

```c
uint32_t pkts_uuid_matched;   /* Phase 2 hits — docked/privacy-mode Axon kit */
```

The two-phase ISR is approximately 30 lines longer than the v1.0 single-phase ISR.

### Backward compatibility

Setting `NULLWEAR_ENABLE_PHASE_2 = 0` at build time disables Phase 2 entirely and restores exact v1.0 matching behaviour. This is the recommended fall-back if Phase 2 surfaces any field issue (e.g. unexpected non-Axon false positives).

## Why this is the right architecture

The two-phase design has three properties that matter:

1. **Fast path stays fast.** Deployed Axon equipment matches in Phase 1 with no additional latency relative to v1.0. Battery cost for the OUI hit path is unchanged.
2. **Slow path is a strict superset.** A Phase-2 hit is only possible if Phase 1 missed, so we never jam the same packet twice. The decision is monotonic.
3. **Selectivity is preserved.** The `0xFE6B` UUID is SIG-registered to Axon. A non-Axon device using `0xFE6B` would be a registry conflict, not a NULLWEAR design defect. False positives against unrelated traffic remain near-zero.

## What v1.1 does NOT do

- It does not match on the 128-bit UUIDs (`METROPOLISDEVICE` / `9ec5d2b8…`). Those are present too and could be matched, but matching on 16 bits is faster and the 16-bit `0xFE6B` is also present on every observed Axon packet, so the 128-bit check would be redundant cost.
- It does not attempt to parse the Service Data **payload** (the 18 bytes following the UUID). That payload is reverse-engineered separately — see *Service-Data Payload Analysis* (companion document).
- It does not extend coverage to Axon devices that broadcast with NEITHER the OUI NOR the `0xFE6B` UUID. No such devices were observed in 3.4 years of telemetry, but if Axon ever ships a third broadcast variant a v1.2 matcher would be needed.

## Validation requirements before shipping v1.1

This change is backward-compatible at the machine-code level (same RADIO peripheral configuration, additional ISR branches) and shares all test infrastructure with v1.0. To certify v1.1 for operational use, the existing test gates apply with one addition:

- **All v1.0 ATP and field-test acceptance criteria must pass.** No regression.
- **One new ATP measurement:** with NULLWEAR/P active and the ESP32 emulator broadcasting in two configurations (OUI-mode and zero-MAC + FE6B-mode), confirm that BOTH packet variants are annihilated with PAR ≥ 0.99. The ESP32 emulator firmware needs a corresponding update to support the zero-MAC + service-data mode — added to the test harness in the same release.
- **Counter-check:** confirm the new `pkts_uuid_matched` counter is non-zero after a controlled Phase-2 test, and zero in environments with no Phase-2 traffic.

## Risk

| Risk | Likelihood | Mitigation |
|---|---|---|
| Phase-2 BCMATCH timing too tight on minimum-length advertisements | Low — empirical p10 PDU is 352 µs, jam-pulse end at 484 µs covers it | Build-time toggle to disable; v1.0 OUI-only path always available |
| False positive against a future non-Axon SIG `0xFE6B` user | Very low — UUID is SIG-registered to Axon | Same as v1.0 OUI false-positive risk: design-tolerable, monitored via `pkts_uuid_matched` counter |
| Battery impact from extra ISR branches | Negligible — Phase 2 only runs on OUI miss; ISR adds < 100 cycles | None required |
| Field defect surfaces during pilot | Possible | Fast revert path: rebuild with `NULLWEAR_ENABLE_PHASE_2=0` |

## Schedule

If v1.0 pilot is in flight, fold v1.1 into the next firmware revision after pilot data confirms v1.0's PAR. Don't ship v1.1 ahead of v1.0 validation — keep the proven-good path live until v1.1 has its own bench acceptance behind it.

## Cross-references

- v1.0 source: same files, see git history.
- BLE primer: [`docs/03-bluetooth-le-primer.md`](03-bluetooth-le-primer.md).
- Annihilation theory: [`docs/04-ble-crc-corruption.md`](04-ble-crc-corruption.md).
- Firmware architecture: [`docs/06-firmware-architecture.md`](06-firmware-architecture.md).
- Validation evidence: companion PDF *Axon BLE Vulnerability — Threat Validation Report*.
- Service-Data payload analysis: [`docs/18-service-data-payload-analysis.md`](18-service-data-payload-analysis.md).
- Bluetooth SIG 16-bit UUID registry: https://www.bluetooth.com/specifications/assigned-numbers/
