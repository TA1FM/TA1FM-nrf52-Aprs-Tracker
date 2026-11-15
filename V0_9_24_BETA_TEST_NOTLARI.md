# TA1FM nrf52 Tracker — V0.9.24 BETA

**Tam kaynak dosyası:** `TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_BETA.ino`

## Temel sürüm ve korunan özellikler

- **Temel:** V0.9.23 **RF TEST 2** — SX1262 ve SX1268 için doğrulamalı **140 mA OCP** ayarı ve güç değiştirilirken OCP'yi yeniden uygulama işlevi aynen korundu.
- GPS, SMART/FIXED Beaconing, LoRa konum/WX/STATUS yayınları, OLED SH1106/SSD1306 seçimi, BLE ve Direct Flash A/B saklama yapısı korunur.
- Flash kayıt şeması **2** olarak kaldı; kayıt yapısı ve CRC algoritması değiştirilmedi. Mevcut ayarların otomatik silinmesi veya `flash init` uygulanması yoktur.
- Mevcut V0.9.23 BLE/USB web paneliyle uyumluluk için **`CFG|BEGIN|V0.9.23` ve `[V23] mode=...` protokol işaretleri KASTEN korunmuştur.** Gerçek sürüm açılış mesajında ve OLED'de V0.9.24 olarak gösterilir.
- V0.9.22 kararlı sürümü korunmuştur. GitHub'daki TX/OCP eksiği bulunan V0.9.23 BETA kaynağı V0.9.24 ile değiştirilerek ana daldan kaldırılmıştır.

## V0.9.24 BETA değişiklikleri

1. **DIGI mükerrer filtresi:** Aynı kaynak çağrı işareti + APRS bilgi içeriği için süre **120 saniyeden 30 saniyeye** indirildi. Farklı içerik ayrı paket olarak kabul edilir. Kuyruktaki aynı paket ikinci kez eklenmez.
2. **DIGI RX/TX:** `[DIGI TX OK]` mesajı artık yalnızca `radio->transmit()` başarılı döndükten sonra yazılır. `[DIGI TX ERROR]` ve kuyruk zaman aşımı sayacı eklendi. Bu kayıt başka bir iGate'in paketi aldığını *garanti etmez*.
3. **DIGI RX toparlanması:** DIGI modunda RX başlatma başarısız olursa ana döngüyü bloke etmeden yaklaşık 3 saniyede bir `startReceive()` denenir. Başarılı/hatalı toparlanmalar sayılır. BLE üzerinden LoRa çipi değiştiğinde zorunlu yeniden başlatma emniyeti korunur; otomatik toparlanma RF'yi izinsiz etkinleştirmez.
4. **BME280 seri mesajları:** Sensör bulunamıyorsa uyarı ilk yoklukta bir kez yazılır, tekrar tekrar yazılmaz. Sonradan bulunması veya okuma hatası gibi durum değişikliklerinde mesajlar korunur.
5. **GPS/pil seri mesajları:** Periyodik durum satırı **15 yerine 60 saniyede bir**. GPS FIX kazanımı/kaybı değişiklik olduğunda ayrıca yazılır. GPS okuma ve otomatik beacon zamanlamaları değişmedi.
6. **Gereksiz iGate notu:** `[TX] RadioLib TX OK; confirm reception on iGate` metni `[TX] RadioLib TX OK` oldu. WX'deki benzer doğrulama tavsiyesi de kısaltıldı.

**Değiştirilmeyenler:** 10 saniyelik RF koruması, 4 paketlik DIGI kuyruğu, 433.775 MHz / SF12 / BW125 / CR4/5, WIDE yönlendirme, güç ve anten anahtarlama kodu. Yeni CAD algoritması eklenmedi.

## Arduino IDE ve güvenli test

1. Çalışan `.ino`, varsa `.uf2`, ve cihaz ayar JSON yedeğini sakla.
2. Yeni `.ino` dosyasını **tamamını eski kaynak yerine kullanarak** Arduino IDE'de aç. IDE ayrı klasör oluşturmayı önerirse kabul et.
3. Önce **Doğrula/Derle**. Kart/core, SoftDevice ve kütüphaneler mevcut çalışan kaynakla aynı kalmalı. **Bu ortamda tam Arduino derlemesi yapılmadı; kullanıcı V0.9.24'ü gerçek cihazda saha testine çıkardı.**
4. İlk açılışta `V0.9.24` başlığını, doğru **SX1262 veya SX1268** seçimini ve `[RF TEST 2] ... OCP ... readback=140.0 mA rc=0` çıktısını kontrol et. RF güç artışını mevcut güvenli 50 ohm ölçüm düzeninde teyit et; PA giriş sınırlarını aşma.
5. **TA1FM-9 araç TRACKER testi:** `p` → `mode=TRACKER`, GPS FIX ve AUTO SMART ayarlarını kontrol et. TX/RX ve GPS FIX değişimlerini kaydet. Telefon USB-OTG kayıt uygulamasının ekran kapalıyken log almaya devam ettiğini önceden dene.
6. **TA1FM-6 ayrı DIGI testi:** `mode?` ile `[DIGI] Eligible`, `TX`, `TX_ERR`, `DUP`, `QUEUE_FULL`, `EXPIRED`, `RX_RECOVER`, `RX_RECOVER_ERR` sayaçlarını izle. Aynı APRS bilgi içeriğini 30 saniye içinde tekrarlayan paketler mükerrer kabul edilmeli. 30 saniye dolduktan sonra gelen uygun paket yeniden işlenebilir; RF TX 10 saniyelik korumaya tabidir.

**Test durumu (15 Kasım 2026):** Kaynak üzerinde statik kontroller yapıldı. Kullanıcının yaklaşık 30 dakikalık / 10 km araç testine ait seri logunda **TA1FM-6, SX1268, TRACKER, AUTO ON / SMART, 22 dBm** görülüyor; GPS FIX mevcut, LoRa TX başarıları ve TA1HF-10 üzerinden geri duyulan tekrarlar kayıtlı.
**Bu test SX1268 TRACKER modunu doğrular; SX1262 ve sabit DIGI çalışma modu ayrıca sınanmalıdır.

73 de TA1FM
