/*
 * Project NULLWEAR — Axon BLE source emulator
 *
 * Target: ESP32 (any variant — ESP32, ESP32-S3, ESP32-C3 all OK).
 * Toolchain: Arduino IDE 2.x with the ESP32 board package, OR
 *            PlatformIO (recommended for reproducible builds).
 *
 * SPDX-License-Identifier: MIT
 *
 * PURPOSE
 * =======
 *
 * Lab testing of NULLWEAR requires a BLE source that emits Axon-shaped
 * advertisements without requiring access to actual Axon hardware.
 * This sketch turns a $5 ESP32 dev board into exactly that source.
 *
 * It supports three modes, selectable at compile time, that mirror the
 * three matching phases the v1.1 firmware and the reference receiver
 * look for:
 *
 *   MODE_DEPLOYED  — In-service Axon broadcast.
 *                    MAC = 00:25:DF:DE:AD:BE  (OUI prefix +
 *                    distinctive suffix).  No FE6B service data.
 *                    Exercises Phase 1 (OUI match) ONLY.
 *
 *   MODE_DOCKED    — Docked / non-deployed Axon broadcast.
 *                    MAC = 00:00:00:00:00:00  (sanitised AdvA).
 *                    Includes BLE Service Data UUID 0xFE6B with a
 *                    representative payload containing a synthetic
 *                    cleartext serial in the same byte position as
 *                    real Axon Body 3 broadcasts (`X60J0TST1`,
 *                    bytes 14-22 of the FE6B payload).
 *                    Exercises Phase 2 (UUID) and Phase 3 (sanitised
 *                    MAC) — the path the v1.0 firmware would have
 *                    missed entirely.
 *
 *   MODE_DUAL      — Alternates between DEPLOYED and DOCKED every
 *                    DUAL_PERIOD_MS milliseconds (default 5000 ms).
 *                    Use this to verify both firmware match-paths in
 *                    a single test run.
 *
 * Default is MODE_DUAL because it gives end-to-end coverage in one
 * board flash. Override at build time:
 *
 *   ArduinoIDE: edit the #define NULLWEAR_TEST_MODE line below.
 *   PlatformIO: add `build_flags = -DNULLWEAR_TEST_MODE=MODE_DEPLOYED`
 *               (or MODE_DOCKED, or MODE_DUAL) to platformio.ini.
 *
 * Use it together with the reference receiver in
 *   ../reference-receiver/ref_receiver.py
 * to perform the verification described in
 *   docs/12-acceptance-test-procedure.md.
 *
 * IMPORTANT
 * =========
 *
 * Because this emulator broadcasts the 00:25:DF OUI and/or the FE6B
 * Service UUID assigned to Axon Public Safety, in some jurisdictions
 * running it could be construed as impersonating a piece of registered
 * law-enforcement equipment. Restrict its use to controlled lab
 * environments (an RF-shielded room or an isolated location well
 * outside Axon equipment range), and power it down between tests.
 */

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <BLEUUID.h>
#include "esp_mac.h"

// ---- Mode selection ------------------------------------------------

#define MODE_DEPLOYED   1
#define MODE_DOCKED     2
#define MODE_DUAL       3

#ifndef NULLWEAR_TEST_MODE
#define NULLWEAR_TEST_MODE  MODE_DUAL
#endif

#ifndef DUAL_PERIOD_MS
#define DUAL_PERIOD_MS  5000   // alternate every 5 s in MODE_DUAL
#endif

// ---- MACs ----------------------------------------------------------

// Deployed: real OUI prefix, distinctive suffix.
static const uint8_t MAC_DEPLOYED[6] = { 0x00, 0x25, 0xDF, 0xDE, 0xAD, 0xBE };

// Docked: sanitised AdvA. Some BLE controllers may refuse to advertise
// with a literal all-zero MAC; if esp_base_mac_addr_set() rejects this
// at boot, the emulator falls back to MAC_DOCKED_FALLBACK and prints a
// warning. The fallback still trips Phase 2 (UUID) but not Phase 3
// (sanitised MAC) on the receiver, which is the realistic worst case.
static const uint8_t MAC_DOCKED[6]          = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t MAC_DOCKED_FALLBACK[6] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 };

