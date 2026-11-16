# TA1FM Tracker Görselleri

Bu klasörde cihaz fotoğrafları ve arşiv görselleri bulunur.

## ⚠️ Geçersiz bağlantı görseli — kullanmayın

**`TA1FM nRF52 Tracker Bağlantı Şeması.png`** dosyası **YANLIŞ BAĞLANTILAR İÇERİR ve KABLOLAMA REFERANSI OLARAK KULLANILMAMALIDIR.**

- OLED/BME280 **SCL** etiketi görselde yanlışlıkla **P1.06** gösterilmiştir. TA1FM V0.9.24 için geçerli kart varyantı **P0.11** kullanır.
- Bazı renkli çizgiler doğru fiziksel uçlara ulaşmaz. Özellikle güç hatları ve P0.17 RF anahtarı kontrolü yanlış yorumlanabilir.
- Görsel **[README](../README.md)** ve **[pin bağlantıları rehberinden](../PIN_BAGLANTILARI_TR.md)** kaldırılmıştır; yalnızca hatalı görselin arşiv kaydı olarak kalmaktadır.

**Yeni, elektriksel olarak test edilmemiş bir görseli bile lehimleme şeması saymayın.** [V0.9.24 kaynak kodu](../TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.ino) ve [güncel yazılı pin tablosunu](../PIN_BAGLANTILARI_TR.md) referans alın; Pro Micro kart revizyonu, RF anahtar/TCXO ve besleme devresi ayrıca doğrulanmalıdır.
