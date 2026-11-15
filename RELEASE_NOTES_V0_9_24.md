# TA1FM nrf52 Tracker V0.9.24 STABLE — Universal BLE

**Kararlı sürüm / Stable firmware — 15 Kasım 2026**

nRF52840 tabanlı MiniMesh Lite NRF için SX1262 ve SX1268 LoRa APRS Tracker + DIGI yazılımı.

## Yenilikler / What's new

- **RF TX düzeltmesi:** SX1262 ve SX1268 için 140 mA OCP ayarı ve TX gücü değişiminden sonra yeniden uygulanması.
- **DIGI:** Mükerrer paket filtresi 120 saniyeden 30 saniyeye indirildi; farklı içerikli paketler ayrı değerlendirilir. Dört paketlik kuyruk ve 10 saniyelik RF TX koruması korunur.
- **DIGI güvenilirliği:** Başarılı/hatalı TX kayıtları ayrıldı; RX yeniden başlatılamazsa kontrollü toparlanma ve ilgili sayaçlar eklendi.
- **Daha temiz seri monitör:** BME280 uyarılarının gereksiz tekrarları azaltıldı; GPS/pil periyodik durum satırları yaklaşık 60 saniyeye alındı; gereksiz iGate doğrulama açıklaması kaldırıldı.
- **Korunan özellikler:** SMART/FIXED Beaconing, GPS, hareket/park algılama, LoRa APRS WX/STATUS, SH1106/SSD1306 OLED seçimi, BLE+USB ayar yönetimi, Direct Flash A/B + CRC32, 433.775 MHz (SF12/BW125/CR4/5).

## Saha testi / Field test

15 Kasım 2026'da kullanıcı tarafından yaklaşık **10 km / 30 dakikalık SX1268 TRACKER saha testi** gerçekleştirildi. Seri kayıtlarda GPS FIX ve SMART konum yayınları, başarılı RadioLib TX sonuçları ve başka bir DIGI tarafından tekrar edilen paketlerin geri alınması görüldü. USB seri bağlantısı test sırasında bilinçli olarak kesilip yeniden bağlandığı için loglarda boşluklar bulunmaktadır. Kullanan amatörlerden olumlu geri bildirim alındı.

**Doğrulama sınırı:** Bu saha testi **SX1268 TRACKER moduna** aittir. SX1262 RF ve yeni DIGI filtre/toparlanma mantığının uzun süreli saha testi ayrıca yapılmalıdır. RadioLib TX OK başka bir iGate'in paketi aldığı garantisi değildir.

## Dosyalar / Downloads

- [**V0.9.24 STABLE UF2 — cihaza doğrudan yükleme**](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/raw/refs/heads/main/TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.uf2)
- [**V0.9.24 STABLE .ino — Arduino kaynak kodu**](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.ino)
- [**Uyumlu USB + BLE Web Ayar Paneli**](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html)
- [**Ayrıntılı sürüm notları**](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/V0_9_24_STABLE_NOTLARI.md)

**Önemli:** STABLE .ino ve .uf2 içerikleri, sahada denenmiş V0.9.24 BETA dosyalarıyla **birebir aynıdır**. Yeniden derlenmedi; kaynakta bazı BETA başlıkları ve web paneli uyumluluğu için V23 protokol işaretleri kalabilir. V0.9.22 kararlı sürümü arşivde korunmuştur.

**Kurulum:** Çalışan UF2 ve ayarlarınızı yedekleyin. Cihazın nRF52840 / MiniMesh Lite NRF uyumunu ve doğru SX1262/SX1268 seçimini doğrulayın. Anten bağlı değilken RF gönderimi yapmayın.

73 de TA1FM
