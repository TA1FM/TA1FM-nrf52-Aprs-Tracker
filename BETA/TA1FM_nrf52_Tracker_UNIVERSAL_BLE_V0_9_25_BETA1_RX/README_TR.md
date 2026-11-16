# TA1FM nRF52 Tracker + Digipeater — V0.9.25 BETA-1 APRS Mesaj RX

**Deneysel sürüm. Arduino IDE derlemesi ve gerçek donanım testi henüz yapılmadı.** Kaynak: [V0.9.24 STABLE](../../TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.ino). STABLE yazılım/UF2 dosyalarına dokunulmadı.

## Eklenenler

- LoRa APRS `3C FF 01` paketinden kendi çağrı işaretine hitap eden standart mesajları ayrıştırır (yalnızca **TRACKER** modunda).
- 9 karakterlik alıcı, 67 karaktere kadar ASCII metin, opsiyonel `{ID` (1–5 alfanümerik) doğrular.
- Son **10 mesaj RAM'de**, yenisi önce; aynı kaynak+ID+metin için 30 dakika tekrar engeli.
- `ackNN` / `rejNN` kontrol paketlerini kullanıcı mesajı olarak saklamaz.
- USB simülasyonuyla gerçek radyo yayını olmadan alıcıyı test eder.
- Mesaj gönderme (TX) ve otomatik ACK **henüz yoktur**. OLED mesaj ekranı/BLE mesajlaşma da sonraki aşamada.
- Mevcut GPS/OLED/BLE/Tracker/Digipeater ve Direct Flash A/B ayar yapısı korunur; BLE `CFG|BEGIN|V0.9.23` uyumluluk işareti değişmez.

## Arduino IDE / test

1. [Bu .ino dosyasını](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA1_RX.ino) indirip **`TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA1_RX`** adlı sketch klasöründe aç.
2. STABLE'da kullandığın **aynı Arduino kart tanımı, Adafruit nRF52 çekirdeği, SoftDevice ve kütüphanelerle** derle. Herhangi bir hata varsa karta yükleme.
3. Çalışan **V0.9.24 STABLE UF2 ve JSON ayar yedeğini** önceden sakla.
4. BETA karta yüklendiyse USB Seri Monitör aç: **115200 baud**.
5. Geçerli bir istasyon çağrı işareti (örnek **TA1FM-8**) kayıtlı ve cihaz **TRACKER modunda** olmalı. Tezgâh testinde otomatik beacon'ı kapatmak için `a off` kullanılabilir.

| USB komutu | İşlev |
|---|---|
| `msg?` | Komut yardımı ve mesaj alım sayaçları |
| `testmsg TA1FM-9 SELAM KANKA` | Kaynağı TA1FM-9, hedefi cihazın kayıtlı çağrı işareti olan standart APRS mesajını **RF yayını olmadan** aynı parser'a verir |
| `msg list` | Son 10 mesajı listeler |
| `msg clear` | Yalnızca RAM mesajlarını temizler, flash kayıtlarına dokunmaz |

Örnek çıktı (cihazın çağrısı TA1FM-8 ise):

```text
[APRS MSG TEST] Inject local frame WITHOUT RF TX...
[APRS MSG TEST] ACCEPTED - NO RF TX
[APRS MSG] FROM : TA1FM-9
[APRS MSG] TO   : TA1FM-8
[APRS MSG] TEXT : SELAM KANKA
[APRS MSG] ID   : 01
[APRS MSG] AUTO ACK DISABLED IN BETA-1
```

Aynı `testmsg` tekrarlandığında `[APRS MSG DUP]` beklenir; değişik metin yeni kayıt oluşturur. `msg list` içinde simülasyon mesajları **(SIM)** işaretiyle görünür.

**Not:** `testmsg` mesaj TX yapmaz fakat cihazın normal konum/WX/status beacon özellikleri önceki yazılım gibi çalışmaya devam eder. Bu test **gerçek RF alımının doğrulaması değildir**. Mesajlar güç kesilince silinir.

## Sonraki adımlar

1. Gerçek LoRa paketini ikinci cihazdan göndererek RX test.
2. Mesaj TX + ACK + sınırlı retry + TX zamanlayıcısı.
3. OLED gelen kutusu / tek tuşla hazır mesaj.
4. BLE/USB web panelinde mesajlaşma.

**APRS protokol kaynağı:** [APRS 1.0.1 mesaj formatı, bölüm 14](https://www.aprs.org/doc/APRS101.PDF).

73 de TA1FM
