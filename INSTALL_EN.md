# TA1FM nRF52 LoRa APRS Tracker — Installation (V0.9.22)

**Current firmware:** V0.9.22 UNIVERSAL BLE  
**Target:** TA2DMX MiniMesh Lite NRF (nRF52840) with **SX1262 or SX1268** LoRa radio  
**APRS LoRa:** 433.775 MHz, SF12, BW125, CR 4/5

> Hardware-specific firmware: this is **not** a general-purpose nRF52840 UF2. Verify your board variant, pin mapping, nice!nano-compatible UF2 bootloader and S140 6.1.1 configuration. Always keep a known-good firmware/config backup.

## 1. Flash the prebuilt UF2

1. Back up your working firmware and settings (JSON export if available).
2. Download [V0.9.22 UF2](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_22.uf2). [Arduino source](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_22.ino) is included in this repository.
3. **Double-tap RESET** to mount the UF2 USB drive.
4. Copy `TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_22.uf2` to that drive. The board will reboot.
5. Read the device settings using the [USB + BLE Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/).
6. Verify callsign, **Direct Flash ACTIVE**, radio chip, OLED, AUTO beacon, GPS FIX and RF status. Reboot after changing the radio chip.

**Do not run flash erasers or format storage for a routine update.** V0.9.22 uses validated **Direct Flash A/B + CRC32** records; it does not initialize LittleFS. Existing older SX1268 Direct Flash records can be read, with SX1268 used by default for records without an explicit chip field.

## 2. USB settings

1. Open the web configurator in **desktop Chrome/Edge** and attach a USB data cable.
2. Select **Connect to device → Read from device (p)**.
3. Edit your settings and click **Write to device**; verify the commit and read again.
4. Disconnect other programs using the same serial/COM port. USB OLED driver changes retain the existing preview/confirm process.

## 3. Bluetooth BLE settings

1. On Android Chrome open the configurator using **HTTPS**, with Bluetooth enabled.
2. Use **Connect via Bluetooth**, choosing **nRF52_Lora_Tracker**.
3. Select **Read BLE settings** before editing.
4. Turn the **Save enabled** switch ON; **there is no PIN** in V0.9.22.
5. Select **Save BLE settings** to stage changes in RAM and persist them in **one Direct Flash A/B transaction**.
6. The switch automatically returns OFF after a successful save or a Bluetooth disconnect.

This switch limits accidental writes; it is **not strong authentication or encryption**. BLE writes require **Direct Flash ACTIVE**. Flash initialization is intentionally **USB-only**.

## 4. New settings and important behavior

- **Radio chip:** Choose the actual SX1262 or SX1268 fitted to the board. Changing it disables RF until **restart**; choosing the wrong chip can prevent radio operation.
- **OLED:** Choose SH1106 or SSD1306. BLE saves the new driver **immediately without a 20-second preview**. An incorrect selection may blank the display; restore with USB/BLE or **hold the user/menu button for the first 3 seconds during boot**. Do not confuse the menu button with double RESET for UF2.
- **APRS symbols:** Two independently configurable two-character codes, e.g. **moving `/>`**, **parked `\P`**. The first character is the APRS symbol table (`/` or `\`); the second is the APRS symbol.
- **Startup TX:** With AUTO enabled, valid callsign, fresh GPS fix, and Comment/Status intervals enabled, send **position + Comment in one packet**, then a separate **Status** subject to the **15-second common LoRa transmit guard**. Comment is not a standalone RF frame.
- **Smart Beacon:** Six configurable speed-dependent intervals; parked detection and optional battery voltage in position comment.
- **WX:** Optional BME280 sensor required; weather beacons are separately scheduled.
- **Messaging:** APRS messaging is **not implemented** in V0.9.22.

## 5. Direct Flash A/B and USB serial

At **115200 baud**, run `p` or `flash?` to inspect the configuration. On a device with **no valid A/B record**, advanced users may initialize settings via USB `flash init` **after backing up**; do not issue this during routine updates or when existing Direct Flash is already ACTIVE.

| Command | Function |
|---|---|
| `p` / `flash?` | Status / Direct Flash record status |
| `chip sx1262` / `chip sx1268` | Select radio chip (reboot required) |
| `symbol />` / `parksymbol \P` | Two-character moving / parked APRS symbols |
| `a on` / `a off` | Automatic beacons |
| `f smart` / `f fixed` | Position mode |
| `smart 600,90,45,30,20,15` | Six Smart Beacon intervals (seconds) |
| `power 10` | Target TX power in dBm (1–22; not guaranteed RF output) |
| `t` / `w` / `v` | Manual position / weather / status requests |

## 6. Build from Arduino source

Arduino IDE with **Adafruit nRF52 core 1.7.0** and the project-specific **TA1FM MiniMesh Lite NRF** board variant. Libraries: **RadioLib 7.8.1**, Adafruit TinyUSB (core), Adafruit GFX, Adafruit SH110X, Adafruit SSD1306, TinyGPSPlus, Adafruit BME280, Adafruit Unified Sensor, and core InternalFileSystem/flash_nrf5x support (the filesystem itself is not started). A mismatched board variant or RF switch mapping can prevent normal operation.

For more commands and troubleshooting see [English User Guide](./USER_GUIDE_EN.md); for hardware wiring see [Pin Connections (Turkish)](./PIN_BAGLANTILARI_TR.md).

## 7. RF safety

Use only lawful amateur-radio frequencies, callsigns and power for your jurisdiction. Never transmit without an antenna. Confirm your RF amplifier and radio module limits before raising TX power.

**73 de TA1FM**
