# TA1FM nrf52 Tracker + LoRa APRS Digipeater

## 🌐 [USB ve BLE Kontrol Paneli — v1.2.0 STABLE](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_v1.2.0_STABLE.html)

**[Kontrol Panelini Tarayıcıda Aç](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_v1.2.0_STABLE.html)** · [HTML dosyası (GitHub)](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker-Configurator/blob/main/TA1FM_Tracker_Control_Panel_v1.2.0_STABLE.html)

**Güncel sürüm / Current stable firmware: v1.2.0 STABLE REBUILD1 — 22 Kasım / November 2026**

 [**USB UF2**](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.uf2) · [**BLE DFU ZIP**](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.zip) · [**Kurulum (TR)**](./KURULUM_TR.md) · [**Installation (EN)**](./INSTALL_EN.md)

**433.775 MHz LoRa APRS** tracker and fixed digipeater for compatible **nRF52840 MiniMesh Lite NRF (TA2DMX pin layout)** hardware. GPS/GNSS, SH1106/SSD1306 OLED, SX1262/SX1268, optional BME280, USB and Bluetooth BLE control.

## 🇹🇷 Türkçe — v1.2.0 özellikleri

- **TRACKER:** SMART/FIXED beacon, GPS konum, hareket/park sembolü, geçerli yön/hız, GNSS otomatik baud tespiti ve FIX kontrolü.
- **Sabit DIGI:** Ayrı TRACKER/DIGI modu, WIDE tekrar, sabit koordinattan zamanlanmış beacon; **internet iGate değildir**.
- **APRS mesajlaşma:** USB/BLE panelinden serbest metin, ACK / sınırlı yeniden gönderim. Gelen **son 3 mesaj RAM'dedir**; güç kesilince silinir.
- **OLED hızlı mesajlar:** Panelde `|` ile ayrılan **en fazla 6 hazır mesaj**, toplam **200 ASCII bayt**, tek mesaj **en fazla 62 karakter**. Alıcı CALL-SSID önceden ayarlanır; OLED/butondan seçilip RF ile gönderilir. Hazır mesajlar mevcut A/B ayar kaydında kalıcıdır.
- **Mesaj bildirimi:** Yeni, cihaza adresli gerçek APRS mesajı geldiğinde OLED otomatik uyanabilir (**15/30/60 sn**). İsteğe bağlı aktif buzzer; **GPIO 6 (P0.06)** MiniMesh üzerinde denenmiştir. GPIO **8, 33, 34, 39** da yazılımda izinlidir; fiziksel kart uyumluluğunu ayrıca doğrulayın.
- **Telemetri:** Konum paketinde isteğe bağlı **Base91** pil, BME280 sıcaklık/nem/basınç; `telem defs` ile PARM/UNIT/EQNS tanımları. Telemetri açık/kapalı durumu RAM'dedir; her açılışta yeniden etkinleştirilir. aprs.fi gösterimi uygun iGate'e bağlıdır.
- **Tek sekmeli birleşik panel:** USB veya BLE bağlantısını bir kez kurup bütün sekmelerde kullanma; canlı bilgiler, APRS, radyo, GPS, mesajlar, OLED, telemetri, sistem.
- **USB UF2 ve Android BLE DFU:** BLE web paneli DFU modunu başlatır; gerçek firmware yüklemesi Nordic DFU uygulamasıyla **ZIP** üzerinden yapılır.

**Not:** OLED sembol kısayolları bu sürümde hâlâ `Car / Walker / House`. Planlanan `Car / Motorcycle / Walker` ve tek onaylı BLE DFU akışı **henüz bu sürüme dahil değildir**.

### Türkçe belgeler

[Kurulum](./KURULUM_TR.md) · [Kullanım](./KULLANIM_TR.md) · [Seri komutlar](./SERI_KONSOL_KOMUT_REHBERI_TR.md) · [Pin bağlantıları](./PIN_BAGLANTILARI_TR.md) · [Sürüm notları](./RELEASE_NOTES_V1_2_0_STABLE_REBUILD1.md)

## 🇬🇧 English — v1.2.0 features

