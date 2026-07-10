// Stage 1 — toolchain bring-up only.
//
// Goal: prove that PlatformIO + the Adafruit nRF52 core can build and flash
// the nRF52840-DK. No sensor, no BLE yet — just a blinking LED and a serial
// heartbeat so we have two independent ways to confirm it's alive.
//
// NOTE (unverified until flashed): on the nRF52840-DK, the four user LEDs
// (LED1..LED4) are wired active-LOW — the LED lights when the pin is driven
// LOW, not HIGH. The Adafruit core's LED_BUILTIN maps to LED1 and its
// ledOn()/ledOff() helpers already account for this, so we use those instead
// of raw digitalWrite() to avoid an inverted-blink surprise.

#include <Arduino.h>

const unsigned long BLINK_INTERVAL_MS = 500;
unsigned long lastToggle = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);

  // Give the USB CDC serial port a moment to enumerate before we print.
  // The nRF52840 has native USB, so this shows up as a virtual COM port
  // with no extra USB-to-serial adapter needed. (The nRF52832 on our real
  // target board has no USB at all — this convenience goes away when we
  // move off the DK, see README.md.)
  uint32_t start = millis();
  while (!Serial && (millis() - start) < 3000) {
    delay(10);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  ledOff(LED_BUILTIN);

  Serial.println("Metric Firmware nRF52 — Stage 1: blink");
}

void loop() {
  unsigned long now = millis();

  if (now - lastToggle >= BLINK_INTERVAL_MS) {
    lastToggle = now;
    ledState = !ledState;

    if (ledState) {
      ledOn(LED_BUILTIN);
    } else {
      ledOff(LED_BUILTIN);
    }

    Serial.print("tick @ ");
    Serial.print(now);
    Serial.print(" ms, LED ");
    Serial.println(ledState ? "ON" : "OFF");
  }
}
