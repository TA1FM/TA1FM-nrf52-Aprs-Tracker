# TA1FM nrf52 Tracker + Digipeater — User Guide (V1.1.0 STABLE)

**[Firmware Release](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.1.0)** · **[V1.1.0 Web Control Panel](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html)** · [Installation](./INSTALL_EN.md) · [Serial Commands](./SERIAL_CONSOLE_COMMAND_GUIDE_EN.md)

## 1. Operating modes

| Mode | Function |
|---|---|
| **TRACKER** | GPS position, SMART/FIXED beaconing, moving/parked APRS symbols and position Comment |
| **DIGIPEATER** | Fixed coordinates, WIDE packet repeating, scheduled RF beacon |

LoRa APRS: **433.775 MHz, SF12, BW125, CR4/5**. Supports SX1262/SX1268 radios, SH1106/SSD1306 OLED and optional BME280 weather sensor.

## 2. Getting started

1. Connect a suitable RF antenna; configure your authorized callsign/SSID.
2. Open the [web panel](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html) using desktop Chrome/Edge for USB or an HTTPS Web Bluetooth-capable browser (e.g. Android Chrome) for BLE.
3. **Read settings before making changes**, enable saving when required, then write and read back. Back up the JSON configuration.
4. Check GPS FIX, LoRa RX, battery voltage, chip selection and TX counters.
5. Incoming APRS message history lives in RAM and is lost after power cycling.

## 3. Tracker: position, speed, Comment, Status, weather

- **SMART Beacon** dynamically uses GPS speed and turn events; **FIXED** uses a selected position interval.
- Configure independent moving and parked APRS symbols (e.g. `/>` and `\\P`).
- When GPS course and speed are fresh and valid, position frames include the APRS `ccc/sss` extension (degrees / knots). No fabricated data is sent.
- **Comment** is inserted into eligible position frames according to its interval; it is not a separate RF packet.
- **Status** is transmitted in a separate APRS `>` frame and prioritized ahead of routine WX and position transmissions.
- **Weather (WX)** needs valid optional BME280 measurements; missing sensors are not substituted with invented data.
- The **shared 10-second radio TX guard** remains active. A local `RadioLib TX OK` does not prove another station received the packet.

## 4. APRS messaging

Use the **Messages** tab in the web panel; select the appropriate USB/BLE device, enter the destination callsign (SSID included), message text and **1, 2 or 3 total RF attempts**. Follow QUEUED/ACK/ACK_TIMEOUT states. The last **10 incoming messages** are held in volatile RAM and can be viewed using the web panel or OLED.

**ACK** confirms a device-level protocol receipt, not that a human read the text. Visibility on aprs.fi depends on a reachable iGate.

## 5. Digipeater

Before selecting `mode digi`, configure callsign and both `digi lat` and `digi lon`. Set `digi path 1|2` and `digi beacon 10|15|30|60`. Verify RF operation. DIGI mode is **not** an APRS-IS iGate.

## 6. USB, BLE and OLED

- USB serial: **115200 baud**. Do not use the serial monitor simultaneously with the USB web panel on the same COM port.
- BLE device name: **`nRF52_Lora_Tracker`**. Saving settings without a PIN is **not strong authentication**.
- OLED driver: SH1106 or SSD1306. USB supports `oled sh1106` / `oled ssd1306` preview followed by `oled confirm`. Recover from wrong display type as documented in [installation](./INSTALL_EN.md).
- Version reporting: `version?` prints `VERSION|1.1.0|STABLE` over USB; BLE supports the corresponding version response; OLED shows `V1.1.0`.

## 7. BLE OTA firmware updates

1. Download the Release **ZIP** DFU package.
2. Connect to the normal Tracker with the [web panel](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html); enable Save and select **Enter BLE DFU Mode** and confirm.
3. The tracker advertises as **`AdaDFU`**. Open the Android **Nordic nRF Device Firmware Update** app.
4. Select the ZIP and AdaDFU. Set **PRN 4** and **Request High MTU OFF**.
5. Keep power stable until success; verify the regular Tracker boots.

Tested bootloader **nice!nano UF2 0.10.0 / S140 6.1.1**. Legacy single-bank DFU interruption may require USB UF2 recovery. The web panel enters DFU mode; **the ZIP transfer takes place in the Nordic app**, not in the browser. USB serial `dfu` is an alternate entry method.

## 8. Common serial commands

```text
version?
p
g
r
chip?
mode?
smart?
flash?
msg hello
msg inbox
msg status
msg send TA1FM-7 HELLO
dfu
```

See [complete serial console commands](./SERIAL_CONSOLE_COMMAND_GUIDE_EN.md).

## 9. Troubleshooting

| Symptom | Action |
|---|---|
| No GPS FIX | Verify GPS power, UART and sky view |
| LoRa RX error | Confirm actual SX1262/SX1268 setting and restart |
| BLE unavailable | Check HTTPS, Bluetooth permissions and whether device is in AdaDFU mode |
| Blank OLED | Verify OLED driver / rescue setting |
| DFU interruption | Use PRN=4, High MTU OFF and stable power; recover via UF2 if needed |
| Inbox erased | Expected after reboot: RAM-only storage |

Older V0.9.x-specific guides are retained for historical reference; this document targets **V1.1.0 STABLE**.
