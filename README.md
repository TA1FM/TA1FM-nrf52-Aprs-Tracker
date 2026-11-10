# TA1FM nRF52 LoRa APRS Tracker

**Firmware: V0.9.2 STABLE · Hardware: TA2DMX MiniMesh Lite NRF (nRF52840)**

> **Türkçe:** TA1FM-7 için geliştirdiğimiz APRS konum ve meteoroloji (WX) takip yazılımı. Bu sürüm, gerçek cihazda konum TX/RX, WX, GPS, AUTO Beacon, LittleFS kalıcı ayarlar ve USB Web Configurator ile test edilmiştir.
>
> **English:** LoRa APRS position and weather tracker firmware for the TA2DMX MiniMesh Lite NRF nRF52840 board. Position TX/RX, weather packets, GPS, AUTO beacons, persistent LittleFS settings and USB web configuration were tested on the project hardware.

## 🌐 Web Configurator / Web ayar paneli

**[Open the TA1FM Web Configurator / Web Configurator'ı aç](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)**

Use **Chrome or Edge on desktop**, connect the board by USB, and choose **Connect**. You can read settings, modify and write them, and save/restore JSON backups. Serial monitor is included. No need to type serial commands for normal configuration.

Masaüstü Chrome veya Edge üzerinden USB'ye bağlayın; ayarları okuyun, değiştirin, cihaza yazın ve JSON yedeği alın. Normal kullanımda seri komut girmeniz gerekmez.

## 📦 Files / Dosyalar

| File | Description |
| --- | --- |
| `TA1FM_nrf52_Tracker_V0_9_2_STABLE.uf2` | Tested UF2 firmware for supported board / Test edilmiş UF2 |
| `TA1FM_nrf52_Tracker_V0_9_2.ino` | Arduino source code / Arduino kaynak kodu |
| `README.md` | Main guide / Ana rehber |
| `KURULUM_TR.md` | Turkish installation and command reference |
| `INSTALL_EN.md` | English installation and command reference |

## ⚠️ Hardware compatibility / Donanım uyumu

**This firmware is hardware-specific. Do not flash it to an arbitrary nRF52840 board.**

- Board: **TA2DMX MiniMesh Lite NRF**, nRF52840, custom TA1FM board variant.
- UF2 bootloader: nice!nano / nRF52840 (`Board-ID: nRF52840-nicenano`), **SoftDevice S140 6.1.1** on the tested board.
- LoRa: **SX1262**, 433.775 MHz, SF12, BW125, CR 4/5; TCXO 1.8 V.
- GPS: **L76K**; environmental sensor: **BME280** (WX when present).
- OLED: SH1106 / SSD1306 variants supported by the sketch.
- Sketch is designed for a custom **Adafruit nRF52 core 1.7.0** board definition and uses **RadioLib 7.8.1** and the libraries listed in the source.

**UF2 firmware flashing:** Double-tap RESET to show the UF2 USB drive, then copy the `.uf2` file. Ensure bootloader/SoftDevice compatibility first. **Do not format the device or use an eraser to perform a normal version update.** Keep a backup of your working firmware and configuration.

**UF2 yükleme:** RESET düğmesine iki kez basarak UF2 USB sürücüsünü açın ve `.uf2` dosyasını kopyalayın. Bootloader ve kart uyumunu önceden kontrol edin. Normal güncellemede silici veya format işlemi yapmayın.

## ✅ Tested features / Test edilen özellikler

- GPS location beacons and Smart/Fixed position intervals
- APRS LoRa RF TX/RX with last-heard station on OLED
- BME280 weather reports (WX) and separate WX/Status schedules
- Parked detection and APRS status/comment messages
- AUTO mode saved to flash; restored after a full power cycle
- LittleFS safer record replacement for persistent configuration
- USB serial commands and Web Configurator read/write

**Important:** On an existing device, AUTO follows the previously saved state. If AUTO is OFF, enable it in Web Configurator or with `a on`, then verify it survives a power cycle. First valid callsign entry can enable AUTO automatically; `NOCALL` cannot transmit.

**Önemli:** Daha önce kaydedilmiş AUTO=OFF ayarı güncellemeden sonra OFF kalabilir. Web Configurator üzerinden AUTO ON yapın; cihaz kapatılıp açıldığında ayar korunur.

## Legal and RF / Yasal uyarı

Transmit only with the appropriate amateur-radio authorization, callsign, and within permitted frequencies/power limits in your jurisdiction. Verify your local amateur-radio rules before operating. This firmware is provided as a project release, without any guarantee of suitability for other hardware.

## Source license / Kaynak kod lisansı

No open-source license has been selected or granted by this repository package. If you intend to permit public reuse, modification or redistribution, add an appropriate `LICENSE` file after choosing its terms. Publishing source code alone does not automatically grant those rights.

**Project ID:** TA1FM · **Release:** V0.9.2 STABLE
