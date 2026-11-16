# TA1FM nRF52 APRS — V0.9.25 BETA-2 OLED Gelen Kutusu

**Deneysel kaynak kod. Henüz Arduino IDE derlemesi veya cihaz testi yapılmadı.** BETA-1'in kullanıcı tarafından başarıyla test edilen mesaj RX, USB `testmsg`, `msg list`, `msg clear` ve mükerrer filtre özellikleri korunur.

- **[Tam BETA-2 .ino dosyası](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA2_OLED.ino)** (STABLE ve BETA-1 ayrı dosyalarda korunur).
- OLED menüsüne **MESSAGES** seçeneği eklenmiştir; OLED'de 10 mesajlık RAM kutusu, gönderen çağrı işareti ve 67 ASCII karakterlik metnin tamamı gösterilir.
- Kısa basış: sonraki mesaj. Uzun basış: ana menüye geri dön. Ana ekranda kısa basışın **konum beacon TX işlevi değişmedi**.
- Gelen yeni mesaj için OLED ana ekranında 5 saniyelik bildirim ve üst satırda **M1...M9/M+** okunmamış mesaj işareti var. OLED **ALWAYS OFF** tercihi korunur.
- Simüle edilmiş mesajın gönderen çağrı işaretinin sonunda `*` bulunur. Mesaj menüsüne girince okunmamış sayacı sıfırlanır.
- **Mesaj TX/ACK yoktur.** Normal konum, hava durumu, durum beacon yayınları önceki gibi çalışır. Gerekirse `a off` ile tezgâh testinde otomatik beacon kapatılır.
- Direct Flash A/B kayıt şeması, RF ve BLE Web Configurator `CFG|BEGIN|V0.9.23` uyumluluk işareti değişmedi.

## Nasıl test edeceğiz?

1. Çalışan **V0.9.24 STABLE UF2** ve cihaz ayarlarını yedekle.
2. Bu dosyayı Arduino IDE'de, önceki çalışan MiniMesh kart profili / Adafruit nRF52 core / SoftDevice / kütüphanelerle **Derle/Doğrula**. Başarılı değilse yükleme.
3. Derleme başarılıysa BETA-2, **karta kurulu önceki firmware'in tamamının yerine geçer**; GitHub'daki STABLE ve BETA-1 dosyaları değişmez.
4. USB Seri Monitör: **115200 baud**. Kayıtlı callsign örneğin `TA1FM-9`, çalışma modu TRACKER.
5. Seri monitöre `testmsg TA1FM-8 SELAM KANKA` yaz. **Bu komut RF mesaj TX yapmaz**. OLED'de `NEW APRS MESSAGE`, gönderen ve üst satırda `M1` gör.
6. Ana ekranda **uzun bas** → menüde **MESSAGES** satırını bulana kadar kısa bas → uzun bas. Gelen mesajı oku.
7. İkinci simüle mesaj için `testmsg TA1FM-8 IKINCI MESAJ` gönder; OLED gelen kutusunda kısa basarak iki mesaj arasında dolaş.
8. `msg list` ile seri konsoldaki kayıtları karşılaştır. `msg clear` yazınca OLED'de `INBOX EMPTY` görülmeli.
9. Tek tuş kısa/uzun davranışları, GPS/beacon/LoRa ve BLE'nin eski çalışma biçimini de kontrol et.

**Sınırlamalar:** Gelen kutusu RAM'dir; güç kesilince kaybolur. Her 21 karakterde keskin satır geçişi yapılır (kelime kaydırma yok). 30 saniye kullanıcı işlemi olmazsa menü otomatik kapanır. `testmsg` gerçek RF yayını veya gerçek LoRa RX saha testinin yerine geçmez.

**73 de TA1FM**
