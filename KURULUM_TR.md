# TA1FM nrf52 Tracker — Kurulum Rehberi (v1.2.0 STABLE REBUILD1)

**Güncel kaynak:** [INO](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino) · [USB UF2](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.uf2) · [BLE DFU ZIP](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.zip) · [Kullanım](./KULLANIM_TR.md) · [Seri komutlar](./SERI_KONSOL_KOMUT_REHBERI_TR.md)

## 1. Gereksinimler

- TA2DMX MiniMesh Lite NRF ile uyumlu **nRF52840 kart**, uygun SX1262/SX1268 LoRa devresi, GPS/GNSS, 128×64 SH1106/SSD1306 OLED; BME280 isteğe bağlıdır.
- Frekans **433.775 MHz**, LoRa **SF12 / BW125 / CR4/5**. Mevcut kartın güç, RF switch, TCXO, pin dizilimi ve bootloader/SoftDevice uyumluluğunu doğrulayın.
- RF gönderiminden önce **anteni bağlayın**, kendi yetkili amatör çağrı işaretinizi kullanın.
- Mevcut cihazı yükseltirken çalışan firmware'i ve mümkünse paneldeki JSON ayar yedeğini saklayın. JSON, firmware veya tüm flash belleğin yedeği değildir.

## 2. İlk kurulum: USB UF2

1. Bu depodaki [`TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.uf2`](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.uf2) dosyasını indirin. Dosya adı `.ino.uf2` ile bitse de **UF2 firmware**'dir.
2. Uygun UF2 bootloader'lı kartı veri taşıyan USB kablosuyla bilgisayara bağlayın.
3. Uyumlu kartta RESET'i hızlıca iki kez tetikleyip UF2 sürücüsünün açıldığını doğrulayın. Kartın bootloader yönergesine öncelik verin.
4. **UF2** dosyasını açılan sürücüye kopyalayın; kart yeniden başlar. **ZIP dosyasını UF2 sürücüsüne kopyalamayın, bu dosya BLE den güncelleme içindir.**
5. OLED'de `V1.2.0`, USB'de `version?` komutuyla `VERSION|1.2.0|STABLE_REBUILD1` bilgisini kontrol edin.
6. İlk açılışta `NOCALL` görünmesi normaldir: **henüz çağrı işareti kaydedilmediği için APRS RF yayını kapalıdır.** Kartı web panelinden yapılandırın.

### Otomatik ilk açılış ayarları — REBUILD1

- **Geçerli TA1FM A/B ayar kaydı varsa:** otomatik yüklenir, açılış sırasında yeniden yazılmaz.
- **Geçerli kayıt yoksa:** BLE/SoftDevice altyapısı başladıktan sonra `0xF2000` / `0xF3000` ayar sayfalarında otomatik ilk kayıt oluşturulur; `flash init` komutu **gerekmez**.
- İlk kurulumda SX1262 varsayılır; kullandığınız gerçek çip farklıysa SX1268 seçin. GPS, OLED ve radyo çalışmasını ayrı ayrı doğrulayın.
- **Dikkat:** Daha önce başka firmware çalıştırılmış kartta eski veriler bu iki **ayrılmış** sayfanın içindeyse otomatik ilk kayıt onları değiştirebilir. İşlem tüm flash/bootloader/SoftDevice alanını silmez; sadece bu karta uygun bellek yerleşiminde kullanılmalıdır.
- İlk kurulum hatasında `flash?` ile durumu inceleyin; çalışmayan ayar alanını rastgele silmeyin.

## 3. Tek birleşik USB/BLE kontrol paneli

v1.2.0'ın tüm özellikleri için **v1.2.0 STABLE REBUILD1 birleşik HTML panelini** kullanın. [Configurator deposunda](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker-Configurator) eski paneller de vardır; **V1.1.0 paneli**, v1.2.0 hızlı mesaj, buzzer ve telemetri kontrollerinin hepsini içermez. Güncel birleşik panelin ayrı yayımlanması gerekir.

- **USB:** Chrome/Edge'de paneli açın; Arduino Seri Monitör'ün tuttuğu COM portunu önce kapatın. Üstten **USB Bağlan** ve **Ayarları Oku** kullanın.
- **BLE:** HTTPS sayfasını Web Bluetooth uyumlu Chrome ile açın. `nRF52_Lora_Tracker` cihazını seçip bağlanın; üstteki ortak **Ayarları Oku** düğmesiyle bilgileri okuyun.
- **Kaydet:** Değişikliklerden önce mevcut ayarları okuyun. BLE kayıtlarında **Kaydetme Açık** iznini etkinleştirin, **Ayarları Kaydet** deyin, tekrar okuyun.
- Sekme değiştirirken tekrar bağlantı gerekmez. USB ile BLE aynı ayar formunu kullanır. BLE yazma anahtarı **güçlü kimlik doğrulama değildir**.
- Çağrı işaretini/SSID'yi, SX1262/SX1268'i, SMART/FIXED veya DIGI modunu, sembolleri, OLED türünü, beacon sürelerini kaydedin. Gerekirse çip değişiminden sonra yeniden başlatın.

