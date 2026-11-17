# TA1FM nrf52 Tracker — V0.9.26 BETA-7 TEST

**DİKKAT:** Önceki BETA-6 ve STABLE dosyaları değiştirilmedi. Bu ayrı deneysel tam firmware'dir. Donanım derlemesi ve RF testi henüz yapılmadı. Önce tek cihazda deneyin.

## BETA-7 yenilikleri

- Mesaj gönderirken **1 (varsayılan), 2 veya 3 toplam RF denemesi**. ACK gelince tekrar gönderimi durur. Tek gönderimde bile ACK 60 saniye beklenir, gelmezse ACK_TIMEOUT yazabilir; ikinci bir RF yayını değildir.
- **USB ve BLE için aynı mesaj protokolü**: HELLO, OUT, INBOX, QUEUED, CANCELLED.
- **Canlı BLE ve USB durum bilgileri**: LIVE|B7|MEM=...|GPS=...|SAT=...|RF=...|TX=...|WX=...|RX=... . Buradaki MEM boş RAM baytı değil, **gelen kutusu mesaj kapasitesi** (0..10).
- OLED, otomatik beacon, LoRa RF, ACK, BLE kaydetme, flash ayarları BETA-6 temelinden korunur; kalıcı flash şeması değişmedi.
- Hazır mesajlar ve iGate APRS-IS→LoRa yanıt aktarımı bu sürümde eklenmedi.

## USB seri monitör komutları — 115200 baud

    msg hello
    msg live
    msg inbox
    msg status
    msg send TA1FM-7 SELAM KANKA
    msg send2 TA1FM-7 SELAM KANKA
    msg send3 TA1FM-7 SELAM KANKA
    msg cancel

msg send bir kez yayın yapar. msg send2 ve msg send3, gerçek eşleşen RF ACK gelmezse sırasıyla en fazla 2 ve 3 toplam yayın yapar. ACK bir insanın okuma onayı değildir.

## BETA-7 panel

https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/BETA/TA1FM_Tracker_Panel_BETA7_Sekmeli_USB_BLE.html

Üst araç çubuğundan BLE ya da USB bağlanın. Mesajlar sekmesinden kullanılacak bağlantıyı seçin. İki cihaz farklı bağlantılardan bağlıysa doğru bağlantıyı seçtiğinize emin olun.

- Mesajlar, Canlı, Ayarlar ve Kayıtlar sekmeleri.
- Mevcut USB ayarları ve BLE ayar kaydetme korunur. Manuel Konum/WX/Status TX düğmeleri USB'ye özeldir.
- USB web seri için Arduino IDE Serial Monitor'ü kapatın. BLE için HTTPS ve destekli tarayıcı gerekir.

## Test sırası

1. BETA-6 yedeğini koruyun. BETA-7 .ino dosyasını tek nRF52 cihaza yeni tam firmware olarak yükleyin (cihazdaki önceki yazılımın yerini tamamen alır).
2. BLE üzerinden MSG|HELLO|B7 ve LIVE|B7 çıktılarının geldiğini kontrol edin.
3. Mesajlar sekmesinde 1 kez seçip diğer çağrı işaretine mesaj gönderin. Alan cihaz gelen kutusunda görüntülesin; ACK durumuna bakın.
4. Aynı cihazı USB'den bağlayıp mesaj kutusunu ve durumu sorgulayın.
5. Hata olursa çalışan BETA-6'ya dönüp konsol logunu paylaşın.

BETA-7 için Arduino IDE derleme veya fiziksel cihaz testi henüz yapılmadı.
