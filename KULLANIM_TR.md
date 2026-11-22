# TA1FM nrf52 Tracker + Digipeater — Kullanım Rehberi (v1.2.0 STABLE REBUILD1)

[Kaynak / INO](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino) · [Kurulum](./KURULUM_TR.md) · [Seri Komutlar](./SERI_KONSOL_KOMUT_REHBERI_TR.md) · [Pinler](./PIN_BAGLANTILARI_TR.md)

## 1. Çalışma modları

| Mod | İşlev |
|---|---|
| **TRACKER** | GPS konum, SMART/FIXED beacon, hareket/park sembolleri, Comment/Status/WX ve isteğe bağlı telemetri |
| **DIGI** | Sabit koordinatlar, WIDE yoluna uygun LoRa APRS paket tekrarları ve kendi beacon'ı |

Frekans **433.775 MHz**, **SF12 / BW125 / CR4/5**. Uyumlu nRF52840 MiniMesh, SX1262/SX1268, SH1106/SSD1306 ve isteğe bağlı BME280 kullanılır. DIGI modu **APRS-IS iGate değildir**.

## 2. İlk çalıştırma ve birleşik panel

1. Anteni bağlayın; firmware yükleyin. Yeni cihazda ayar alanları otomatik başlatılır; `flash init` gerekmez.
2. İlk kurulumda `NOCALL` normaldir ve **APRS RF gönderimi engellenir**. Kendi lisanslı çağrı işaretinizi kaydedin.
3. Güncel v1.2.0 **birleşik HTML web panelini** Chrome/Edge üzerinden USB veya Web Bluetooth üzerinden BLE ile açın. Configurator deposundaki [eski v1.1.0 panelinin](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker-Configurator) en yeni kontrolleri içermeyebileceğini unutmayın.
4. **Bir kez BLE/USB bağlanın; sekmeler arasında bağlantı aynen korunur.** Üstte **Ayarları Oku**, değişiklikleri yapınca **Ayarları Kaydet**. BLE için **Kaydetme Açık** izni gerekir.
5. SX1262/SX1268, GPS FIX, LoRa RX, batarya gerilimi, OLED ve otomatik beacon ayarlarını kontrol edin.

## 3. GPS, SMART/FIXED, Status ve WX

- **SMART:** Hız ve dönüş koşullarına göre konum aralığı. **FIXED:** seçilen dakika aralığı. GPS FIX veya güncel hız verisi yoksa hatalı hız/yön tahmin edilmez.
- **GNSS baud:** Geçerli NMEA checksum bilgisiyle otomatik algılama; GNSS tanı komutları `gnss?` / `gnss baud?`.
- **Sembol:** Normal hareket ve park APRS sembolleri ayrı ayarlanır. Bu sürümün OLED hazır sembol seçenekleri **Car / Walker / House**; Motorcycle değişikliği gelecekte planlanmıştır.
- **Comment:** Zamanı gelince konum paketinde yer alır, ayrı paket oluşturmaz. **Status:** bağımsız `>` APRS paketidir.
- **WX:** İsteğe bağlı BME280 ile sıcaklık/nem/basınç bilgisi; sensör yokken hayali değer üretilmez.
- **RF TX:** Ortak **10 saniye** koruma; `RadioLib TX OK` sadece yerel gönderim sonucudur.

## 4. Serbest APRS mesajlaşması ve ACK

1. Birleşik panelde **Mesajlar** sekmesine geçin.
2. Tam alıcı CALL-SSID ve ASCII metin girin; **1/2/3 toplam RF deneme** seçin.
3. `QUEUED`, `WAIT_ACK`, `ACK_RECEIVED` veya zaman aşımını takip edin.
4. **Karşı istasyon ACK gönderirse mesajı cihazın aldığı doğrulanır; insanın okuduğu kanıtlanmaz.**
5. Son **3 gerçek gelen APRS mesajı RAM'de** tutulur; OLED ve USB/BLE gelen kutusunda görünür. Cihaz kapanınca silinir. APRS.fi'de görünmesi RF paketinin uygun iGate'e ulaşmasına bağlıdır.

## 5. OLED üzerinden Hızlı Mesajlar (QUICK MSG)

- Panelden hedef alıcı çağrı işaretini ve mesaj listesini kaydedin. Örnek:

~~~text
Yoldayim|Konumuma bak|Eve donuyorum|OK|BYE
~~~

- Sınır: **6 mesaj**, aralarındaki `|` dahil **200 ASCII byte**, mesaj başına **62 karakter**.
- OLED'de menüden **QUICK MSG** açılır. Kısa basışla mesaj seçilir; uzun basışla önizlenir ve ikinci uzun basışla gönderim onaylanır.
- Gönderimde RF üzerinden otomatik ACK isteği kullanılır; hazır mesaj TX'i **en fazla 2 toplam deneme** yapar.
- **Hazır mesajlar kalıcı Direct Flash A/B kaydındadır.** Gelen mesaj geçmişi ise RAM'dedir.

