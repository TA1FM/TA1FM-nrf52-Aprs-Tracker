# TA1FM nRF52 APRS Tracker — English installation

## Supported target
Built for the **TA2DMX MiniMesh Lite NRF**, a custom **nRF52840** board using a nice!nano-compatible UF2 bootloader and Nordic **S140 6.1.1** on the tested hardware. The UF2 binary is **not universal** across nRF52840 boards.

## Flash the pre-built firmware
1. Back up your currently working firmware and config.
2. Check your bootloader and hardware compatibility.
3. Double-tap RESET to mount the UF2 USB drive.
4. Copy `TA1FM_nrf52_Tracker_V0_9_2_STABLE.uf2` to the drive.
5. After restart, verify GPS, LoRa and persisted settings.

**Do not use a flash eraser or format LittleFS for a normal update.**

## Configure without serial commands
Open the [TA1FM Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/) in **desktop Chrome/Edge**, connect via USB, read settings, edit values, write to the tracker and optionally export/import JSON backups.

## Build from Arduino source
Arduino IDE with **Adafruit nRF52 core 1.7.0** and the project's custom **TA1FM MiniMesh Lite NRF (TA2DMX)** board definition/pin variant. Libraries: **RadioLib 7.8.1**, Adafruit TinyUSB (from core), Adafruit GFX, Adafruit SH110X, Adafruit SSD1306, TinyGPSPlus, Adafruit BME280, Adafruit Unified Sensor and Adafruit LittleFS/InternalFileSystem (from core). Select the matching custom board variant, not a generic nRF52840 board.

## USB serial quick commands (115200 baud)

| Command | Function |
|---|---|
| `p` | Print device/config status |
| `a on` / `a off` | Enable/disable AUTO, saved persistently |
| `f smart` / `f fixed` | Position beacon mode |
| `i 1|3|5|10|15` | Fixed position interval in minutes |
| `x off|15|30|60` | WX interval in minutes |
| `z off|15|30|60` | Status interval in minutes |
| `j off|15|30|60` | Comment schedule (comment is included in positions) |
| `k 1|3|5` | Parking confirmation wait in minutes |
| `s car|walker|house` | Supported APRS symbols in V0.9.2 |
| `comment YOUR TEXT` | Set position comment |
| `status YOUR TEXT` | Set status text |
| `comment?` / `status?` | Query saved texts |
| `t` / `w` / `v` | Manually send position / WX / Status |

Valid callsign required for TX; GPS fix required for position. An initial valid callsign can turn AUTO on, while existing explicitly saved OFF remains OFF on subsequent boot. If necessary, enable AUTO in the web UI or run `a on`; it survives reboot after saving.

**Motorcycle symbol support is not present in V0.9.2.** That is planned for a future version. The version supports car, walker and house.

## RF compliance
Use only where the operating frequency, transmit power and amateur-radio authorization permit. Always use your own callsign. This project release is intended for the specified hardware; verify before flashing.
