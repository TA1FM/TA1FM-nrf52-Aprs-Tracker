# TA1FM nRF52 Tracker — V0.9.25 BETA-3 OLED Mesaj Menüsü

**Deneysel sürüm. Arduino IDE üzerinde derleme ve gerçek cihaz testi henüz yapılmadı.** [BETA-2'nin](../TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA2_OLED/TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA2_OLED.ino) tam kaynak kodundan türetildi. Eski sürümler ve V0.9.24 STABLE GitHub'da aynen bırakıldı.

## Yenilikler

1. **MESSAGES ana menünün 1. sırasında.** BME280 takılı olsa da olmasa da birinci sırada; kalan menü işlevleri sırayla aşağı kaydırıldı.
2. **Okunmamış mesaj bildirimi kalıcı.** Mesaj kutusu açılıncaya veya temizleninceye dek ana ekranda NEW APRS MESSAGE / FROM / TAP: READ NOW / HOLD: MENU görünür. OLED'in belirlenmiş uyku süresi yine geçerlidir; ekran uyandığında bildirim geri gelir. "ALWAYS OFF" tercihi korunur.
3. **Tek kısa basışla doğrudan mesaj kutusuna girilir**, cihazın OLED'i uykudaysa bile. Gelen kutusunda en yeni mesaj ilk görüntülenir. Okunmamış sayaç sıfırlanır.
4. Yeni mesaj **yoksa**, ana ekrandaki eski kısa basış **konum beacon TX** işlevi aynen çalışır.
5. Gelen kutusunda **kısa basış** sıradaki mesaja ilerler. En eski mesajdan sonra bir öğe daha vardır: **DELETE ALL**. Sonrasında tekrar en yeni mesaja döner.
6. **Silme güvenliği:** DELETE ALL öğesindeyken **uzun bas** onay ekranına girer. Onay ekranında **kısa bas = CANCEL** (hiçbir şey silinmez); **ikinci uzun bas = tüm RAM mesajlarını sil**. Yalnızca mesaj kutusunu temizler, Direct Flash A/B cihaz ayarlarına dokunmaz.
7. Mesaj görüntülerken uzun bas ana menüye döner. Mesaj yoksa MESSAGES ekranı **INBOX EMPTY** der.
8. GPS, SMART/FIXED beacon, WX, STATUS, Digipeater, OLED seçimi ve BLE ayar protokolü değişmedi. **Mesaj TX ve ACK hâlâ kapalı.**

## Test

1. Önce V0.9.24 STABLE UF2 ve cihaz JSON ayarlarının yedeğini sakla.
2. [BETA-3 tam .ino](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA3_MSG_MENU.ino) dosyasını Arduino IDE'de, aynı isimli sketch klasöründe, BETA-2 ile çalışan aynı kart/SoftDevice/kütüphane ayarlarıyla **Derle/Doğrula**. Hata varsa karta yükleme.
3. Karta kurarsan BETA-3 eski firmware'in **tam yerine geçer**, ancak GitHub'daki STABLE/BETA-1/BETA-2 dosyalarına dokunulmaz.
4. USB 115200 baud, station TRACKER ve kayıtlı callsign (ör. TA1FM-9). Tezgâh testinde normal otomatik beacon istemezsen `a off`.
5. `testmsg TA1FM-8 SELAM KANKA 1` — gelen mesaj bildirimini gör. En az 5 saniye bekle: **bildirim kaybolmamalı**. Ekran otomatik uyursa sonra tek kısa basış mesajı açmalı.
6. Kısa bas: **en yeni mesaj kutusu** açılmalı. Başlangıç mesajı doğru mu? Mesaj yoksa konum TX'nin eski kısa basış davranışını kontrollü test et.
7. `testmsg TA1FM-8 SELAM KANKA 2` yaz. Ana menüden **uzun bas → ilk menü MESSAGES → uzun bas** yolunu da test et.
8. Mesaj kutusundayken kısa basışlarla iki mesajı geç; en sonda **DELETE ALL** görüntülenecek.
9. DELETE ALL'de uzun bas → onay ekranı; kısa bas → **CANCEL**. `msg list` ile mesajların silinmediğini doğrula.
10. DELETE ALL'e tekrar git, uzun bas ile onay aç, ikinci uzun bas ile sil. `msg list` = **INBOX 0/10**, OLED **INBOX EMPTY**, okunmamış bildirim yok.
11. Standart `msg clear` USB komutu da aynı RAM temizleme işini sürdürür.

**NOT:** `testmsg` RF mesaj TX yapmaz. Eski AUTO beacon özelliği aktifse normal konum/WX/status RF yayınları olabilir. Mesajlar cihaz yeniden başladığında RAM'den silinir. OLED uyku politikasını değiştirmedik; kalıcı olan **mesaj bildiriminin durumu**, ekranın sürekli açık kalması değildir.

73 de TA1FM
