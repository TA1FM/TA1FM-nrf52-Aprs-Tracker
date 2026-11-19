# TA1FM nrf52 Tracker — Installation (V1.1.0 STABLE)

**Latest:** [V1.1.0 STABLE Release](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.1.0) · [Control Panel](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html) · [User Guide](./USER_GUIDE_EN.md) · [Serial Commands](./SERIAL_CONSOLE_COMMAND_GUIDE_EN.md)

## 1. Hardware and safety

Compatible **nRF52840 MiniMesh Lite NRF**, SX1262 or SX1268 radio, GPS, 433.775 MHz antenna; SH1106/SSD1306 OLED and optional BME280. LoRa: **433.775 MHz, SF12, BW125, CR4/5**.

This is **board-specific firmware**, not a universal nRF52840 image. Check GPIO pins, RF switch, TCXO, SoftDevice and bootloader compatibility. Always attach an antenna before transmitting. Back up working firmware and saved configuration JSON.

## 2. USB UF2 installation / recovery

1. Download the **V1.1.0 `.uf2`** from [Release Assets](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.1.0).
2. Connect a USB data cable, then double-tap RESET on a compatible UF2 bootloader board.
3. Copy the UF2 file onto the mounted UF2 drive; the board reboots.
4. Verify OLED `V1.1.0` and USB serial `V1.1.0 STABLE` at 115200 baud.
5. Check callsign, SX1262/SX1268 selection, GPS FIX, LoRa RX and saved settings.

Do **not** copy the DFU ZIP to the UF2 drive. Do not erase flash or run `flash init` as part of normal updates.

## 3. USB web configuration

Open the [V1.1.0 Control Panel](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html) in desktop Chrome/Edge. Close other programs that own the serial COM port. Connect via USB, **read settings first**, edit and save, then read back for verification. Save a JSON configuration backup. If changing the radio chip, restart the board for the new selection to take effect.

## 4. Android Bluetooth BLE configuration

Use HTTPS and a Web Bluetooth-capable browser such as Android Chrome. Connect to **`nRF52_Lora_Tracker`**, read settings, enable the **Save** switch, apply changes, then read back. The save switch helps avoid accidental writes but **is not strong authentication**.

## 5. Wireless firmware update — Android Nordic DFU

**Tested bootloader:** **nice!nano UF2 0.10.0 / S140 6.1.1**. Required Android application: **Nordic Semiconductor nRF Device Firmware Update**.

1. Download the **V1.1.0 `.zip`** DFU firmware package from [Release Assets](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.1.0).
2. Connect to the normal Tracker from the [web panel](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html) over Bluetooth.
3. Enable Save; choose **Enter BLE DFU Mode** and confirm the two-step action.
4. The device disconnects and advertises as **`AdaDFU`**.
5. Open the Nordic DFU app, select the **ZIP** package and **AdaDFU** device.
6. Set **PRN = 4** and **Request High MTU = OFF**.
7. Start transfer. Maintain stable power, do not reset, and verify the normal Tracker restarts after success.

Alternative: send `dfu` from the USB serial console (115200 baud / Newline) to enter AdaDFU.

**Important:** This is **Legacy single-bank BLE DFU**. An interrupted upload may erase the application and require USB UF2 recovery. A prior working transfer took about 2.5 minutes; actual time varies. The web panel **enters DFU mode only**, it does not upload the firmware ZIP directly.

## 6. First configuration and verification

- **TRACKER:** Set a valid licensed callsign, confirm GPS FIX/LoRa RX, SMART or FIXED beacon and APRS symbols. Fresh GPS course/speed may appear as `ccc/sss` in APRS position packets.
- **DIGIPEATER:** Before `mode digi`, set callsign and fixed latitude/longitude. Choose `digi path 1|2` and beacon interval.
- **APRS messages:** USB/BLE panel supports send, receive, inbox, ACK and 1/2/3 total RF TX attempts. An ACK is a device-level acknowledgment, not proof of human reading.
- **Storage:** Direct Flash A/B + CRC32 settings persist; the last 10 received APRS messages remain in RAM only, and are lost on restart.
- **Scheduling:** Status is a separate RF packet; Comment is embedded in selected position packets. The shared 10-second RF guard remains active.

## 7. Troubleshooting

| Symptom | Check |
|---|---|
| No GPS FIX | Antenna, outdoor sky view, supply, GPS UART |
| LoRa RX error | Correct SX1262/SX1268 setting, RF hardware, restart |
| USB panel fails | Close Serial Monitor / other COM-port users |
| No BLE device | Bluetooth enabled, HTTPS, browser permissions |
| No AdaDFU | Compatible UF2 bootloader; test USB `dfu` |
| DFU interrupted | PRN=4, High MTU OFF; USB UF2 recovery might be required |
| Blank OLED | Correct SH1106/SSD1306 driver and recovery steps |

Previous V0.9.x documents are historical archives, **not current V1.1.0 installation instructions**.
