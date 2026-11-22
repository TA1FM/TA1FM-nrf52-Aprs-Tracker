# TA1FM nrf52 Tracker — Installation (v1.2.0 STABLE REBUILD1)

[Source INO](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino) · [USB UF2 firmware](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.uf2) · [BLE DFU ZIP](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.zip) · [User guide](./USER_GUIDE_EN.md) · [Serial commands](./SERIAL_CONSOLE_COMMAND_GUIDE_EN.md)

## 1. Supported hardware

This firmware targets a **compatible TA2DMX MiniMesh Lite NRF / nRF52840 pin layout**, using an **SX1262 or SX1268**, GNSS receiver, 128×64 SH1106/SSD1306 OLED, and optionally a BME280.

- APRS LoRa: **433.775 MHz, SF12, BW125, CR 4/5**. Check local frequency/license regulations.
- Verify radio SPI wiring, RF switch, TCXO, GPIO mapping, SoftDevice and UF2 bootloader before uploading to an unfamiliar board. It is **not a universal nRF52840 firmware image**.
- Connect a suitable antenna before any RF TX. Back up existing firmware and settings before upgrading.
- Pinout: [TR hardware guide](./PIN_BAGLANTILARI_TR.md).

## 2. First installation or recovery via USB UF2

1. Download the actual [`TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.uf2`](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.uf2) file from this repository. The filename ends in `.ino.uf2`, but it is a **UF2 binary**, not an Arduino source.
2. Connect the board with a USB data cable; on a compatible UF2 bootloader, quickly trigger RESET twice.
3. Copy the **UF2** to the mounted bootloader volume. Do **not** copy the BLE DFU ZIP there.
4. After reboot, check the OLED version (`V1.2.0`), and send `version?` over 115200 baud USB serial. Expected: `VERSION|1.2.0|STABLE_REBUILD1`.
5. Check LoRa chip (SX1262/SX1268), GNSS, display, and saved configuration.

### Automatic first-boot settings (REBUILD1)

- If valid TA1FM Direct Flash A/B settings exist, they are **loaded without writing anything at boot**.
- Otherwise, once the BLE/SoftDevice stack is running, REBUILD1 automatically creates the initial configuration in the dedicated settings pages **`0xF2000` / `0xF3000`**. Users do **not** need `flash init`.
- The first-boot callsign is **`NOCALL`**: APRS transmission remains disabled until a valid authorized callsign is configured. The default radio chip is SX1262 on a fresh install.
- If another firmware left data in the dedicated settings pages, initialization may replace **that data**. This is **not** a full-chip erase and does not intentionally modify bootloader/SoftDevice areas.
- If initialization fails, inspect `flash?` and the `[DIRECT] AUTO INIT` logs instead of blindly erasing memory.

## 3. USB and Bluetooth web panel

Use the **v1.2.0 STABLE REBUILD1 unified, tabbed HTML panel** for the full feature set. The public [Configurator repository](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker-Configurator) currently contains older panels (including V1.1.0) that may omit new preset, buzzer and telemetry controls; publishing the new panel is a separate repository step.

- **USB:** Desktop Chrome/Edge Web Serial; close Arduino Serial Monitor if it holds the same COM port. Click **USB Connect** then **Read Settings**.
- **BLE:** HTTPS with a Web Bluetooth-capable browser (for example Android Chrome); connect to **`nRF52_Lora_Tracker`**, then **Read Settings**.
- Edit and **Save Settings**, then read back to verify. For BLE writes, enable **Save/Write Permission** first.
- Changing tabs does **not** require a new connection. The BLE write-permission toggle is not cryptographic authentication.
- Set your licensed callsign, actual radio chip, TRACKER/DIGI mode, GNSS/beacon timing, symbols and OLED type.

## 4. Quick messages, buzzer and telemetry setup

**OLED Quick Messages:** In the unified panel, enter a single destination CALL-SSID and a `|`-separated list, up to **6** entries, **200 ASCII bytes total**, max **62 characters per message**. Save. Use the physical button / **QUICK MSG** OLED menu to select, preview, confirm and send; device-level ACK is requested. Quick message presets persist with normal A/B settings.

**Incoming inbox:** Last **3** addressed APRS messages are stored in **RAM only**; they do not survive power cycling.

**Notification:** Real new addressed messages may wake the OLED for **15/30/60 seconds** and trigger a connected active buzzer. **P0.06 / GPIO 6** is confirmed working on the tested MiniMesh. Other firmware-allowed GPIOs: 8, 33, 34, 39. Confirm these are unassigned on your hardware. Use a suitable 3.3 V active buzzer (transistor driver if necessary).

**Base91 telemetry:** Send `telem on` to add battery and optional BME280 readings to APRS position frames. Send `telem defs` for PARM/UNIT/EQNS definitions. Telemetry enablement is **RAM-only** and must be re-enabled after reboot; telemetry packets require iGate coverage for aprs.fi visibility.

## 5. Android BLE OTA firmware updates

**Previously tested environment:** compatible nice!nano UF2 0.10.0 / S140 6.1.1 bootloader. Other bootloader/SoftDevice combinations require verification.

1. Download the **[application DFU ZIP](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.zip)**, not the UF2.
2. Connect to the normal Tracker using the unified BLE web panel, enable the configuration save/write permission.
3. Select **Enter BLE DFU Mode** and confirm. Current firmware uses `dfu arm` (15-second arm window) followed by `dfu go`; **single-confirmation DFU is planned, not implemented**.
4. The normal BLE connection disconnects and compatible bootloader should advertise as **`AdaDFU`**. If the device simply returns to its normal name, investigate DFU startup/bootloader state; do not assume an upload occurred.
5. In Android **Nordic nRF Device Firmware Update**, select **AdaDFU** and the ZIP. Previously successful settings: **PRN = 4**, **Request High MTU = OFF**.
6. Keep power stable until completion, then verify `VERSION|1.2.0|STABLE_REBUILD1` and existing settings.

The web panel **does not transfer the DFU ZIP**; it requests bootloader entry. Alternative: USB console command `dfu`. Legacy single-bank BLE DFU interruption may require USB UF2 recovery. Do not update or erase the bootloader as a routine firmware upgrade.

## 6. Troubleshooting

| Symptom | What to check |
|---|---|
| `NOCALL` on first boot | Expected. Save an authorized callsign before RF TX |
| Settings do not persist | Read `flash?`; check the `AUTO INIT` / save-verification logs |
| No GPS FIX | GNSS supply/UART, sky view, `gnss?`, `gnss baud?` |
| Radio TX/RX fails | Physical SX1262/SX1268, RF switch, TCXO, antenna, restart |
| Web USB unavailable | Close other applications occupying COM port |
| BLE not found | Bluetooth access, HTTPS, connected client, device state |
| Blank OLED | Check SH1106/SSD1306 selection and recovery procedure |
| No AdaDFU | Confirm command response, bootloader compatibility, try safe USB UF2 fallback |
| Missing messages after reboot | Expected: incoming inbox is RAM-only |

Archived V0.9.x documents should not be used as current v1.2.0 installation instructions.
