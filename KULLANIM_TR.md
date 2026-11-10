# TA1FM nRF52 LoRa APRS Tracker — Kullanım Kılavuzu (Türkçe)

**Firmware:** V0.9.2 STABLE · **Donanım:** TA2DMX MiniMesh Lite NRF (nRF52840 / SX1262)  
**Frekans:** 433.775 MHz · **Web ayar paneli:** https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/

> Bu belge **V0.9.2 kaynak koduna göre** hazırlanmıştır. Başka firmware sürümlerinde menüler değişebilir. `TA1FM-7` yalnızca örnek çağrı işaretidir; kendi geçerli amatör telsiz çağrı işaretinizi kullanın.

## 1. İlk kullanım

1. V0.9.2 firmware'i kartın uyumlu UF2 bootloader'ı üzerinden yükleyin. Normal güncellemede Eraser gerekli değildir.
2. USB kablosuyla bilgisayara bağlayın; **Chrome veya Edge** ile [TA1FM Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/) sayfasını açın.
3. **USB Bağlan** ile COM portunu seçin; **Cihazdan Oku** ile mevcut ayarları görüntüleyin.
4. Çağrı işaretinizi, AUTO Beacon, sembol, gönderim aralıkları, Comment ve Status alanlarını düzenleyip **Cihaza Yaz** işlemini yapın.
5. İşlem sonrasında cihazın durumunu yeniden okuyup ayarları kontrol edin. İsterseniz ayarları **JSON** olarak yedekleyin.
6. Anten bağlıyken, GPS'in açık gökyüzünü görebildiği bir yerde `GPS:FIX` durumunu bekleyin.

**Önemli:** Geçerli çağrı işareti yokken cihaz `NOCALL` gösterir ve APRS yayını engellenir. İlk kez geçerli çağrı işareti kaydedildiğinde AUTO otomatik açılır; daha sonra bilinçli olarak kapatılan AUTO ayarı yeniden başlatmada korunur. AUTO kapalıysa panelden açın.

## 2. Tek fiziksel tuş nasıl kullanılır?

| İşlem | Ana ekran | Menü ekranı | Ayar ekranı |
|---|---|---|---|
| **Kısa bas** | Manuel konum APRS Beacon gönderme isteği | Sonraki menü maddesi | Değeri değiştir / sonraki seçenek |
| **Uzun bas** (yaklaşık **1,2 sn**) | Ana menüyü aç | Seçili maddeyi aç | Kaydet/onayla veya bilgi sayfasından geri dön |

**Menü örneği:** Ana ekrandayken 1,2 saniye basılı tut → `SET` menüsü açılır → kısa basarak `DISPLAY` üzerine gel → uzun basarak gir.

- Menülerde **30 saniye işlem yapılmazsa ana ekrana dönülür** (OLED sürücü testi ayrı zamanlayıcı kullanır).
- Ana ekrandaki kısa basış **manuel konum TX** isteğidir; yayının gerçekleşmesi geçerli çağrı işareti, GPS ve telsiz hazır olma koşullarına bağlıdır.
- Tek tuş **RESET düğmesi değildir**; bootloader'a geçmek için kartın RESET düğmesi kullanılır.

## 3. OLED ana ekranı

Aşağıdaki örnek değerler açıklama amaçlıdır:

```text
TA1FM-7        18:42:15
---------------------
GPS:FIX       SAT:10
SPD:  0.0 km/h
LAT: 40.97500
LON: 27.50400
BAT:3.92V  RX:TA1FM-9
```

| Gösterge | Anlamı |
|---|---|
| `TA1FM-7` | Cihazın çağrı işareti |
| Saat | GPS saatinden hesaplanır, menüde UTC veya UTC+3 seçilebilir |
| `GPS:FIX` | Geçerli, güncel GPS konumu mevcut |
| `GPS:SEARCH` | GPS konumu henüz hazır değil |
| `SAT` | GPS uydu sayısı |
| `SPD` | GPS'in hesapladığı hız, km/sa |
| `LAT` / `LON` | Enlem / boylam (ondalıklı derece) |
| `BAT` | Yaklaşık batarya gerilimi |
| `RX` | LoRa üzerinden son duyulan APRS istasyonunun çağrı işareti |

