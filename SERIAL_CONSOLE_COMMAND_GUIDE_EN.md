# TA1FM nrf52 Tracker — USB Serial Console Command Guide (V1.1.0 STABLE)

**Firmware:** [V1.1.0 STABLE](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.1.0) · **USB Serial:** 115200 baud · **Line ending:** Newline.

This reference covers the V1.1.0 STABLE USB command interface. Close the web configurator before opening Serial Monitor on the same COM port. Example callsigns are placeholders; use your own licensed callsign.

## 1. Device information and diagnosis

| Command | Description |
|---|---|
| `version?` | `VERSION\|1.1.0\|STABLE` |
| `p` / `?` | Detailed state and command summary |
| `g` | GPS fix / satellites / characters |
| `r` | LoRa RX / error / packet count |
| `b` | Optional BME280 values |
| `flash?` | Direct Flash A/B status |
| `chip?` | Selected SX1262/SX1268 |
| `power?` | TX power |
| `smart?` | SMART rate table |
| `mode?` / `digi?` | TRACKER/DIGI and WIDE details |
| `oled?` | OLED settings |
| `comment?` / `status?` | Stored APRS texts |

## 2. Radio, operating mode

| Command | Description |
|---|---|
| `c TA1FM-7` | Save an example callsign |
| `chip sx1262` / `chip sx1268` | Select radio; **restart after change** |
| `power 1` … `power 22` | TX power in dBm |
| `mode tracker` / `mode digi` | Select operating mode |
| `ble on` / `ble off` | BLE enabled / disabled |
| `u` | Toggle UTC and UTC+3 |

## 3. Tracker and beacon control

| Command | Description |
|---|---|
| `a on` / `a off` | Automatic beacon |
| `f smart` / `f fixed` | Position scheduling |
| `i 1` / `3` / `5` / `10` / `15` | Fixed position interval in minutes (prefix each value with `i`) |
| `smart 600,90,45,30,20,15` | Six speed-based intervals, seconds |
| `motion on` / `motion off` | First-motion beacon |
| `batt on` / `batt off` | Append battery voltage |
| `k 1` / `k 3` / `k 5` | Park detection wait (minutes) |
| `x off` / `x 15` / `x 30` / `x 60` | WX interval |
| `z off` / `z 15` / `z 30` / `z 60` | Status interval |
| `j off` / `j each` / `j 15` / `j 30` / `j 60` | Comment interval |
| `comment TEXT` / `status TEXT` | Save APRS text |
| `t` | Manual position TX |
| `w` | Manual WX TX (fresh GPS+BME280 required) |
| `v` | Manual Status TX |
| `symbol />` | Normal APRS symbol |
| `parksymbol \\P` | Parked APRS symbol |
| `s car` / `s walker` / `s house` | Preset symbols |

Fresh GPS course/speed is automatically included in position payloads as APRS `ccc/sss`. Status is considered before routine WX/position; the common **10-second RF guard** remains active.

## 4. Digipeater

| Command | Description |
|---|---|
| `digi lat 40.97500` | Fixed latitude (example) |
| `digi lon 27.50400` | Fixed longitude (example) |
| `digi path 1` / `digi path 2` | WIDE repeat options |
| `digi beacon 10` / `15` / `30` / `60` | Minutes between DIGI beacons |
| `mode digi` | Requires valid callsign and both coordinates |

## 5. OLED

| Command | Description |
|---|---|
| `oled sh1106` / `oled ssd1306` | Preview display driver |
| `oled confirm` / `oled cancel` | Commit / cancel preview |
| `oled wake` | Wake display |
| `sleep 0` … `sleep 6` | Display sleep selection |

## 6. APRS messages

| Command | Description |
|---|---|
| `msg hello` | Message protocol capabilities |
| `msg live` | Live device state |
| `msg inbox` / `msg list` | Received messages |
| `msg status` / `msg?` | Outgoing message/ACK state |
| `msg send TA1FM-7 HELLO` | One total RF attempt |
| `msg send2 TA1FM-7 HELLO` | Two total RF attempts |
| `msg send3 TA1FM-7 HELLO` | Three total RF attempts |
| `msg cancel` | Cancel outbound message |
| `msg clear` | Clear received inbox |
| `testmsg TA1FM-7 TEST` | Local message injection, **no RF TX** |

ACK is a device protocol acknowledgment, **not a human read receipt**. The ten-entry inbox is volatile RAM.

## 7. Firmware and flash

| Command | Description |
|---|---|
| `dfu` | Reboot to **AdaDFU** BLE bootloader |
| `version?` | Report firmware version |
| `flash?` | Query persistent settings storage |
| `flash init` | **Advanced use only**; do not initialize flash during a routine upgrade |

**BLE OTA:** Android Nordic nRF Device Firmware Update: Release **ZIP**, select **AdaDFU**, set **PRN 4**, **Request High MTU OFF**. Tested nice!nano UF2 0.10.0 / S140 6.1.1. Interrupted Legacy single-bank DFU may require USB UF2 recovery; see [Installation](./INSTALL_EN.md).

Older V0.9.x command references are archived and not the current firmware documentation. Attach an antenna before RF TX and follow amateur radio licensing rules.
