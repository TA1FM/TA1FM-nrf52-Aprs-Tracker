# TA1FM nrf52 Tracker — USB Serial Console Commands (v1.2.0 STABLE REBUILD1)

**USB serial: 115200 baud, Newline line ending.** [Installation](./INSTALL_EN.md) · [User guide](./USER_GUIDE_EN.md)

Based on the [v1.2.0 STABLE REBUILD1 Arduino source](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino). Close the USB web configurator before opening the same COM port in Serial Monitor. Substitute your authorized callsign for examples.

## 1. Read-only status and diagnostics

| Command | Function |
|---|---|
| `version?` | `VERSION|1.2.0|STABLE_REBUILD1` |
| `p` / `?` | Complete state and command summary |
| `g` / `r` / `b` | GNSS / LoRa / BME280 status |
| `gnss?` / `gnss baud?` | GNSS diagnostics / auto-baud status |
| `gnss debug` / `gnss gsv on` | Extra GNSS debug |
| `gnss probe` | GNSS model probe (use carefully) |
| `chip?` / `power?` | Radio variant / power |
| `mode?` / `digi?` | TRACKER/DIGI settings |
| `smart?` | SMART beacon rate table |
| `flash?` | Persistent Direct Flash A/B status |
| `preset?` | OLED quick message configuration |
| `notify?` | Buzzer pin and OLED wake setting |
| `telem?` | Base91 telemetry runtime state |
| `oled?` / `comment?` / `status?` | Display and APRS text settings |

## 2. Callsign, radio and mode

| Command | Purpose |
|---|---|
| `c TA1FM-7` | Set an **example** callsign |
| `chip sx1262` / `chip sx1268` | Select radio; a restart may be required |
| `power 1` ... `power 22` | TX power dBm |
| `mode tracker` / `mode digi` | Operating mode |
| `ble on` / `ble off` | BLE advertising |
| `u` | UTC / UTC+3 display toggle |

## 3. Tracker / beacon / symbols

| Command | Purpose |
|---|---|
| `a on` / `a off` | Automatic beacon |
| `f smart` / `f fixed` | SMART or FIXED |
| `i 1` / `i 3` / `i 5` / `i 10` / `i 15` | Fixed beacon interval (minutes) |
| `smart 600,90,45,30,20,15` | SMART speed-rate intervals (seconds) |
| `motion on` / `motion off` | First-motion beacon |
| `batt on` / `batt off` | Battery value in position packets |
| `k 1` / `k 3` / `k 5` | Parking time (minutes) |
| `x off` / `x 15` / `x 30` / `x 60` | Weather interval |
| `z off` / `z 15` / `z 30` / `z 60` | Status interval |
| `j off` / `j each` / `j 15` / `j 30` / `j 60` | Comment timing |
| `comment TEXT` / `status TEXT` | APRS text |
| `symbol />` / `parksymbol \\P` | Moving / parked symbol |
| `s car` / `s walker` / `s house` | Three current OLED presets (**motorcycle is planned**) |
| `t` / `w` / `v` | Manual position / weather / Status RF TX |

Keep the antenna connected when transmitting. Shared RF TX guard is at least 10 seconds.

## 4. DIGI mode

| Command | Purpose |
|---|---|
| `digi lat 40.97500` / `digi lon 27.50400` | Fixed latitude / longitude |
| `digi path 1` / `digi path 2` | WIDE path policy |
| `digi beacon 10` / `15` / `30` / `60` | DIGI beacon interval (minutes) |
| `mode digi` | Requires callsign and both coordinates |

## 5. OLED and APRS messages

| Command | Purpose |
|---|---|
| `oled sh1106` / `oled ssd1306` | OLED driver preview |
| `oled confirm` / `oled cancel` | Confirm/cancel OLED preview |
| `oled wake` / `sleep 0` ... `sleep 6` | Wake / sleep mode |
| `msg hello` / `msg live` | Messaging capability/live status |
| `msg inbox` / `msg list` | **Last 3** received messages (RAM) |
| `msg status` / `msg?` | Outbound status / ACK |
| `msg send TA1FM-7 HELLO` | One total RF attempt |
| `msg send2 TA1FM-7 HELLO` | Up to two attempts |
| `msg send3 TA1FM-7 HELLO` | Up to three attempts |
| `msg cancel` / `msg clear` | Cancel outgoing / clear RAM inbox |
| `testmsg TA1FM-7 TEST` | Simulated incoming message, **no RF** |
| `msg flash?` | Legacy read-only note: messages use RAM, not flash |

### Persistent OLED quick messages

Use the **unified web panel** where possible. Advanced transfer protocol:

- `preset?` — preset status; `preset get 0` ... `preset get 3` — read list parts.
- `preset begin CALL LEN` — begin writing; `preset part INDEX CHUNK` — upload each indexed part; `preset commit` — verify/save; `preset cancel` — abort.

Limits: **6** messages, **200 ASCII bytes including `|` delimiters**, each **≤62 characters**, one target CALL-SSID. Presets are stored with existing A/B configuration, **not in the incoming-message RAM inbox**.

## 6. Buzzer and OLED notification

| Command | Purpose |
|---|---|
| `notify?` | Persistent notification settings |
| `notify save 6 1 1 30` | GPIO 6 buzzer ON, OLED wake ON, 30 seconds; save |
| `notify save -1 0 1 30` | Disable buzzer, leave OLED wake on |
| `buzzer test` | Test the active buzzer |
| `buzzer?` / `buzzer on` / `buzzer off` | Runtime settings (not persistent) |

Allowed GPIO: **6, 8, 33, 34, 39**, or **-1** for no buzzer. **GPIO 6 (P0.06)** was tested on the target MiniMesh hardware. OLED wake durations: **15, 30, 60 seconds**. Verify the actual PCB and use a suitable active 3.3 V buzzer.

## 7. Base91 APRS telemetry

| Command | Purpose |
|---|---|
| `telem on` / `telem off` | Enable/disable telemetry (RAM-only state) |
| `telem?` | Runtime state |
| `telem defs` | Queue PARM / UNIT / EQNS (3 RF definition frames) |
| `telem cancel` | Cancel pending definitions |

Telemetry state resets after reboot. Separate definition packets are **not automatically transmitted on startup**.

## 8. Firmware / settings storage

| Command | Purpose |
|---|---|
| `dfu` | Request AdaDFU bootloader from USB |
| `flash?` | Inspect Direct Flash A/B settings |
| `flash init` | **Legacy advanced command**: not needed for normal v1.2.0 REBUILD1 first installation; the first boot auto-initializes |

Existing valid A/B settings are preserved at boot. First boot uses `NOCALL` and disables APRS RF transmission until configured. BLE DFU uses a `dfu arm` 15-second window then `dfu go`; the current UI uses an approval flow, and single-confirmation DFU is **future work**. Android Nordic DFU uses the **application ZIP**, PRN **4**, High MTU **OFF** on the previously tested bootloader.

**73 de TA1FM**