## 4. Menülerin tamamı

**BME280 bağlı ve algılandıysa 9 seçenek:**

| Menü | Açıklama | Kullanım |
|---|---|---|
| `BME280 WX` | Sıcaklık, bağıl nem ve basınç | Bilgi ekranı; uzun bas → geri |
| `AUTO BEACON` | Otomatik yayın ON / OFF | Kısa bas → değiştir; uzun bas → kaydet |
| `POS SYMBOL` | APRS konum sembolü | Kısa bas → sonraki; uzun bas → kaydet |
| `UTC OFFSET` | `UTC` veya `UTC+3` | Kısa bas → değiştir; uzun bas → kaydet |
| `DISPLAY` | OLED sürücüsü `SH1106` veya `SSD1306` | Kısa bas → seç; uzun bas → test et |
| `GPS INFO` | Uydu, FIX ve GPS seri veri sayacı | Bilgi ekranı; uzun bas → geri |
| `LORA INFO` | LoRa durumu, paket sayıları, RSSI/SNR, AUTO | Bilgi ekranı; uzun bas → geri |
| `DEVICE INFO` | Cihaz ve firmware bilgisi | Bilgi ekranı; uzun bas → geri |
| `EXIT` | Menüden çıkış | Uzun bas → ana ekran |

**BME280 algılanmazsa** `BME280 WX` maddesi gizlenir; menü 8 seçenek olur.

### AUTO BEACON

- `ON`: Otomatik konum yayınları etkinleşir; uygun koşullarda zamanlayıcı ve Smart Beaconing mantığı çalışır.
- `OFF`: Otomatik yayın kapalıdır. Yeniden başlatma bu tercihi değiştirmez.
- `POS:SMART` ve `WX:60m` gibi ayrıntılar bilgi satırında görünür; **Smart/Fixed ve ayrıntılı aralıkları Web Configurator'dan** ayarlayın.

### POS SYMBOL

V0.9.2'de üç seçenek vardır:

| OLED seçimi | APRS simgesi |
|---|---|
| `CAR />` | Otomobil (`/>`) |
| `WALKER /[` | Yürüyen kişi (`/[`) |
| `HOUSE /-` | Ev (`/-`) |

**Motosiklet ayrı bir seçenek olarak V0.9.2'de henüz bulunmuyor.** Sonraki sürüm için planlandı. Park hâlinde firmware konum paketinde park göstergesini kullanabilir.

### DISPLAY — OLED tipi seçimi (önemli)

1. `DISPLAY` menüsüne girin.
2. Kısa basarak `SH1106` ↔ `SSD1306` arasında geçin.
3. Uzun basarak **10 saniyelik görüntü testini** başlatın.
4. `DISPLAY TEST` yazısı okunaklıysa 10 saniye dolmadan **tekrar uzun basıp onaylayın**.
5. Onaylamazsanız sistem **önceki ekran tipine geri döner**.

**Ekran hiç görünmüyorsa açılış kurtarması:** Cihazın **menü tuşunu basılı tutarak enerji verin** ve tuşu açılışın ilk **3 saniyesi** boyunca bırakmayın. Firmware kayıtlı OLED sürücü tipini diğerine geçirip saklar. Bu işlem **USB bootloader RESET çift tıklamasıyla karıştırılmamalıdır**.

## 5. Beacon ve hava durumu mantığı

