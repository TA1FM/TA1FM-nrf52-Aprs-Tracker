# TA1FM nRF52 LoRa APRS Tracker — User Guide (English)

**Firmware:** V0.9.2 STABLE · **Hardware:** TA2DMX MiniMesh Lite NRF (nRF52840 / SX1262)  
**Frequency:** 433.775 MHz · **Web configurator:** https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/

> This guide is based on the **actual V0.9.2 source code**. Other firmware versions may have different menus. `TA1FM-7` is an example callsign; configure your own valid amateur radio callsign.

## 1. Getting started

1. Install the V0.9.2 firmware using the board's compatible UF2 bootloader. An eraser is **not needed** for routine updates.
2. Connect the board by USB and open the [TA1FM Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/) in **Chrome or Edge**.
3. Select **USB Connect** and the correct serial/COM port; use **Read from Device** to retrieve current settings.
4. Configure callsign, AUTO Beacon, symbol, transmit intervals, Comment, and Status. Use **Write to Device** to apply changes.
5. Read the settings again to verify them. You can also create a **JSON backup**.
6. With an antenna attached, take the tracker outdoors or to a clear-sky location and wait for `GPS:FIX`.

**Important:** When the callsign is `NOCALL`, APRS transmission is blocked. Saving the first valid callsign turns AUTO on. A later explicit AUTO OFF choice is remembered after power cycling; re-enable it in the configurator if required.

## 2. Single-button operation

| Action | Main screen | Menu | Setting / info screen |
|---|---|---|---|
| **Short press** | Request a manual position APRS beacon | Next menu item | Change/select next value; no action on read-only info pages |
| **Long press** (about **1.2 sec**) | Open main menu | Open selected item | Save/confirm, or return from read-only pages |

**Example:** Hold for 1.2 seconds on the main screen → `SET` menu → tap until `DISPLAY` is selected → hold to enter.

- The menu returns to the main screen after **30 seconds** without user activity, except for the separate OLED preview countdown.
- A short tap on the main screen requests manual position TX, but actual transmission still requires a valid callsign, GPS position, and ready radio.
- The menu button is **not** the RESET button used to enter the UF2 bootloader.

## 3. OLED main screen

Example display (values are illustrative):

```text
TA1FM-7        18:42:15
---------------------
GPS:FIX       SAT:10
SPD:  0.0 km/h
LAT: 40.97500
LON: 27.50400
BAT:3.92V  RX:TA1FM-9
```

| Display | Meaning |
|---|---|
| Callsign | Configured station callsign |
| Time | GPS time, adjusted to UTC or UTC+3 as selected |
| `GPS:FIX` | Fresh, valid GPS location available |
| `GPS:SEARCH` | Waiting for valid GPS location |
| `SAT` | Number of GPS satellites |
| `SPD` | GPS speed in km/h |
| `LAT` / `LON` | Latitude / longitude in decimal degrees |
| `BAT` | Approximate battery voltage |
| `RX` | Last APRS station heard over LoRa |

## 4. Full OLED menu reference

With a detected BME280 sensor, the menu has **9 items**:

| Menu | Purpose | Button action |
|---|---|---|
| `BME280 WX` | Temperature, relative humidity, pressure | Read-only; hold to go back |
| `AUTO BEACON` | Automatic transmissions ON/OFF | Tap to toggle; hold to save |
| `POS SYMBOL` | APRS position symbol | Tap to cycle; hold to save |
| `UTC OFFSET` | `UTC` or `UTC+3` | Tap to change; hold to save |
| `DISPLAY` | `SH1106` / `SSD1306` OLED driver | Tap to select; hold to test |
| `GPS INFO` | Satellite count, fix, GPS character count | Read-only; hold to go back |
| `LORA INFO` | LoRa health, RX statistics, RSSI/SNR, AUTO | Read-only; hold to go back |
| `DEVICE INFO` | Device model and firmware | Read-only; hold to go back |
| `EXIT` | Return to main screen | Hold to exit |

Without BME280, the `BME280 WX` item is hidden, leaving **8 items**.

### AUTO BEACON

- `ON`: Enables automatic beacon scheduling subject to operating conditions.
- `OFF`: Disables automatic beacons; this choice persists after power cycling.
- The page shows information such as `POS:SMART` and `WX:60m`, but **Smart/Fixed selection and detailed intervals are set through the Web Configurator**.

