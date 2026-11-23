# TA1FM nrf52 Tracker v1.3.0 BETA2 — Test Planı

## Kapsam
BETA1 taban alınmıştır. v1.2.0 STABLE ve v1.3.0 BETA1 kaynakları değiştirilmemiştir.
Firmware: `BETA/TA1FM_nrf52_Tracker_v1.3.0_BETA2.ino`
Panel: `https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/BETA/TA1FM_Tracker_Control_Panel_v1.3.0_BETA2.html`

## Testler
1. **Derleme/açılış:** Arduino'da derleyin. Açılışta `UNIVERSAL V1.3.0 BETA2`, LoRa RX, GPS, OLED tuşları ve DFU normal çalışmalı.
2. **BME280 yok + telemetri açık:** Panelden `telem on` gönderin. Konumda `Batt=...` görünmeli fakat `|....|` Base91 telemetri eki bulunmamalı. Yeni -100 °C / %0 / 0 hPa veri örnekleri üretilmemeli. aprs.fi daha önceki hatalı tarihî ölçümleri tutabilir.
3. **BME280 var:** Sensör bağlantısını/doğrulanmış ölçümlerini kontrol edin. `telem on` ile yeni konum paketinde tam Base91 telemetri eki görünmeli. Geçersiz veya eski (>15 sn) BME ölçümleri bu eki engellemeli.
4. **Comment:** Panelde Comment aktif (commentMin=15/30/60 veya 1) ve Auto açıkken art arda birkaç konum paketi üretin. Her başarılı `[TX] REQUEST` satırında aynı Comment metni görülmeli; pil metni varsa ardından gelmeli. Comment OFF ise metin eklenmez. BETA2'de dakika değeri Comment tekrar aralığı olarak uygulanmaz; metin her konuma eklenir.
5. **Status:** İlk konumdan sonra 10 saniyelik RF güvenlik aralığını koruyarak ayrı `[STATUS TX]` paketi bekleyin; ayarlanan süre geldiğinde tekrar gönderim kontrol edin. Olası çakışmada rutin/stale konum Status'u sürekli öne geçirmemeli. `RadioLib OK` yalnız yerel RF TX başarısını gösterir; aprs.fi'ye ulaştığını kanıtlamaz.
6. **Panel mesaj görünümü:** USB ve BLE'de alıcıya panelden mesaj gönderin. `MSG|QUEUED|TO|ID` alındığında Mesajlar alanında **→ alıcı · #ID · Sırada** görünsün. **Gönderim durumu** düğmesiyle `MSG|OUT` sonucu okunabilmeli. **Gelenleri oku** sonrasında gönderilen kayıtlar kaybolmamalı. Panelin gönderilen listesi **yalnız açık panel oturumunda** tutulur, cihaz flash'ına yazılmaz. `ACK_RECEIVED` karşı taraftaki RF cihazın ACK yanıtıdır, insan tarafından okunduğu anlamına gelmez.

## Değişmeyenler
- RF frekansı, LoRa çip seçimi, GNSS/OLED/DFU ve mevcut flash A/B ayar kayıtları korunur.
- STABLE kaynak ve panel değiştirilmemiştir.
- Web panelinin yeni sürümü eski BETA1 cihazı ile de ortak mesaj komutları üzerinden çalışabilir.

## Uyarı
Bu dosyalar kod ve JavaScript sözdizimi açısından kontrol edilmiştir; Arduino toolchain derlemesi ve fiziksel cihaz testleri henüz yapılmamıştır. Denenmiş STABLE sürümü yedekte tutun.