- GPS SMART/FIXED APRS tracking with automatic GNSS baud detection, fresh course/speed and parked-symbol support.
- Selectable fixed-position LoRa digipeater with WIDE path handling (not an APRS-IS iGate).
- APRS messaging over BLE/USB with ACK and bounded retries; **last 3 received messages in RAM only**.
- **OLED quick messages:** up to 6 presets, 200 ASCII bytes total, max 62 characters per message, separated by `|`; fixed destination callsign, selected/sent with the physical button; presets persist in existing A/B config storage.
- Automatic OLED wake on a new addressed APRS message (15/30/60 seconds); optional active buzzer. **P0.06 / GPIO 6** has been tested on the target MiniMesh board.
- Optional **Base91 telemetry** on APRS position frames; battery and BME280 temperature/humidity/pressure with manual PARM/UNIT/EQNS definitions.
- Single tabbed **USB/BLE** interface, SH1106/SSD1306 display, SX1262/SX1268 radio selection.
- Automatic first-run settings initialization. `NOCALL` keeps APRS transmission disabled until configured.
- Wireless DFU using a compatible bootloader and Android Nordic DFU **ZIP** package, or USB **UF2** firmware installation.

**Current-version control panel:** [Open v1.2.0 STABLE USB/BLE Panel](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_v1.2.0_STABLE.html) · [HTML source on GitHub](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker-Configurator/blob/main/TA1FM_Tracker_Control_Panel_v1.2.0_STABLE.html). Earlier v1.1.0 panels lack some v1.2.0 controls.

### English documentation

[Installation](./INSTALL_EN.md) · [User guide](./USER_GUIDE_EN.md) · [Serial commands](./SERIAL_CONSOLE_COMMAND_GUIDE_EN.md)

## Installation / Güncelleme dosyaları

| File | Purpose |
|---|---|
| [`TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino`](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino) | Arduino source. Its internal build identifier is `V1.2.0_STABLE_REBUILD1`. |
| [`TA1FM_nrf52_Tracker_v1.2.0_STABLE.uf2`](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.uf2) | USB UF2 bootloader volume (compatible hardware only). |
| [`TA1FM_nrf52_Tracker_v1.2.0_STABLE.zip`](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.zip) | Android BLE DFU application ZIP (not a USB UF2 file). |

For the exact tested hardware and pin map see [pin guide](./PIN_BAGLANTILARI_TR.md). Bootloader and SoftDevice compatibility must be confirmed; this is **not** a universal nRF52840 image. Use a properly matched antenna and an authorized amateur callsign, and obey local RF rules. **ACK confirms device receipt, not human reading.**

## 📸 Fotoğraflar / Photos

### Tamamlanmış Tracker / Finished tracker

<p align="center"><img src="images/minimesh2.jpg" width="320" alt="TA1FM LoRa APRS Tracker"></p>

### Hardware / Donanım

| MiniMesh kartı | Cihazın içi |
|:---:|:---:|
| <img src="images/deeplabstudio_micromesh_kart.jpg" width="340"> | <img src="images/minimesh1.jpg" width="340"> |

<p align="center"><img src="images/micromesh1.jpg" width="350" alt="MiniMesh Tracker"><img src="images/micromesh2.jpg" width="350" alt="MiniMesh inside"></p>

### Pro Micro DIY wiring / Bağlantı şeması

[![TA1FM nRF52 Tracker wiring diagram](images/TA1FM%20nRF52%20Tracker%20Ba%C4%9Flant%C4%B1%20%C5%9Eemas%C4%B1.png)](images/TA1FM%20nRF52%20Tracker%20Ba%C4%9Flant%C4%B1%20%C5%9Eemas%C4%B1.png)

The DIY diagram is a reference only; verify actual PCB revision, RF switch, TCXO, power design and GPIO. See [pin guide and fakeTec V4 compatibility caveats](./PIN_BAGLANTILARI_TR.md#faketec-v4-pcb-uyumlulugu-ve-diy-kurulum). Original PCB project: [gargomoma/fakeTec_pcb](https://github.com/gargomoma/fakeTec_pcb); TA1FM does not own the third-party design.

## Archived releases / Eski sürümler

Older V0.9.x, V1.0 and V1.1 documentation is kept in Git history and archived directories; use the current installation and user guides above for v1.2.0.

**73 de TA1FM 📡**
