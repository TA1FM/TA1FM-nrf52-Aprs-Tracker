# TA1FM nrf52 Tracker — Kurulum (V1.1.0 STABLE)

**Güncel sürüm:** [V1.1.0 STABLE Release](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.1.0) · [Web paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html) · [Kullanım](./KULLANIM_TR.md) · [Seri komutlar](./SERI_KONSOL_KOMUT_REHBERI_TR.md)

## 1. Hazırlık ve uyumluluk

- Projeye uygun **nRF52840 MiniMesh Lite NRF**, SX1262 veya SX1268, uygun 433.775 MHz anten ve GPS gerekir. OLED SH1106/SSD1306; BME280 isteğe bağlıdır.
- LoRa ayarları **433.775 MHz / SF12 / BW125 / CR4/5**. Cihazın gerçek RF çipini doğru seçin.
- Çalışan firmware'i ve web panelindeki ayar JSON'unu yedekleyin; JSON firmware'in yedeği değildir.
- Bu dosyalar bütün nRF52840 kartlarında çalışmaz: pinler, RF switch, TCXO, SoftDevice ve UF2 bootloader uyumlu olmalıdır.
- **RF yayını öncesinde anteni bağlayın.**

## 2. USB UF2 ile ilk kurulum / kurtarma

1. [Release Assets](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.1.0) bölümündeki **V1.1.0 `.uf2`** dosyasını indirin.
2. Veri aktarabilen USB kablosuyla MiniMesh'i bilgisayara bağlayın.
3. Uyumlu UF2 bootloader'da **RESET'e iki kez** basarak UF2 sürücüsünü açın.
4. `.uf2` dosyasını sürücüye kopyalayın. Cihaz yeniden başlar.
5. OLED'de `V1.1.0` ve USB Seri Monitör'de `V1.1.0 STABLE` bilgisini doğrulayın (115200 baud).
6. Çağrı işaretini, gerçek SX1262/SX1268 seçimini, GPS FIX, RF RX ve kayıtlı ayarları kontrol edin.

**`.zip` dosyasını UF2 sürücüsüne kopyalamayın.** ZIP, Android BLE DFU içindir. Rutin güncellemelerde `flash init`, flash silme veya formatlama kullanmayın.

## 3. USB web panelinden ayar yapma

1. Masaüstü Chrome/Edge üzerinden [V1.1.0 panelini](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html) açın.
2. Aynı COM portunu kullanan Arduino Seri Monitör'ü kapatın.
3. USB ile bağlanın; cihazdan mevcut ayarları okuyun.
4. Çağrı işareti/SSID, TRACKER veya DIGI, radyo çipi, OLED, GPS, SMART/FIXED, Comment, Status ve mesaj ayarlarını kontrol edin.
5. Ayarları kaydedin; yeniden okuyarak doğrulayın. Gerektiğinde JSON yedeğini indirin.
6. **Radyo çipi değişirse yeniden başlatın.** Yanlış OLED tipi seçildiğinde OLED kurtarma/önizleme seçeneklerini kullanın.

## 4. Android Bluetooth BLE ayarları

1. Android Chrome gibi Web Bluetooth uyumlu tarayıcıyla **HTTPS** paneli açın.
2. `nRF52_Lora_Tracker` cihazını seçin; BLE bağlantısını kurun.
3. **Önce cihazdan ayarları okuyun**; ardından **Kaydetme Açık** anahtarını etkinleştirin.
4. Değişiklikleri kaydedip cihazdan tekrar okuyun. Ayar yazma kilidi gerektiğinde kapanır.
5. BLE kaydetme anahtarı **şifreleme veya güçlü kimlik doğrulama değildir**; cihazı güvenli ortamda yapılandırın.

## 5. BLE OTA / telefondan kablosuz firmware güncelleme

**Ön koşul:** Uyumlu bootloader; başarıyla test edilen kombinasyon **nice!nano UF2 bootloader 0.10.0, S140 6.1.1**. Yükleme için Android **Nordic Semiconductor — nRF Device Firmware Update** uygulaması.

1. [Release](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.1.0) sayfasından V1.1.0 **DFU `.zip`** paketini telefona indirin.
2. Normal Tracker açıkken [web paneline](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html) Bluetooth ile bağlanın.
3. **Kaydetme Açık** konumuna geçin ve **BLE DFU Moduna Geç** butonuna basıp onaylayın.
4. Normal BLE bağlantısının kesilmesi beklenir. Cihaz **`AdaDFU`** adıyla yeniden yayın yapar.
5. Nordic DFU uygulamasında ZIP paketini ve **AdaDFU** cihazını seçin.
6. **PRN = 4**, **Request High MTU = OFF (kapalı)** ayarlayın; aktarımı başlatın.
7. Gücü kesmeyin, RESET'e basmayın; tamamlandıktan sonra cihazın normal Tracker modunda açıldığını kontrol edin.

**USB alternatifi:** 115200 baud / Newline seri terminalinden `dfu` komutu da AdaDFU modunu açar.

**Risk:** Legacy **single-bank DFU**, kesintide uygulama alanını silebilir. Güncelleme başarısız olursa USB UF2 ile yeniden yüklemek gerekebilir. Testte aktarım yaklaşık 2,5 dakika sürdü, fakat süre sabit değildir. DFU ZIP dosyası **Web Bluetooth panelinden aktarılmaz**; panel yalnızca DFU modunu açar.

## 6. İlk test ve çalışma modu

- **TRACKER:** Geçerli çağrı işareti, GPS FIX, ANTEN, LoRa RX OK ve otomatik gönderim ayarlarını doğrulayın. Konum paketlerinde geçerli GPS yön/hız bilgisi `ccc/sss` olarak eklenebilir. Comment ayara göre konum paketine eklenir; Status ayrı RF paketidir.
- **DIGI:** `mode digi` öncesi sabit enlem-boylam ve çağrı işareti girin, `digi path 1|2` ve beacon süresini ayarlayın. DIGI modunda sabit koordinatlar kullanılır.
- **Mesajlaşma:** USB/BLE panelinde Mesajlar bölümünü açın; alıcı çağrı işaretini girin, 1/2/3 deneme seçin; ACK, insan tarafından okunduğu anlamına gelmez.
- **Direct Flash A/B + CRC32:** Normal güncellemelerde silmeyin. Gelen 10 APRS mesajı RAM'dedir ve yeniden başlatmada silinir.

## 7. Sorun giderme

| Belirti | Kontrol |
|---|---|
| GPS FIX yok | Anten, uydu görüşü, GPS besleme/UART ve dış ortam |
| LoRa RX ERR | SX1262/SX1268 seçimi, anten, kablo, yeniden başlatma |
| Web panel USB bağlanmıyor | Diğer seri monitör/COM kullanan programı kapat |
| BLE cihazı görünmüyor | Bluetooth izni, BLE açık, HTTPS, güç |
| AdaDFU görünmüyor | Bootloader uyumluluğu; önce USB `dfu` ile doğrula |
| DFU yüzde ilerlerken hata | PRN=4, High MTU OFF; USB UF2 kurtarma gerekebilir |
| OLED boş | SH1106/SSD1306 seçimi ve kurtarma yöntemi |

**Not:** Eski V0.9.x kurulum dosyaları tarihsel arşivdir. Bu belge **V1.1.0 STABLE** içindir.
