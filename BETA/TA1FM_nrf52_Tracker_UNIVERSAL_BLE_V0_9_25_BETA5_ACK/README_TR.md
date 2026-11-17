# TA1FM nRF52 Tracker — V0.9.25 BETA-5 APRS ACK / RETRY

**Deneysel kaynak kod: Arduino IDE derlemesi ve gerçek cihazda ACK testi henüz yapılmadı.** [BETA-4 tam kodundan](../TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA4_MSG_TX/TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA4_MSG_TX.ino) üretildi. V0.9.24 STABLE ve BETA-1/2/3/4 dosyaları aynen korundu.

## Eklenenler

- USB `msg send`, APRS standardı `{0001`, `{0002` vb. **4 rakamlı mesaj ID** gönderir. Her yeni mesajda RAM tabanlı numara artar; yeniden başlatmada sıfırlanır.
- Mesaj alıcısı hedef callsign'ı ve ID'yi doğrular. **Gerçek RF** üzerinden gelen, bu cihaza adreslenmiş ID'li mesaj için otomatik `:ack0001` yanıtı hazırlar; tekrar gönderilmiş aynı ID'ye de tekrar ACK verebilir.
- RF ACK kaynağı **gönderdiğimiz hedef istasyon** ve ID **beklediğimiz mesaj ID** ise `ACK_RECEIVED`; aynı eşleşmeyle `rej` gelirse `REJECTED`. ACK'nin bir insan tarafından okunduğunu göstermez.
- Cihaz maksimum **3 RF TX denemesi** (ilk yayın + 2 yeniden yayın), her başarılı yayından sonra **60 saniye ACK bekleme**. Sonraki ACK gelmezse `ACK_TIMEOUT`.
- RF mesaj TX ve ACK için **mevcut ortak 10 saniyelik TX beklemesi** korunur. En fazla 4 bekleyen otomatik ACK işi; ACK'ler kullanıcı mesajı ve otomatik beacon'dan önce gönderilir.
- Simüle `testmsg` paketleri **asla gerçek RF ACK oluşturmaz**. Simüle `msg testack` paketleri gerçek teslimat onayı olarak kaydedilmez.
- OLED: gerçek RF ACK gelince ana ekranda 5 saniye **ACK RECEIVED** bildirimi, gönderen ve mesaj ID görüntülenir. Gelen mesaj bildirimleri, birinci sıradaki MESSAGES menüsü ve **DELETE ALL** onayı korunur.
- **BLE üzerinden mesaj gönderimi YOK**; USB RF TX var. Mevcut BLE ayar protokolü ve Direct Flash A/B kayıt düzeni değiştirilmedi.

## Önemli: TA1FM-10 iGate hâlâ mesaj ACK'lemiyor olabilir

Şimdiye kadar TA1FM-10 iGate'in APRS mesajını **RF'den alıp APRS-IS'e aktardığını** doğruladık. Ancak bunun otomatik RF ACK ürettiğini doğrulamadık. Bu yüzden TA1FM-9'dan TA1FM-10'a numaralı mesaj gönderirsen **ACK gelmezse 3 kez tekrar edip TIMEOUT gösterebilir**. Bu, BETA-5'in hatalı olduğu anlamına gelmez; karşı uçta ACK desteği gereklidir. Gerçek ACK testi için karşı uç yazılımına da destek eklemek gerekebilir.

## USB komutları

| Komut | İşlev |
|---|---|
| `msg?` | RX/TX/ACK yardım ve sayaçlar |
| `msg preview TA1FM-10 SELAM KANKA` | `{0001` ID'li paket önizlemesi; **RF yok** |
| `msg send TA1FM-10 SELAM KANKA` | **Gerçek RF TX**; ID ekler ve ACK bekler (en fazla 3 deneme) |
| `msg status` | `QUEUED` / `WAIT_ACK` / `ACK_RECEIVED` / `ACK_TIMEOUT` / `REJECTED`; deneme sayısı ve ID |
| `msg cancel` | Bekleyen mesajı / tekrar denemeyi iptal eder; daha önce yayınlanmış RF'yi geri alamaz |
| `msg outbox` | Aktif veya son giden mesajın ACK durumunu gösterir |
| `msg testack TA1FM-10 0001` | ACK parser'a **RF'siz simülasyon** verir, gerçek teslimat işaretlemez |
| `testmsg TA1FM-8 SELAM KANKA` | Gelen mesajı **RF'siz** simüle eder; otomatik RF ACK **üretmez** |
| `msg list` / `msg clear` | Gelen kutusu listesi / RAM'i temizleme |

## Test planı

1. Çalışan STABLE UF2'yi ve cihaz ayarlarını yedekle.
2. [BETA-5 tam .ino](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA5_ACK.ino) dosyasını, aynı isimli sketch klasöründe, BETA-4'te kullandığın Adafruit nRF52 / SoftDevice / RadioLib sürümleriyle **Derle/Doğrula**. Hata çıkarsa karta yükleme.
3. Derleme başarılıysa BETA-5 cihazdaki firmware'in **tam yerine geçer**; eski GitHub dosyalarına dokunulmaz. Seri monitör 115200 baud.
4. Önce `msg preview TA1FM-10 SELAM KANKA` — yayın yapmadan, sonuna `{0001` eklendiğini gör.
5. USB `testmsg TA1FM-8 SELAM KANKA` — OLED gelen kutusuna mesaj gelir ancak `[APRS ACK TX]` **görünmemeli**.
6. RF yayın yapmaya hazır olduğunda ve uygun hedef cihazda ACK desteği varsa `msg send TA1FM-10 SELAM KANKA`. Önce `[APRS MSG TX] TX OK (LOCAL)... TRY=1/3`, sonra `msg status` ile `WAIT_ACK` beklenir.
7. Alıcı aynı ID için RF `ack0001` verirse `ACK_RECEIVED`. Vermezse yaklaşık 60 saniye aralıklarla 2 ek deneme ve sonrasında `ACK_TIMEOUT`.
8. `msg testack TA1FM-10 0001` — ACK parser'ın test yanıtını gör, ancak **ACK_RECEIVED durumuna geçmemeli**.
9. `msg cancel` — yeniden yayın denemesini durdur.
10. Karşı cihazdan gerçek RF mesaj yolla; `[APRS ACK] QUEUED`, ardından `[APRS ACK TX] LOCAL OK` ve alıcıda ACK görüldüğünü kontrol et.

## Sınırlar / güvenlik

- ACK yalnızca karşı cihazın paketi RF'de aldığını işaretler; insanın okuduğunu göstermez.
- 10 saniyelik ortak TX koruması uygulanır ama gerçek radyo çakışmaları ve RF iş yüküne karşı sahada denemeler gereklidir.
- Mesaj ID sayacı ve giden durum RAM'de, yeniden başlatılınca silinir. 4 haneli sayaç 9999'dan sonra döner.
- Mesaj metni **62 basılabilir ASCII karakterle sınırlı**, çünkü `{0001` 5 karakter kullanır ve APRS mesaj bilgi alanı 67 karakteri aşmamalıdır.
- `msg send` gerçek RF yayını yapar. 433.775 MHz anten/güç/amatör radyo şartlarını gözet.
- **Hazır mesaj özelliği bu sürümde YOK**, isteğe göre en sonda yapılacak.

73 de TA1FM
