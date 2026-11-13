# TA1FM nRF52 LoRa APRS Tracker — User Guide (English)

**Firmware:** V0.9.22 UNIVERSAL BLE · **Hardware:** TA2DMX MiniMesh Lite NRF (nRF52840 / SX1262 or SX1268)  
**Frequency:** 433.775 MHz · **Web configurator:** https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/

> This guide covers the **V0.9.22 UNIVERSAL BLE source and current USB/BLE web panel**. Older firmware versions can have different menus/storage. `TA1FM-7` is an example callsign; configure your own valid amateur radio callsign.

## 1. Getting started

1. Install the V0.9.22 firmware using the board's compatible UF2 bootloader. An eraser is **not needed** for routine updates.
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

With a detected BME280 sensor, the menu has **11 items**:

| Menu | Purpose | Button action |
|---|---|---|
| `BME280 WX` | Temperature, relative humidity, pressure | Read-only; hold to go back |
| `AUTO BEACON` | Automatic transmissions ON/OFF | Tap to toggle; hold to save |
| `POS SYMBOL` | APRS position symbol | Tap to cycle; hold to save |
| `UTC OFFSET` | `UTC` or `UTC+3` | Tap to change; hold to save |
| `DISPLAY` | `SH1106` / `SSD1306` OLED driver | Tap to select; hold to test |
| `OLED TIMER` | Screen OFF delay (always ON/OFF; 15s/30s/1m/2m/5m) | Tap to select; hold to save |
| `TX POWER` | Requested LoRa power, 1–22 dBm | Tap to adjust; hold to save |
| `GPS INFO` | Satellite count, fix, GPS character count | Read-only; hold to go back |
| `LORA INFO` | LoRa health, RX statistics, RSSI/SNR, AUTO | Read-only; hold to go back |
| `DEVICE INFO` | Device model and firmware | Read-only; hold to go back |
| `EXIT` | Return to main screen | Hold to exit |

Without BME280, the `BME280 WX` item is hidden, leaving **10 items**.

### AUTO BEACON

- `ON`: Enables automatic beacon scheduling subject to operating conditions.
- `OFF`: Disables automatic beacons; this choice persists after power cycling.
- The page shows information such as `POS:SMART` and `WX:60m`, but **Smart/Fixed selection and detailed intervals are set through the Web Configurator**.

### POS SYMBOL

The OLED menu offers three presets; the USB/BLE panel also supports arbitrary valid two-character APRS moving and parked symbols:

| OLED option | APRS symbol |
|---|---|
| `CAR />` | Car (`/>`) |
| `WALKER /[` | Walking person (`/[`) |
| `HOUSE /-` | House (`/-`) |

For additional APRS symbols, use the two-character moving and parked symbol fields in the USB/BLE panel.

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
- All position, WX and Status RF frames share a **minimum 15-second LoRa TX guard** from the previous transmission. These are safety gaps, **not** default beacon intervals.

## 6. Web Configurator and backups

[**Open TA1FM Web Configurator**](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)

1. Establish the USB serial connection.
2. Use **Read from Device**. Do not assume pre-filled fields were loaded from the tracker until the read succeeds.
3. Make changes and choose **Write to Device**.
4. Check the confirmation messages, then read again to verify.
5. Export a **JSON backup** if needed. Importing a backup into the web form may still require writing it to the device.
6. You can power-cycle the board and re-read settings to verify persistence.

The serial monitor shows diagnostic lines and advanced command responses. **Only one program should hold the COM port at a time.**

### 6.1 USB and Bluetooth BLE configuration (V0.9.22)

**[USB + BLE web configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)**

- **USB:** Desktop Chrome/Edge, USB data cable; **Connect to device → Read from device (p) → Write to device**. Close other COM-port programs. USB OLED type changes keep their preview/confirmation flow.
- **BLE:** Android Chrome over HTTPS with Bluetooth enabled. **Connect via Bluetooth** to **nRF52_Lora_Tracker** → **Read BLE settings** → turn **Save enabled ON** → edit → **Save BLE settings**.
- BLE has **no PIN**. The save toggle is a write gate, **not authentication**. Settings are staged and saved in **one Direct Flash A/B + CRC32 transaction**. The toggle returns OFF after a successful save or disconnection.
- BLE writes require **Direct Flash ACTIVE**; flash initialization is intentionally USB-only. Settings backups can be exported/imported as JSON; **import does not itself write to the device**.

### 6.2 Radio chip, OLED and APRS symbols

- **Radio:** Select the actual installed **SX1262** or **SX1268** in the BLE panel. After a chip change **reboot the device**. RF is disabled until reboot; wrong selection can prevent RF communication. USB advanced serial: `chip sx1262` / `chip sx1268`.
- **OLED:** Select SH1106 or SSD1306. BLE saves and activates the driver **immediately, without the 20-second preview**. Wrong type can blank the screen. To recover, hold the **menu/user button for the first 3 seconds during boot**; this is separate from double RESET for UF2. USB retains its preview/confirm method.
- **Map symbols:** Separate two-character APRS codes for **moving** (`/>`, car) and **parked** (`\P` by default). First character is the table (`/` or `\`), second the symbol. Enter a valid APRS code, not emoji. USB serial: `symbol />` / `parksymbol \P`.

### 6.3 Startup beacons and RF guard

With AUTO enabled, valid callsign, fresh GPS FIX and Comment/Status intervals enabled, the first **position packet also includes Comment**, followed by a **separate Status packet** after the **15-second shared LoRa TX guard** permits it. Comment is not a separate RF frame. WX remains independently timed and uses the same shared 15-second TX guard.

### 6.4 Direct Flash storage and recovery

V0.9.22 uses **Direct Flash A/B + CRC32**, not LittleFS configuration files. Back up the last working UF2 before upgrading. Use USB serial `p` / `flash?` to inspect status. `flash init` is an advanced **USB-only** action intended **only for devices with no valid existing A/B record**, after backing up settings. APRS messaging is **not implemented** in this release.

## 7. Advanced serial command reference

Most users can rely entirely on the web configurator.

| Command | Function |
|---|---|
| `p` | Print status and configuration |
| `c TA1FM-7` | Save example callsign |
| `a on` / `a off` | AUTO on / off |
| `f smart` / `f fixed` | Position mode |
| `i 1` / `i 3` / `i 5` / `i 10` / `i 15` | Fixed interval (minutes) |
| `s car` / `s walker` / `s house` | Three built-in OLED position symbols |
| `symbol />` / `parksymbol \\P` | Moving / parked APRS symbols (two characters) |
| `chip sx1262` / `chip sx1268` | Select radio chip (reboot required) |
| `flash?` | Direct Flash A/B record status |
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
| Settings not saved | Check `flash?` (Direct Flash ACTIVE), `SAVED` / `SAVE FAILED` messages and readback |
| BLE can't connect | Use Android Chrome + HTTPS; check Bluetooth is on and no other app owns connection |
| BLE Save disabled | Turn on Save enabled after reading; switch returns OFF after saving/disconnect |
| LoRa fails after chip change | Verify actual SX1262/SX1268 hardware, reboot and inspect RF status |
| USB won't connect | Data-capable USB cable, Chrome/Edge, serial port already open elsewhere |

## 9. Links

- [Firmware repository and UF2](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker)
- [Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)
- [Third-party configuration erase tools](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/tools/README.md)

**TA1FM V0.9.22 — 73 de TA1FM!**
