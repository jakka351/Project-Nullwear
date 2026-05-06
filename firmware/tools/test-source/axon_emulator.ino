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
 * Lab testing of NULLWEAR requires a BLE source that broadcasts the
 * same OUI as a real Axon device, without requiring access to actual
 * Axon hardware. This sketch turns a $5 ESP32 dev board into exactly
 * that source.
 *
 * The emulator broadcasts an ADV_NONCONN_IND advertising packet at
 * roughly the same rate (~30 Hz) as a real Axon body camera. The MAC
 * address is fixed at 00:25:DF:DE:AD:BE so it can easily be
 * distinguished in the receiver log from any real Axon devices that
 * may be in the test area.
 *
 * Use it together with the reference receiver in
 *   ../reference-receiver/ref_receiver.py
 * to perform the verification described in
 *   docs/12-acceptance-test-procedure.md.
 *
 * IMPORTANT
 * =========
 *
 * Because this emulator broadcasts a 00:25:DF MAC, in some
 * jurisdictions running it could be construed as impersonating
 * a piece of registered law-enforcement equipment. Restrict its
 * use to controlled lab environments (an RF-shielded room or
 * an isolated location well outside Axon equipment range), and
 * power it down between tests.
 */

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>

// Fixed test MAC: OUI 00:25:DF, suffix DE:AD:BE for unmistakability. 
// Can't not have DEADBEEF somewhere can we? What about CAFEBABE?
static uint8_t g_mac[6] = { 0x00, 0x25, 0xDF, 0xDE, 0xAD, 0xBE };

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println("=== NULLWEAR test source — Axon BLE emulator ===");
    Serial.printf("Will broadcast OUI 00:25:DF MAC: "
                  "%02X:%02X:%02X:%02X:%02X:%02X\n",
                  g_mac[0], g_mac[1], g_mac[2],
                  g_mac[3], g_mac[4], g_mac[5]);

    // Override the controller MAC. ESP32 BLE stack uses its base MAC
    // for the advertising address. Setting it before BLEDevice::init
    // makes the change take effect.
    esp_base_mac_addr_set(g_mac);

    BLEDevice::init("NULLWEAR-TEST");
    BLEServer *server = BLEDevice::createServer();
    BLEAdvertising *adv = server->getAdvertising();

    // Broadcast at roughly the rate a real Axon BLE adv is observed
    // at: 33 ms interval = ~30 Hz. (BLE adv interval units are
    // 0.625 ms.)
    adv->setMinInterval(0x0030);    // 48 * 0.625 = 30 ms
    adv->setMaxInterval(0x0040);    // 64 * 0.625 = 40 ms

    // Reduce TX power to a more representative level
    BLEDevice::setPower(ESP_PWR_LVL_N0);   // 0 dBm

    adv->start();

    Serial.println("Advertising. Use the reference receiver to verify.");
    Serial.println("Reset the board to stop.");
}

void loop() {
    // Periodic heartbeat to the serial port so the operator can confirm
    // the emulator is alive.
    static uint32_t last_print_ms = 0;
    uint32_t now = millis();
    if (now - last_print_ms > 5000) {
        last_print_ms = now;
        Serial.printf("[%lu] still advertising %02X:%02X:%02X:%02X:%02X:%02X\n",
            now, g_mac[0], g_mac[1], g_mac[2], g_mac[3], g_mac[4], g_mac[5]);
    }
    delay(100);
}
