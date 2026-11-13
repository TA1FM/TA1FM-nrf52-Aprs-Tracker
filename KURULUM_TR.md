# TA1FM nRF52 APRS Tracker — Kurulum (V0.9.22)

**Sürüm:** TA1FM nrf52 Tracker **V0.9.22 UNIVERSAL BLE**  
**Hedef kart:** TA2DMX MiniMesh Lite NRF / nRF52840, desteklenen LoRa çipi **SX1262 veya SX1268**  
**LoRa APRS:** 433.775 MHz · SF12 · BW125 · CR 4/5

> Bu UF2 her nRF52840 kartına uymaz. Test altyapısında nice!nano uyumlu UF2 bootloader, Nordic S140 6.1.1 ve projeye özel kart pinleri vardır. Yüklemeden önce kart/bootloader ve fiziksel LoRa çipinizi doğrulayın.

## 1. Hazır UF2 dosyasını yükleme

1. Mevcut çalışan UF2'yi ve mümkünse web panelinden JSON ayar yedeğini saklayın.
2. [V0.9.22 UF2 firmware](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_22.uf2) dosyasını indirin. [Arduino kaynak kodu](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_22.ino) da depodadır.
3. Kartın **RESET** tuşuna art arda iki defa basıp bilgisayarda açılan UF2 sürücüsünü bulun.
4. `TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_22.uf2` dosyasını sürücüye kopyalayın. Kart yeniden başlar.
5. Web panelinde çağrı işareti, **Direct Flash ACTIVE** durumu, SX1262/SX1268 seçimi, OLED, AUTO ve APRS sembollerini kontrol edin.
6. **LoRa çipini değiştirdiyseniz yeniden başlatın.** Anteni takılı tutarak GPS FIX ve LoRa RX/TX durumunu doğrulayın.

**Uyarı:** Normal sürüm güncellemesinde silici UF2 kullanmayın, flash'ı formatlamayın. Yeni sürümde ayarlar **Direct Flash A/B + CRC32** kayıtlarıyla tutulur; LittleFS veri dosyası kullanılmaz. Önceki V0.9.14 SX1268 Direct Flash kaydı varsa okunabilir; eski kayıtta açık bir çip seçimi yoksa SX1268 kabul edilir.

## 2. Web paneli: USB ve Bluetooth BLE

**[TA1FM nrf52 Tracker — USB + BLE Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)**

### USB

1. Masaüstü **Chrome/Edge** açın ve USB veri kablosunu bağlayın.
2. **Cihaza bağlan → Cihazdan oku (p)** ile ayarları yükleyin.
3. Değişiklikleri yapıp **Ayarları cihaza yaz** seçin; kayıt mesajından sonra tekrar okuyun.
4. Seri Monitör aynı COM portuna bağlıysa kapatın. USB OLED değişiminde mevcut önizleme/onay sistemi korunur.

### BLE

1. **Android Chrome + HTTPS** kullanın; telefonda Bluetooth açık olsun.
2. **Bluetooth ile bağlan** seçip `nRF52_Lora_Tracker` cihazına bağlanın.
3. **BLE Ayarları Oku** ile gerçek ayarları alın.
4. **Kaydetme açık** anahtarını AÇIK yapın (**PIN yok**).
5. **BLE Ayarları Kaydet** seçin. Başarılı kayıttan veya bağlantı kesilmesinden sonra anahtar yeniden KAPALI olur.

BLE kaydı, değişiklikleri tek **Direct Flash A/B + CRC32** işlemiyle kaydeder. Bu anahtar güçlü erişim güvenliği sağlamaz; eşleştirmesiz BLE UART bağlantısında yakın çevredeki diğer istemcilere karşı kimlik doğrulama yoktur.

## 3. İlk çalıştırmada yapılacak ayarlar