## 4. OLED hazır mesajlar ve bildirim

- Panelin **Mesajlar** bölümüne alıcı CALL-SSID yazın, hazır mesajları `|` işaretiyle ayırın. En fazla **6**, toplam **200 ASCII byte** ve tek mesaj **62 karakter**.
- Kaydettikten sonra OLED **QUICK MSG** menüsüne gidin. Kısa basarak seçin, uzun basarak önizleyip onaylayarak gönderin. Gönderim ve RF ACK sonuçlarını izleyin.
- Hazır mesajlar ve buzzer/OLED bildirim ayarları Direct Flash A/B'de saklanır. **Gelen son 3 mesaj yalnızca RAM'dedir** ve açılışta sıfırlanır.
- Buzzer: panelden **GPIO 6 (P0.06)** seçilebilir; aynı kartta test edilmiştir. GPIO **8, 33, 34, 39** yazılımca izinlidir ama her PCB'de boş olduğunu varsaymayın. 3,3 V uyumlu aktif buzzer kullanın; akıma göre transistör gerekebilir.
- Yeni gerçek, cihaza adresli mesaj geldiğinde OLED **15/30/60 sn** uyanabilir. Normal konum paketi ve tekrar mesajlarda bildirim tetiklenmez.

## 5. APRS telemetri ve GNSS

- `telem on`: konum paketine Base91 pil voltajı, BME280 varsa sıcaklık/nem/basınç ekler. `telem defs`: üç PARM/UNIT/EQNS tanım paketini RF kuyruğuna alır. aprs.fi için paketlerin bir iGate'e ulaşması gerekir.
- **Telemetri aç/kapat ayarı RAM'dedir**, yeniden başlatıldığında tekrar etkinleştirilmelidir. `telem?` ile kontrol edin.
- GNSS, doğrulanmış NMEA verisiyle otomatik baud algılar. GPS FIX yoksa `gnss?` ve `gnss baud?` kullanın. FIX'in gelmesi ilk soğuk başlangıçta zaman alabilir.

## 6. Android BLE OTA DFU

**Ön koşul:** Uyumlu bootloader/SoftDevice. Daha önce test edilmiş örnek: nice!nano UF2 **0.10.0 / S140 6.1.1**; farklı kart ve bootloader uyumluluğunu ayrıca doğrulayın.

1. [`TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.zip`](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.zip) dosyasını telefona indirin.
2. Normal Tracker'a panelden BLE ile bağlanın ve **Kaydetme Açık** iznini etkinleştirin.
3. **BLE DFU Moduna Geç** komutunu onaylayın. Mevcut sürümde `dfu arm` / `dfu go` akışı ve **15 saniyelik** arming süresi vardır; DFU için tek onay geliştirmesi henüz dahil değildir.
4. Cihazın normal BLE bağlantısı kopar. **AdaDFU** reklamını kontrol edin; göremiyorsanız tam güç kesip yeniden açmak bazı durumlarda yardımcı olmuştur, ancak bu garantili bir çözüm değildir.
5. Android **Nordic nRF Device Firmware Update** uygulamasında **AdaDFU** cihazını ve DFU **ZIP**'i seçin. **PRN = 4**, **Request High MTU = OFF** kullanın.
6. İşlem boyunca bağlantıyı ve gücü koruyun. Tamamlandıktan sonra normal yazılım sürümünü ve ayarları kontrol edin.

**Risk:** Legacy single-bank DFU kesintisinde uygulama yeniden USB UF2 ile kurtarılmak zorunda kalabilir. HTML panel firmware ZIP'ini doğrudan yüklemez; sadece DFU moduna geçirir. USB seri terminalinden `dfu` ayrı bir DFU giriş yöntemidir.

## 7. Sorun giderme

| Belirti | Kontrol |
|---|---|
| Yeni cihazda `NOCALL` | Normal ilk kurulum. Geçerli çağrı işaretini kaydedin; otomatik flash başlatma tamamlanmış olmalı |
| Ayarlar kalmıyor | `flash?` ile A/B geçerli kaydı ve yazma hatalarını kontrol edin |
| GPS FIX yok | Dış ortam/anten/GNSS güç-UART, `gnss?` ve `gnss baud?` |
| LoRa RX/TX çalışmıyor | SX1262/SX1268 seçimi, anten, RF switch, TCXO, restart |
| BLE görünmüyor | Güç, reklam adı, HTTPS/Chrome, başka BLE bağlantısı |
| USB bağlanamıyor | COM portunu kullanan Seri Monitör'ü kapatın |
| OLED boş | SH1106/SSD1306 seçimini ve OLED kurtarma ayarını kontrol edin |
| AdaDFU görünmüyor | Önce güncel bağlantı/bootloader durumunu ve DFU yanıtlarını kontrol edin; gerekirse USB UF2 ile yükleyin |

**Not:** Burası güncel v1.2.0 STABLE REBUILD1 rehberidir; V0.9.x rehberleri tarihsel arşivdir.