// ---- FE6B service-data payload ------------------------------------
//
// Layout the receiver expects (and that real Axon Body 3 broadcasts
// produce) for the bytes AFTER the 0xFE6B UUID:
//
//   bytes  0-1   opaque header
//   bytes  2-11  80-bit per-device identifier (10 bytes)
//   bytes 12-13  opaque
//   bytes 14-22  cleartext model/serial code, 9 ASCII chars
//                (e.g. "X60J0xxxN" on real units)
//
// Real units are 23 bytes in the payload. We synthesise a 23-byte
// payload with a recognisable but obviously-fake serial so an analyst
// inspecting capture files can immediately tell it came from the
// emulator and not a real Axon device.

static const uint8_t FE6B_PAYLOAD[23] = {
    /*  0 */ 0x00, 0x00,
    /*  2 */ 0x4E, 0x55, 0x4C, 0x57, 0x45, 0x41, 0x52, 0x21, 0x21, 0x21,  // "NULWEAR!!!"
    /* 12 */ 0x00, 0x00,
    /* 14 */ 'X','6','0','J','0','T','S','T','1',                          // synthetic 9-char serial
};

// ---- State for MODE_DUAL ------------------------------------------

static bool g_dual_state_docked = false;     // start in DEPLOYED
static uint32_t g_dual_last_switch_ms = 0;
static BLEAdvertising *g_adv = nullptr;
static bool g_docked_mac_was_fallback = false;

// ---- Helpers ------------------------------------------------------