## 6. Yeni mesaj bildirimi: OLED ve buzzer

- Yeni, cihaza adresli ve gerçek APRS mesajında OLED uyandırılır; panelden **15, 30 veya 60 saniye** seçilir. Önceden uyuyorsa süre bitince uyku moduna dönebilir.
- Aktif 3,3 V buzzer için GPIO **6 (P0.06)** hedef MiniMesh üzerinde test edilmiştir; bildirimi **bir uzun + iki kısa bip** olarak gözlemlenmiştir. Yazılım GPIO **8, 33, 34, 39** seçeneklerini de kabul eder; kartın fiziksel pin bağlantılarını doğrulayın.
- Buzzer bağlanmasa bile OLED mesaj bildirimi çalışabilir. Normal konum/Status veya tekrarlanan APRS mesajları için yeni mesaj bildirim sesi amaçlanmaz.
- Buzzer GPIO ve OLED bildirimi **panelden kalıcı kaydedilir**. Seri `notify?` ve `buzzer test` kontrol edilebilir.

## 7. Base91 APRS telemetri

- `telem on`: geçerli konum paketine Base91 telemetri ekler (pil voltajı, BME280 sıcaklık/nem/basınç). `telem off`: kapatır.
- `telem defs`: istasyonun `PARM.Batt,Temp,Hum,Press,Spare`, `UNIT.V,C,%,hPa,-` ve `EQNS...` tanımlarını RF üzerinden sırayla gönderir.
- **Telemetri aktif/pasif durumu RAM'dedir**, cihaz yeniden açılınca tekrar açılması gerekir. `telem?` ile sorgulayın.
- aprs.fi'de çözümleme için paketlerin APRS-IS'e erişmesi gerekir. İlgili telemetri kullanıcının saha testlerinde aprs.fi'de gösterilmiştir.

## 8. Sabit DIGI modu

- Geçerli çağrı işareti ve iki sabit koordinat olmadan DIGI yapılandırmasını tamamlamayın.
- `digi lat 40.97500` / `digi lon 27.50400`, `digi path 1|2`, `digi beacon 10|15|30|60` ayarlarını kullanın.
- WIDE tekrarları, mükerrer filtrelemesi ve paylaşılan RF TX koruması vardır. Kapsama, anten ve RF ortamına bağlıdır.

## 9. USB, BLE ve yazılım güncellemesi

- USB seri: **115200 baud**. Aynı COM portunu hem Seri Monitör hem web paneliyle eşzamanlı açmayın.
- BLE reklam adı: `nRF52_Lora_Tracker`; panel yazma izni **şifre değil**, kazara yazmayı önleyen kontrol mekanizmasıdır.
- Sürüm: `version?` → `VERSION|1.2.0|STABLE_REBUILD1`; BLE `ping` → `PONG|TA1FM|V1.2.0_STABLE_REBUILD1`.
- BLE OTA: Panelle DFU'ya girip Android **Nordic DFU** uygulamasında [ZIP](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.zip) yükleyin. **PRN 4**, **High MTU OFF** tavsiye edilen test ayarıdır. Mevcut DFU onay akışı korunmuştur; **tek onay** ileriki sürüm planıdır.
- USB UF2 kurtarma/güncelleme için [UF2 dosyası](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.uf2). Bootloader dosyalarını uygulama UF2 ile karıştırmayın.

## 10. Sorun giderme

| Belirti | Kontrol |
|---|---|
| GPS FIX yok | Dış ortam, güç, UART, `gnss?`, `gnss baud?` |
| LoRa RX/TX yok | Gerçek SX1262/SX1268 seçimi, anten, TCXO/RF anahtarı |
| BLE yok | Bluetooth izni, HTTPS, reklam, bağlantı/enerji |
| Mesaj RAM'den silindi | Normal davranış: 3 mesaj, yeniden başlatmada sıfırlanır |
| Buzzer çalışmıyor | GPIO, aktif 3,3 V model, bildirim açık/kapalı, `buzzer test` |
| OLED'de hazır mesaj yok | Panel kayıt durumu, `preset?`, hedef CALL-SSID |
| İlk kurulum ayar kaydı yok | `flash?` ve `[DIRECT] AUTO INIT` tanı satırlarını kontrol edin |
| DFU takıldı | Bootloader/SoftDevice, AdaDFU görünürlüğü, kararlı güç; USB UF2 kurtarma |

**73 de TA1FM! 📡**
