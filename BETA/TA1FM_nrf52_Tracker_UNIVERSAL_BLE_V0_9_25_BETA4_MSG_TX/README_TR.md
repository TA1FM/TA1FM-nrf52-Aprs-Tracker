# TA1FM nRF52 Tracker — V0.9.25 BETA-4 USB APRS MESAJ TX

**DENEYSEL — Arduino IDE derlemesi ve gerçek RF testleri henüz yapılmadı.** BETA-3'ten ayrı tam yazılım kaynağıdır. **V0.9.24 STABLE ve bütün eski BETA dosyaları korunmuştur.**

## Yeni özellik: tek seferlik adresli APRS mesajı

BETA-3'ün **kalıcı gelen mesaj bildirimi**, ana menü **MESSAGES = 1**, tek kısa basışla okumaya girme, mesaj listesinin sonundaki **DELETE ALL → ikinci uzun basışla onay** özellikleri devam ediyor.

Bu BETA-4, USB Seri Monitör üzerinden gerçek **433.775 MHz LoRa APRS RF mesaj TX** ekler. İlk sürüm güvenli ve basit tutulmuştur:

- APRS formu: `CALLSIGN>APZTFM,WIDE1-1::DEST     :METIN`; **mesaj ID'sizdir** (standarda uygun onaysız mesaj), bu yüzden **ACK ve otomatik tekrar gönderim YOK**.
- Tek mesajlık RAM bekleme kuyruğu, 30 saniye zaman aşımı, iptal komutu, 10 saniyelik ortak RF yayın koruması.
- Mesaj sıradaysa otomatik beacon öncelik vermez, ancak halihazırda yapılmış yayın korunur.
- Mesaj için **GPS FIX gerekmez**, ancak **geçerli yerel çağrı işareti, TRACKER modu ve hazır LoRa modülü** gerekir.
- Hedef farklı ve geçerli bir CALL-SSID olmalı; metin 1–67 yazdırılabilir ASCII karakter olabilir (`|`, `~`, `{` hariç).
- Bu BETA yalnızca **USB üzerinden manuel TX** yapar; BLE Web Paneli ve tek tuş hazır mesaj TX ileride eklenecektir.
- `[APRS MSG TX] TX OK (LOCAL)` **teslimat onayı değildir**. Karşı istasyonun aldığını ancak başka bir cihazın RF kaydıyla doğrulayabiliriz.

## Arduino IDE

1. [Tam BETA-4 .ino dosyası](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA4_MSG_TX.ino) indir, `TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA4_MSG_TX` adlı klasörde aç.
2. Önce mevcut STABLE UF2 / çalışan firmware / cihaz JSON ayarlarının yedeğini sakla.
3. Önceki BETA ile **aynı Adafruit nRF52 kart profili, SoftDevice ve RadioLib/kütüphane sürümleri** kullan. Önce **Derle / Doğrula**.
4. Derleme başarılıysa yeni kod cihaza yüklenebilir: **BETA-4 yüklenince kurulu önceki firmware'in tümünün yerini alır**. GitHub'daki STABLE/BETA-3 kaynaklarına dokunulmaz.
5. USB Seri Monitör, **115200 baud**, satır sonu Newline.

## Mesaj test komutları

| Komut | Ne yapar |
|---|---|
| `msg preview TA1FM-8 SELAM KANKA` | Tam APRS RF mesajını **yayın yapmadan** gösterir |
| `msg send TA1FM-8 SELAM KANKA` | Gerçek RF ile **bir kez** yayın için kuyruğa alır |
| `msg status` | QUEUED / TX_OK_LOCAL_ONLY / TX_RADIO_ERROR / QUEUE_TIMEOUT / CANCELLED durumu |
| `msg cancel` | Henüz yayınlanmamış mesajı kuyruktan siler |
| `testmsg TA1FM-8 SELAM KANKA` | BETA-3'ün **RF yayınsız** gelen mesaj testini yapar |
| `msg list` | Gelen mesajları listeler |
| `msg clear` | Yalnızca gelen kutusu RAM mesajlarını temizler |

**Dikkat:** `msg send` gerçek amatör telsiz RF yayını başlatır; test için uygun anten, doğru güç ve geçerli hedef istasyon kullan. İlk denemede **`msg preview`** ile paket formatını doğrula, sonra gerekliyse `msg send`. **Kendi callsign'ına gönderim reddedilir** çünkü yarı çift yönlü cihaz kendi TX'ini RX olarak alamaz.

## Örnek paket

Gönderici `TA1FM-9`, hedef `TA1FM-8` ise:

```text
TA1FM-9>APZTFM,WIDE1-1::TA1FM-8  :SELAM KANKA
```

Buradaki çift `::`, ilk `:` header sonu, ikinci `:` APRS message veri türüdür. `TA1FM-8` yedi karakter olduğu için 9 karakterlik alıcı alanı iki boşlukla tamamlanır.

**Sınırlar:** Mesaj ID/ACK ve retry (BETA-5), BLE mesaj yazma (sonraki adım), tek tuş hazır mesaj TX (sonraki adım) bu sürümde YOK. `msg send` komutundan sonra TX, mevcut RF korumasına bağlı olarak 10 saniyeye kadar ertelenebilir; RF hazır olmazsa 30 saniyede zaman aşımına uğrar.

73 de TA1FM