static void log_mac(const char *prefix, const uint8_t *mac) {
    Serial.printf("%s%02X:%02X:%02X:%02X:%02X:%02X\n",
                  prefix, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// Try to install a base MAC. Returns true if the requested MAC took.
static bool try_set_base_mac(const uint8_t *mac) {
    esp_err_t err = esp_base_mac_addr_set(mac);
    if (err == ESP_OK) return true;
    Serial.printf("  esp_base_mac_addr_set() rejected MAC: err=%d\n", (int)err);
    return false;
}

// Build the deployed-mode advertisement: classic empty payload, just
// the controller MAC matters. Phase 1 hit only.
static void configure_advert_deployed() {
    BLEAdvertisementData advData;
    advData.setName("NULLWEAR-TEST");
    advData.setFlags(0x06);        // LE General Discoverable + BR/EDR Not Supported
    g_adv->setAdvertisementData(advData);
}

// Build the docked-mode advertisement: include FE6B service data with
// the synthetic serial. Phase 2 hit (and Phase 3 if MAC took).
static void configure_advert_docked() {
    BLEAdvertisementData advData;
    advData.setFlags(0x06);
    BLEUUID fe6b((uint16_t)0xFE6B);
    std::string payload;
    payload.assign((const char *)FE6B_PAYLOAD, sizeof(FE6B_PAYLOAD));
    advData.setServiceData(fe6b, payload);
    g_adv->setAdvertisementData(advData);
}

// Switch live MAC + advert payload between modes. Stops the advertiser
// briefly because the BLE stack does not allow base-MAC changes while
// advertising.
static void switch_to_deployed() {
    g_adv->stop();
    if (!try_set_base_mac(MAC_DEPLOYED)) {
        Serial.println("  WARNING: deployed MAC rejected — using whatever the controller picked");
    }
    configure_advert_deployed();
    g_adv->start();
    log_mac("  -> DEPLOYED MAC ", MAC_DEPLOYED);
}

static void switch_to_docked() {
    g_adv->stop();
    bool ok = try_set_base_mac(MAC_DOCKED);
    if (!ok) {
        ok = try_set_base_mac(MAC_DOCKED_FALLBACK);
        g_docked_mac_was_fallback = true;
        Serial.println("  WARNING: all-zero MAC rejected, using 02:00:00:00:00:00 fallback");
        Serial.println("           Receiver Phase-3 (sanitised-MAC) will NOT trigger;");
        Serial.println("           Phase-2 (FE6B UUID) will trigger normally.");
    }
    if (!ok) {
        Serial.println("  WARNING: docked-mode MAC could not be set at all");
    }
    configure_advert_docked();
    g_adv->start();
    if (g_docked_mac_was_fallback) {
        log_mac("  -> DOCKED MAC (fallback) ", MAC_DOCKED_FALLBACK);
    } else {
        log_mac("  -> DOCKED MAC ", MAC_DOCKED);
    }
}

// ---- setup() / loop() ---------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println("=== NULLWEAR test source — Axon BLE emulator ===");
#if NULLWEAR_TEST_MODE == MODE_DEPLOYED
    Serial.println("Mode: DEPLOYED  (Phase 1 OUI match only)");
#elif NULLWEAR_TEST_MODE == MODE_DOCKED
    Serial.println("Mode: DOCKED    (Phase 2 UUID + Phase 3 sanitised-MAC)");
#elif NULLWEAR_TEST_MODE == MODE_DUAL
    Serial.printf("Mode: DUAL      (alternating every %u ms)\n",
                  (unsigned)DUAL_PERIOD_MS);
#else
#error "NULLWEAR_TEST_MODE must be MODE_DEPLOYED, MODE_DOCKED, or MODE_DUAL"
#endif

    // Pick the initial MAC before BLEDevice::init so the controller
    // brings up advertising on the right address from the first packet.
#if NULLWEAR_TEST_MODE == MODE_DEPLOYED
    if (!try_set_base_mac(MAC_DEPLOYED))
        Serial.println("  WARNING: deployed MAC rejected at init");
#elif NULLWEAR_TEST_MODE == MODE_DOCKED
    if (!try_set_base_mac(MAC_DOCKED)) {
        if (try_set_base_mac(MAC_DOCKED_FALLBACK)) {
            g_docked_mac_was_fallback = true;
            Serial.println("  WARNING: all-zero MAC rejected, using fallback");
        }
    }
#else  // MODE_DUAL — start in deployed
    if (!try_set_base_mac(MAC_DEPLOYED))
        Serial.println("  WARNING: deployed MAC rejected at init");
#endif

    BLEDevice::init("NULLWEAR-TEST");
    BLEServer *server = BLEDevice::createServer();
    g_adv = server->getAdvertising();

    // Broadcast at roughly the rate a real Axon BLE adv is observed at:
    // ~33 ms interval = ~30 Hz. (BLE adv interval units are 0.625 ms.)
    g_adv->setMinInterval(0x0030);   // 48 * 0.625 = 30 ms
    g_adv->setMaxInterval(0x0040);   // 64 * 0.625 = 40 ms

    BLEDevice::setPower(ESP_PWR_LVL_N0);   // 0 dBm

#if NULLWEAR_TEST_MODE == MODE_DEPLOYED
    configure_advert_deployed();
    log_mac("Advertising as DEPLOYED ", MAC_DEPLOYED);
#elif NULLWEAR_TEST_MODE == MODE_DOCKED
    configure_advert_docked();
    if (g_docked_mac_was_fallback)
        log_mac("Advertising as DOCKED (fallback MAC) ", MAC_DOCKED_FALLBACK);
    else
        log_mac("Advertising as DOCKED ", MAC_DOCKED);
#else  // MODE_DUAL
    configure_advert_deployed();
    log_mac("Advertising as DEPLOYED ", MAC_DEPLOYED);
    g_dual_state_docked   = false;
    g_dual_last_switch_ms = millis();
#endif

    g_adv->start();
    Serial.println("Advertising. Use the reference receiver to verify.");
    Serial.println("Reset the board to stop.");
}

void loop() {
#if NULLWEAR_TEST_MODE == MODE_DUAL
    uint32_t now = millis();
    if ((now - g_dual_last_switch_ms) >= DUAL_PERIOD_MS) {
        g_dual_last_switch_ms = now;
        if (g_dual_state_docked) {
            switch_to_deployed();
            g_dual_state_docked = false;
        } else {
            switch_to_docked();
            g_dual_state_docked = true;
        }
    }
#endif

    // Periodic heartbeat so the operator can confirm the emulator is alive.
    static uint32_t last_print_ms = 0;
    uint32_t now = millis();
    if (now - last_print_ms > 5000) {
        last_print_ms = now;
#if NULLWEAR_TEST_MODE == MODE_DEPLOYED
        Serial.printf("[%lu] still advertising DEPLOYED\n", now);
#elif NULLWEAR_TEST_MODE == MODE_DOCKED
        Serial.printf("[%lu] still advertising DOCKED%s\n",
                      now, g_docked_mac_was_fallback ? " (fallback MAC)" : "");
#else
        Serial.printf("[%lu] still advertising DUAL  (current=%s)\n",
                      now, g_dual_state_docked ? "DOCKED" : "DEPLOYED");
#endif
    }
    delay(50);
}
