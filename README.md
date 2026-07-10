# Metric Firmware nRF52

Replacement firmware for the STM32F411 + MPU6050 board in `../Metric Prototype`,
targeting new hardware:

- MCU: Nordic nRF52832 on a Laird/Ezurio BL652 module (custom board — not
  built yet)
- IMU: Bosch BMI270 (replacing the MPU6050)
- Output: BLE notifications (replacing USB serial)

Built with PlatformIO + the Adafruit nRF52 Arduino core (`framework = arduino`
on the `nordicnrf52` platform), so the Arduino-style code stays close to the
STM32 firmware this replaces.

The old STM32/MPU6050 firmware in `Metric Prototype` is untouched and stays
the reference for the serial data format
(`time_ms, ax, ay, az, gx, gy, gz` at 100 Hz) that this firmware must
eventually reproduce over BLE.

## Bring-up plan

Built and tested in stages, on real hardware, one at a time:

1. **Toolchain bring-up** — bare PlatformIO project on the nRF52840-DK,
   blink + serial heartbeat. Proves the build/flash chain works. *(current
   stage)*
2. **BMI270 sensor bring-up** — read chip ID (expect `0x24`), then accel +
   gyro, printed over serial.
3. **BLE transport** — define a GATT service/characteristic, stream samples
   over BLE notifications instead of serial.
4. **Format match** — align the exact packet layout with the API/BLE spec so
   the existing analysis pipeline (`Metric Prototype/imu_pipeline`) works
   unchanged.

Each stage is a separate commit once it's confirmed working on hardware.

## Hardware bring-up target vs. final target

Stage 1–3 are brought up on an **nRF52840-DK**, not the final board, because:

- No custom PCB or dev kit for the BL652 exists yet.
- The nRF52840-DK has an on-board J-Link debugger and native USB, both of
  which make bring-up much easier than the final board will allow.

Known differences to handle before this moves to the real board:

- **No USB on nRF52832.** The BL652/nRF52832 has no native USB peripheral,
  so serial-over-USB (used for debug prints in stages 1–2) won't exist on
  the final board. Debug output there will need SWD/RTT, a UART-to-USB
  bridge, or just BLE.
- **No official PlatformIO board definition for the BL652.** Stage 4 (or
  earlier) will likely need a custom `boards/*.json` — the closest existing
  PlatformIO boards are `nrf52_dk` (Nordic's own nRF52832 DK) and
  `adafruit_feather_nrf52832`, neither of which matches the BL652's actual
  pinout/flash layout.
- **I2C vs SPI for the BMI270 is still open.** The BMI270 supports both;
  which one the custom board wires up isn't decided yet. Whichever library
  we pick in Stage 2 should support both so Stage 2 can be validated on the
  DK with jumper wires before the pin choice is locked in for the PCB.
- **Pin mapping for the custom board is unknown** — SPI/I2C pins, any
  interrupt line from the BMI270, LED/button pins, antenna matching for the
  module's RF — none of this exists yet since the board hasn't been
  designed. Flag these explicitly whenever they'd affect firmware.

## Stage 1 — what to verify on hardware

1. Install the "Nordic Semiconductor" platform and Adafruit nRF52 core
   support in PlatformIO if not already present (PlatformIO will pull
   `platform = nordicnrf52` automatically on first build).
2. Flashing uses `nrfjprog` (Nordic's nRF Command Line Tools) via the DK's
   on-board J-Link — install nRF Command Line Tools / SEGGER J-Link
   software if `pio run -t upload` can't find the debugger.
3. Connect the nRF52840-DK over USB, build and upload.
4. **Confirm on hardware:**
   - LED1 on the DK blinks at ~1 Hz.
   - Opening a serial monitor at 115200 baud shows `Metric Firmware nRF52 —
     Stage 1: blink` followed by a `tick @ ... ms, LED ON/OFF` line twice a
     second.

Nothing in this stage has been run on real hardware yet — the LED polarity
handling (`ledOn()`/`ledOff()` instead of raw `digitalWrite()`) is based on
the DK's documented active-low LED wiring, but hasn't been confirmed.
