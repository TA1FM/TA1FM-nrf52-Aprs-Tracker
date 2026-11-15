# TA1FM nrf52 Tracker + Digipeater V0.9.24 STABLE — Universal BLE

**Kararlı sürüm / Stable firmware — 15 Kasım 2026**

**Bu sürümde APRS LoRa DIGIPEATER (DIGI) özelliği eklendi!** nRF52840 MiniMesh Lite NRF üzerinde **TRACKER** veya **sabit DIGI** modu seçilebilir (eşzamanlı çalışmaz). SX1262/SX1268 için tek yazılım.

## 📡 Yeni özellik: DIGIPEATER (DIGI)

- Uygun **WIDE1-1** paketlerini, seçili yola göre **WIDE2-1 / WIDE2-2** paketlerini de LoRa RF üzerinden tekrarlar.
- Tekrarlanan paketlere kendi çağrı işaretini yıldızlı ekler (örnek: `TA1FM-6*`); `WIDE2-2` için kalan yol `WIDE2-1` olur.
- **30 saniyelik mükerrer paket filtresi**, **4 paketlik kuyruk**, **10 saniyelik RF TX koruması** ve RX toparlanma sayaçları içerir.
- Kayıtlı enlem/boylamdan **10 / 15 / 30 / 60 dakikada bir sabit beacon** gönderebilir.
- Mod ve ayarlar uyumlu **USB / BLE Web Ayar Paneli** veya USB seri komutlarıyla değiştirilebilir. DIGI için geçerli çağrı işareti ve sabit koordinatlar gerekir.

**[DIGIPEATER kurulumu ve komutlar (TR)](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/DIGIPEATER_V0_9_24_TR.md)**

> **DIGI, APRS-IS iGate değildir.** Paketleri RF üzerinden tekrarlar; aprs.fi için ayrıca bir iGate gerekir. Yeni DIGI iyileştirmeleri için ayrı uzun süreli saha testi henüz tamamlanmadı.

## Yenilikler / What's new

- **RF TX düzeltmesi:** SX1262 ve SX1268 için 140 mA OCP ayarı ve TX gücü değişiminden sonra yeniden uygulanması.
- **DIGI modu ve filtre:** Bu sürümde sabit istasyon Digipeater çalışma modu da bulunur. Mükerrer filtre 120 saniyeden 30 saniyeye indirildi; farklı içerikli paketler ayrı değerlendirilir.
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
- [**DIGIPEATER özellikleri ve ayarlar**](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/DIGIPEATER_V0_9_24_TR.md)
- [**Ayrıntılı sürüm notları**](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/V0_9_24_STABLE_NOTLARI.md)

**Önemli:** STABLE .ino ve .uf2 içerikleri, sahada denenmiş V0.9.24 BETA dosyalarıyla **birebir aynıdır**. Yeniden derlenmedi; kaynakta bazı BETA başlıkları ve web paneli uyumluluğu için V23 protokol işaretleri kalabilir. V0.9.22 kararlı sürümü arşivde korunmuştur.

**Kurulum:** Çalışan UF2 ve ayarlarınızı yedekleyin. Cihazın nRF52840 / MiniMesh Lite NRF uyumunu ve doğru SX1262/SX1268 seçimini doğrulayın. Anten bağlı değilken RF gönderimi yapmayın.

73 de TA1FM