### POS SYMBOL

V0.9.2 supports exactly these three choices:

| OLED option | APRS symbol |
|---|---|
| `CAR />` | Car (`/>`) |
| `WALKER /[` | Walking person (`/[`) |
| `HOUSE /-` | House (`/-`) |

A dedicated motorcycle symbol is **not yet included in V0.9.2**; it is planned for a later release. The firmware may use a parked-position indicator in position beacons.

### DISPLAY — selecting the correct OLED type

1. Enter `DISPLAY`.
2. Tap to toggle between `SH1106` and `SSD1306`.
3. Hold to start the **10-second display preview**.
4. If `DISPLAY TEST` is readable, **hold again before the countdown expires** to confirm and save.
5. If unconfirmed, the firmware **restores the previous display driver**.

**Blank-screen recovery:** Hold the device's **menu button while powering up** and keep it held for the first **3 seconds**. The firmware switches to the other OLED driver and stores that selection. Do not confuse this with double-tapping the separate RESET button for UF2 bootloader access.

## 5. Position, parked and weather operation

- **Smart Beaconing:** Adjusts position timing according to GPS speed, motion and heading-change logic.
- **Fixed Position:** Uses a configurable fixed interval.
- **Parked:** Detects sustained lack of movement; the default parked confirmation is 3 minutes.
- **WX:** With BME280 present, can transmit separate APRS weather beacons, default interval 60 minutes.
- **Status:** Separate APRS status message with configurable interval.
- **Comment:** Included **in a position beacon**, not sent as its own packet.
- Position TX requests have a 10-second minimum spacing; auxiliary WX/Status TX requests require 30 seconds since the most recent transmission. These are safety gaps, **not** default beacon intervals.

## 6. Web Configurator and backups

[**Open TA1FM Web Configurator**](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)

1. Establish the USB serial connection.
2. Use **Read from Device**. Do not assume pre-filled fields were loaded from the tracker until the read succeeds.
3. Make changes and choose **Write to Device**.
4. Check the confirmation messages, then read again to verify.
5. Export a **JSON backup** if needed. Importing a backup into the web form may still require writing it to the device.
6. You can power-cycle the board and re-read settings to verify persistence.

The serial monitor shows diagnostic lines and advanced command responses. **Only one program should hold the COM port at a time.**

## 7. Advanced serial command reference

Most users can rely entirely on the web configurator.

| Command | Function |
|---|---|
| `p` | Print status and configuration |
| `c TA1FM-7` | Save example callsign |
| `a on` / `a off` | AUTO on / off |
| `f smart` / `f fixed` | Position mode |
| `i 1` / `i 3` / `i 5` / `i 10` / `i 15` | Fixed interval (minutes) |
| `s car` / `s walker` / `s house` | Position symbol |
| `k 1` / `k 3` / `k 5` | Park confirmation interval (minutes) |
| `x off` / `x 15` / `x 30` / `x 60` | WX interval |
| `z off` / `z 15` / `z 30` / `z 60` | Status interval |
| `j off` / `j 15` / `j 30` / `j 60` | Position Comment interval |
| `comment TEXT` | Save Comment text |
| `status TEXT` | Save Status text |
| `comment?` / `status?` | Read stored texts |
| `t` / `w` / `v` | Manual position / WX / Status TX |

**Serial speed:** 115200 baud. Comment and Status accept ASCII text of 1–62 characters without `|` or `~`.

## 8. Troubleshooting

| Symptom | First checks |
|---|---|
| `GPS:SEARCH` | Sky visibility, antenna, GPS power, satellite reception |
| No TX | Use `p`: valid callsign? AUTO ON? GPS FIX YES? LoRa ready? |
| OLED blank | Wrong SH1106/SSD1306 selection? Try 3-second power-on OLED recovery |
| BME280 menu missing | Sensor not detected; check wiring and I2C address |
| Settings not saved | Look for `saved` vs. `SAVE FAILED` in serial output |
| USB won't connect | Data-capable USB cable, Chrome/Edge, serial port already open elsewhere |

## 9. Links

- [Firmware repository and UF2](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker)
- [Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)
- [Third-party configuration erase tools](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/tools/README.md)

**73 de TA1FM!**