- **Çağrı işareti:** Kendi yetkili çağrı işaretinizi ve SSID'yi girin; `NOCALL` iken TX engellenir.
- **LoRa çipi:** Gerçek SX1262 veya SX1268 modülünü seçin. Çip değişirse RF yeniden başlatmaya kadar devre dışı kalır.
- **OLED tipi:** SH1106 veya SSD1306. BLE seçimi **20 saniye önizlemesiz doğrudan kaydeder**. Ekran görünmezse açılışta **menü tuşunu 3 sn basılı** tutarak kurtarın. Bu tuş UF2 için kullanılan RESET değildir.
- **APRS sembolleri:** Hareket ve park ayrı iki karakterlidir; örnekler `/>` ve `\P`.
- **Otomatik yayın:** AUTO, SMART/FIXED, WX/Status/Comment aralıklarını kontrol edin. İlk GPS konumu Comment ile aynı pakette, Status daha sonra ayrı pakettedir; ortak RF gönderim aralığı **en az 15 sn**.
- **TX gücü:** 1–22 dBm çip hedefi; gerçek anten çıkışı donanıma bağlıdır. Yasal/hardware sınırlarını aşmayın.
- **BME280:** WX paketi için isteğe bağlıdır; sensör yoksa ilgili veri gönderilemez.

## 4. Direct Flash ayar kaydı

- `p` ve `flash?` komutlarıyla seri porttan (115200 baud) mevcut durumu kontrol edin.
- **ACTIVE** ise ayarlar kayıtlıdır; rutin kullanımda `flash init` çalıştırmayın.
- Geçerli Direct Flash kaydı **yoksa**, yalnızca donanım ve ayar yedeği doğrulandıktan sonra **USB seri bağlantısı** ile `flash init` kullanılabilir. Bu ilk ayar kaydıdır; BLE üzerinden başlatma bilinçli olarak desteklenmez.
- Değişiklikleri kaydettikten sonra cihazı tekrar okuyup gerekirse yeniden başlatıp kalıcılığı doğrulayın.

## 5. Arduino IDE ile derleme

- **Adafruit nRF52 core 1.7.0**, projeye özel **TA1FM MiniMesh Lite NRF (TA2DMX)** kart tanımı ve uyumlu UF2 bootloader.
- **RadioLib 7.8.1**, Adafruit TinyUSB (core ile), Adafruit GFX, Adafruit SH110X, Adafruit SSD1306, TinyGPSPlus, Adafruit BME280, Adafruit Unified Sensor ve core ile gelen InternalFileSystem/flash_nrf5x. **LittleFS başlatılmıyor**; bu başlık düşük seviye flash sürücüsü için derlemeye dahil.
- Yanlış board variant/pin eşlemesi, RF anahtarı veya bootloader seçimi cihazı çalışmaz hâle getirebilir.

## 6. Seri komutlardan örnekler (115200 baud)

| Komut | İşlev |
|---|---|
| `p` / `flash?` | Durum / Direct Flash kontrolü |
| `chip sx1262` / `chip sx1268` | LoRa çipini seç (yeniden başlat) |
| `c TA1FM-7` | Örnek çağrı işareti (kendi işaretinizi kullanın) |
| `a on` / `a off` | Otomatik yayın |
| `f smart` / `f fixed` | Konum modunu seç |
| `symbol />` | Hareket sembolü |
| `parksymbol \P` | Park sembolü |
| `smart 600,90,45,30,20,15` | Hıza bağlı altı Smart Beacon aralığı (saniye) |
| `k 1` / `k 3` / `k 5` | Park onay süresi (dakika) |
| `power 10` | RF hedef gücü (1–22 dBm) |
| `t` / `w` / `v` | Manuel konum / WX / Status isteği |

Detaylar: [Türkçe kullanım kılavuzu](./KULLANIM_TR.md).

## 7. Pin bağlantıları ve güvenlik

[TA1FM Pin Bağlantıları](./PIN_BAGLANTILARI_TR.md) — LoRa SPI, RF anahtarı, GPS UART, OLED, BME280, tuş ve batarya ADC.

Anten bağlanmadan RF yayın yapmayın; 3.3 V GPIO sınırlarına dikkat edin. Yalnızca yetkili amatör telsiz frekans/çağrı işareti ve güç sınırlarıyla kullanın.

**73 de TA1FM**