- **Smart Beaconing:** GPS hızı ve hareket koşullarına göre konum gönderim aralığı değişir; hareket/dönüş mantığı devrededir.
- **Fixed Position:** Sabit zaman aralığı, panel veya seri komutla seçilir.
- **Parked:** Cihaz bir süre hareket etmezse park durumuna geçebilir. Mevcut cihazda varsayılan park doğrulama süresi 3 dakikadır.
- **WX:** BME280 varsa bağımsız hava durumu APRS paketi oluşturabilir. Varsayılan WX aralığı 60 dakika olarak ayarlanmıştır.
- **Status:** Ayrı durum mesajıdır; gönderim aralığı ayarlanabilir.
- **Comment:** **Ayrı bir APRS paketi değildir; konum Beacon'ına eklenir.**
- Konum TX istekleri arasında en az 10 saniye, WX/Status gibi yardımcı TX isteklerinde ise son yayınla arasında en az 30 saniye koruma bulunur. Bunlar gerçek yayın periyotlarının yerine geçmez.

## 6. Web Configurator ve yedekleme

[**TA1FM Web Configurator'ı aç**](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)

1. USB bağlantısını kurun.
2. **Cihazdan Oku** ile ayarları alın. Panelde bir değer görünüyor olması, mutlaka cihazdan okunmuş olduğu anlamına gelmez; bağlantı ve okuma sonucunu kontrol edin.
3. İstediğiniz alanları düzenleyin.
4. **Cihaza Yaz** ile aktarın; başarılı kayıt mesajlarını kontrol edin.
5. **JSON yedek** alın. Geri yükleme sonrasında ayarları yeniden cihaza yazmanız gerekebilir.
6. Enerjiyi kesip yeniden verdikten sonra tekrar okuyarak kalıcılığı sınayabilirsiniz.

Seri monitör, ileri seviye kullanıcılar için cihazın tanılama mesajlarını ve komut yanıtlarını gösterir. **Aynı COM portunu birden fazla program eşzamanlı kullanmamalıdır.**

## 7. İleri düzey: seri komut özeti

Normal kullanıcıların bunları bilmesine gerek yoktur; web paneli tercih edilir.

| Komut | İşlev |
|---|---|
| `p` | Durumu ve ayarları göster |
| `c TA1FM-7` | Çağrı işareti kaydet (örnek) |
| `a on` / `a off` | AUTO açık / kapalı |
| `f smart` / `f fixed` | Konum modu |
| `i 1` / `i 3` / `i 5` / `i 10` / `i 15` | Sabit konum aralığı (dakika) |
| `s car` / `s walker` / `s house` | APRS sembolü |
| `k 1` / `k 3` / `k 5` | Park süresi (dakika) |
| `x off` / `x 15` / `x 30` / `x 60` | WX aralığı |
| `z off` / `z 15` / `z 30` / `z 60` | Status aralığı |
| `j off` / `j 15` / `j 30` / `j 60` | Comment ekleme aralığı |
| `comment METIN` | Comment metni |
| `status METIN` | Status metni |
| `comment?` / `status?` | Metinleri oku |
| `t` / `w` / `v` | Manuel konum / WX / Status TX |

USB seri bağlantı hızı **115200 baud**. Comment ve Status metinleri firmware'de ASCII, 1–62 karakter ve `|` ile `~` dışındaki karakterler olacak şekilde sınırlandırılmıştır.

## 8. Sık karşılaşılan durumlar

| Belirti | İlk kontrol |
|---|---|
| GPS:SEARCH | Anten/görüş alanı, GPS güç beslemesi, uydu alımı |
| TX yapılmıyor | `p` çıktısında çağrı işareti geçerli mi, `AUTO=ON` mu, `GPS fix=YES` mi, LoRa hazır mı? |
| Ekran karanlık | SH1106/SSD1306 seçimi; 3 saniyelik açılış OLED kurtarması |
| BME280 menüsü yok | Sensör algılanmamış olabilir; bağlantıları ve I2C adresini kontrol edin |
| Ayar kaydedilmiyor | Panel/seri monitörde `saved` veya `SAVE FAILED` mesajını kontrol edin |
| USB bağlantısı yok | Veri aktarabilen USB kablosu, Chrome/Edge, başka uygulamanın COM portunu kapatması |

## 9. Faydalı bağlantılar

- [Firmware kaynak kodu ve UF2 — GitHub](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker)
- [Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)
- [Üçüncü taraf temizleme araçları](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/tools/README.md)

**73 de TA1FM!**
