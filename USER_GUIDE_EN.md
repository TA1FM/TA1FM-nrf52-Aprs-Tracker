# TA1FM nrf52 Tracker + Digipeater — User Guide (v1.2.0 STABLE REBUILD1)

[Installation](./INSTALL_EN.md) · [Firmware](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino) · [Serial commands](./SERIAL_CONSOLE_COMMAND_GUIDE_EN.md) · [Pinout (TR)](./PIN_BAGLANTILARI_TR.md)

## 1. Modes and radio

| Mode | Description |
|---|---|
| **TRACKER** | GNSS APRS position, SMART/FIXED beacons, moving/parked symbols, Comment, Status, optional WX and telemetry |
| **DIGI** | Fixed-position LoRa APRS WIDE packet repeating with scheduled own beacon; **not** an internet APRS-IS iGate |

Hardware: compatible nRF52840 MiniMesh Lite NRF, SX1262/SX1268, SH1106/SSD1306, optional BME280; **433.775 MHz, SF12, BW125, CR4/5**.

## 2. Initial setup and unified control panel

- Install UF2 or BLE DFU firmware using the [installation guide](./INSTALL_EN.md). On the first boot, persistent settings auto-initialize; **no manual `flash init` command**.
- An unconfigured device reads **`NOCALL`** and **does not transmit APRS** until you save a valid authorized callsign.
- Use the **v1.2.0 unified USB/BLE tabbed HTML panel** for all new features. The [older Configurator repository pages](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker-Configurator) may not expose new controls.
- Connect once using USB (desktop Chrome/Edge) or BLE (Web Bluetooth/HTTPS); the same connection works across **Status, General, APRS, Radio, Messages, OLED, Telemetry and System** tabs.
- **Read Settings**, edit, enable BLE save permission if applicable, **Save Settings**, then read back to verify. Save a JSON backup when changing important parameters.

## 3. Tracker operation, GNSS and beacons

- SMART adjusts location reports using speed and turns; FIXED uses a selected time interval.
- GNSS automatically detects a valid NMEA baud rate; position transmissions need a valid fix. A cold GNSS start may take time. Inspect `gnss?` / `gnss baud?`.
- Current moving and parked APRS symbols are configurable separately. The built-in **OLED quick symbol** choices are still **Car / Walker / House** in this release. The Motorcycle replacement is a **future change**.
- Fresh valid GPS course/speed can appear as APRS `ccc/sss`. Comment is part of eligible position frames; Status is a separate APRS `>` packet.
- Weather (WX) requires actual BME280 data. RF TX uses a shared **10-second guard**. Local `RadioLib TX OK` does not confirm a remote station received the frame.

## 4. APRS messages and ACK

- In **Messages**, enter a destination callsign including SSID and a message. Choose **1, 2 or 3 total RF attempts**.
- Follow queued, sent, ACK received or timeout statuses. An ACK means **device-level receipt, not human reading**.
- The last **3 received addressed messages** are kept in **RAM only**, can be read via OLED/USB/BLE, and disappear on restart. APRS.fi receives only RF traffic forwarded by an iGate.

## 5. OLED quick/preset messages

1. In the panel, configure a **single target CALL-SSID** and a `|`-delimited list, e.g.:

~~~text
ON MY WAY|POSITION? |HOME SOON|OK|BYE
~~~

2. Save the list to the **existing Direct Flash A/B configuration record**; these outgoing message presets survive reboots.
3. Use the physical button to open **QUICK MSG** on OLED. Short press selects, long press previews, another long press confirms/sends.
4. Limits: **6 presets**, **200 ASCII bytes** for the whole list including `|` separators, **62 ASCII characters per message**.
5. Quick-message sends request RF ACK and allow at most **2 total RF attempts**.

## 6. Incoming notification and active buzzer

- A new real APRS message addressed to this device can wake the OLED; notification time **15/30/60 seconds**. Normal position packets do not trigger a message alert.
- An optional **3.3 V active buzzer** can be enabled. **P0.06 / Arduino GPIO 6** was tested successfully on the target MiniMesh; user testing observed **one long and two short beeps**. Other allowed GPIO numbers: **8, 33, 34, 39**, subject to board-specific hardware verification.
- Configure buzzer pin, buzzer enable, OLED wake and duration in the unified panel; they are persistent configuration values. The `buzzer test` command checks the output.

## 7. Base91 position telemetry

- Enable at runtime with `telem on` to add battery voltage and available BME280 temperature/humidity/pressure to APRS position frames. `telem off` disables.
- `telem defs` queues the APRS `PARM`, `UNIT` and `EQNS` definitions. These are not automatically sent at boot.
- **Telemetry enablement is RAM-only** and resets after a reboot; presets and other saved settings remain persistent.
- Actual visualization on aprs.fi has been confirmed in project tests, subject to LoRa iGate reception.

## 8. Digipeater mode

Set a valid callsign, fixed latitude and longitude, then use `mode digi`, `digi path 1|2` and `digi beacon 10|15|30|60`. Keep the antenna connected before RF transmissions. Digipeater RF repeats are not APRS-IS uploads.

## 9. Connectivity and firmware updates

- USB uses **115200 baud**. Do not share a single COM port between Serial Monitor and the USB web panel.
- BLE advertising name: **`nRF52_Lora_Tracker`**; the save/write toggle is not strong security authentication.
- USB `version?` → `VERSION|1.2.0|STABLE_REBUILD1`. BLE `ping` → `PONG|TA1FM|V1.2.0_STABLE_REBUILD1`.
- USB UF2: [firmware UF2](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.uf2). BLE DFU: [firmware ZIP](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.zip) via Android Nordic DFU after entering **AdaDFU** mode.
- Previously tested DFU settings: **PRN 4, High MTU OFF**. The older two-step DFU flow remains in v1.2.0. Single-confirmation entry is on the future work list.

## 10. Troubleshooting

| Problem | Remedy |
|---|---|
| First-run `NOCALL` | Set/save your licensed callsign |
| No GPS FIX | GNSS power, UART, sky view, `gnss?` |
| LoRa TX/RX failure | Chip/SPI/RF switch/TCXO/antenna and restart |
| OLED message presets missing | Check `preset?` and whether the list was saved |
| No buzzer | Check configured GPIO, 3.3 V active buzzer, `notify?` and `buzzer test` |
| Missing incoming messages after power cycle | Expected: only 3-message RAM inbox |
| DFU bootloader not advertising | Check bootloader/SoftDevice compatibility; recover with correct UF2 if necessary |

**73 de TA1FM!**
