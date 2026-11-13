# TA1FM nrf52 Tracker — V0.9.22 sürüm notları / Release notes

**Firmware:** V0.9.22 UNIVERSAL BLE SAVE SWITCH  
**Hedef:** TA2DMX MiniMesh Lite NRF / nRF52840 · 433.775 MHz LoRa APRS  
**LoRa:** SX1262 veya SX1268 (seçilebilir; LLCC68 desteklenmez)

Bu dosya, V0.9.22 kaynak kodu ve eşleşen USB/BLE HTML panelindeki özellikleri açıklar. Donanım üzerindeki performans ve RF kapsama değerleri burada garanti edilmez. Önceki sürümler arşivde tutulur.

## Değişiklikler / What's new

| Konu / Area | V0.9.22 |
|---|---|
| **LoRa çipi** | Tek firmware içinde **SX1262 / SX1268 seçimi**; web panelinde BLE üzerinden kalıcı kayıt. Çip değişiminden sonra **yeniden başlatma gerekir**, o zamana kadar RF durdurulur. |
| **Bluetooth BLE** | BLE adı **nRF52_Lora_Tracker**; Nordic UART Service (NUS). Canlı GPS, uydu, çağrı işareti, çip, RF ve batarya bilgileri. |
| **Kaydetme izni** | Önceki PIN/üç dakikalık kilit yerine **Kaydetme AÇIK/KAPALI** anahtarı. Kayıt veya bağlantı kesilince **KAPALI**. Bu anahtar erişim güvenliği sağlamaz. |
| **BLE kayıt** | Değişiklikler RAM taslağında hazırlanır; **tek Direct Flash A/B + CRC32** kaydıyla saklanır. Flash başlatma USB'ye özeldir. |
| **Hareket sembolü** | İki karakterli APRS simgesi örn. `/>`; web paneli ve `symbol />` seri komutu. |
| **Park sembolü** | Hareketten bağımsız iki karakterli APRS simgesi örn. `\P`; `parksymbol \P` seri komutu. |
| **OLED sürücüsü** | SH1106/SSD1306 BLE ile **doğrudan** kaydedilir ve uygulanır; 20 sn deneme istenmez. USB önizleme/onay sistemi korunur. Açılışta **3 sn menü tuşuyla** ekran kurtarma korunur. |
| **Sürüm gösterimi** | OLED Device Info'daki **V0.9.22** etiketinin ekrana sığması için konumu düzeltildi. |
| **İlk yayın** | AUTO, GPS FIX ve ilgili aralıklar etkinse ilk konum paketine Comment eklenir; Status ayrı pakette daha sonra gönderilir. |
| **TX koruma** | Konum/WX/Status için LoRa RF yayınları arasında **ortak minimum 15 saniye**. |
| **Eski özellikler** | GPS, Smart/Fixed Beacon, park algılama, WX (BME280), Status/Comment, pil voltajı, LoRa RX, OLED, USB seri komutları ve güç ayarı korunur. |

## Kısa kullanım / Quick start

1. [V0.9.22 UF2](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_22.uf2) indirin; UF2 bootloader ile yükleyin. [Kaynak kod](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_22.ino).
2. [Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/) açın.
3. **USB:** Masaüstü Chrome/Edge → Cihaza bağlan → Cihazdan oku → Ayarları cihaza yaz.
4. **BLE:** Android Chrome + HTTPS → Bluetooth ile bağlan → **nRF52_Lora_Tracker** → BLE Ayarları Oku → **Kaydetme AÇIK** → BLE Ayarları Kaydet.
5. LoRa çipi değiştiyse **yeniden başlatın**. OLED karardıysa açılışta **3 saniye menü tuşunu basılı** tutun.
6. Güncel GPS FIX, geçerli çağrı işareti, anten ve doğru RF ayarlarını doğrulayın.

## Önemli notlar / Safety and compatibility

- Donanımdaki **gerçek** LoRa çipini seçin. Yanlış çip seçimi RX/TX'i durdurabilir. Anten bağlamadan yayın yapmayın.
- OLED'de yanlış sürücü ekranı karartabilir, ancak USB/BLE veya 3 sn açılış kurtarma seçeneği vardır.
- BLE yazma izni açma anahtarı **şifrelenmiş erişim kontrolü değildir**; yakın BLE cihazlarıyla paylaşılan ayarları korumaz.
- V0.9.22 **Direct Flash A/B** kayıt sistemini kullanır. Önceden geçerli bir kayıt bulunmazsa `flash?` sorgulayın; `flash init` yalnızca USB üzerinden, gerekli yedeklerden sonra ileri seviye işlemdir.
- **Comment** konum paketine eklenen metindir; bağımsız RF paket değildir. **APRS mesajlaşması mevcut değildir.**
- Çağrı işareti `NOCALL` iken TX engellenir; bölgesel amatör telsiz yetki, frekans ve güç kurallarına uyun.

## Belgeler / Documentation

- [README](./README.md)
- [Türkçe Kullanım](./KULLANIM_TR.md) · [Türkçe Kurulum](./KURULUM_TR.md)
- [English User Guide](./USER_GUIDE_EN.md) · [English Installation](./INSTALL_EN.md)
- [Pin bağlantıları](./PIN_BAGLANTILARI_TR.md)

**73 de TA1FM**
