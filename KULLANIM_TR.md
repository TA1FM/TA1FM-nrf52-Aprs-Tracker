> **✅ GÜNCEL SÜRÜM: v1.0.0 STABLE (17.11.2026)** — [Release ve UF2/INO dosyaları](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.0.0) · [Güncel USB + BLE kontrol paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/). **Bu belgenin aşağıdaki ayrıntılı talimatları eski v0.9.24 sürümünü temel alır ve arşiv amaçlı korunur.** v1.0.0 ayrıca USB/BLE APRS mesajlaşmasını destekler; eski sürüm bağlantılarını güncel indirme olarak kullanmayın.

# TA1FM nrf52 Tracker + Digipeater — Kullanım Kılavuzu (Türkçe)

**Güncel sürüm:** V0.9.24 STABLE · **Hedef:** TA2DMX MiniMesh Lite NRF / nRF52840 · **LoRa APRS:** 433.775 MHz, SF12, BW125 kHz, CR 4/5

Bu kılavuz doğrudan **V0.9.24 STABLE** kaynak kodu ve bu sürümle uyumlu USB/BLE ayar paneli temel alınarak **baştan yazılmıştır**. Önceki sürümün talimatlarını art arda ekleyen bir değişiklik listesi değildir. **TRACKER ve DIGIPEATER** kullanımı tek belgede açıklanır.

**Hızlı bağlantılar:** [Hazır STABLE UF2](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.uf2) · [Arduino kaynak kodu](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.ino) · [Güncel Web Ayar Paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html) · [Adım adım kurulum](./KURULUM_TR.md) · [Tüm USB konsol komutları](./SERI_KONSOL_KOMUT_REHBERI_V0_9_24_TR.md)

## 1. Cihazın iki çalışma modu

| Mod | Görevi | Konum kaynağı |
|---|---|---|
| **TRACKER** | GPS konumunu SMART veya FIXED zamanlamayla APRS LoRa üzerinden yayınlar; hareket, park, WX/Status ve Comment özelliklerini kullanır. | Güncel GPS FIX |
| **SABİT DIGI / DIGIPEATER** | Uygun WIDE yoluyla gelen APRS paketlerini LoRa RF'de tekrarlar; kayıtlı sabit istasyon konumunu belirlenen aralıkta beacon olarak gönderir. | Kaydedilmiş enlem/boylam |

**Modlar aynı anda çalışmaz.** Yeni kurulmuş ayarlarda TRACKER seçilidir. DIGI bir **RF tekrarlayıcıdır**; internet APRS-IS iGate değildir. İnternette görünürlük için bir iGate'in LoRa paketlerini alması gerekir.

Modu [Web Ayar Paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html) içindeki **Çalışma modu** alanından veya USB seri komutları `mode tracker` / `mode digi` ile değiştirirsiniz. DIGI'ye geçmek için **geçerli çağrı işareti ve sıfır olmayan iki sabit koordinat** gerekir.

## 2. İlk kullanım: en kısa yol

1. Cihazınıza uygun V0.9.24 STABLE UF2'yi yükleyin. **Eski çalışan UF2 ve JSON ayar yedeğini saklayın.** Ayrıntılar [kurulum rehberinde](./KURULUM_TR.md).
2. Anteni bağlayın; SX1262/SX1268 donanım seçimini doğrulayın. Yanlış çip seçimi RF'yi çalıştırmayabilir.
3. Bilgisayarda USB veya Android Chrome'da Bluetooth BLE ile [ayar paneline](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html) bağlanın; **önce cihazdan gerçek ayarları okuyun**.
4. **Çağrı işareti**, **TRACKER/DIGI**, beacon, sembol, TX güç, OLED ve Bluetooth ayarlarını kontrol edin; değişiklikleri kaydedin.
5. **TRACKER** kullanıyorsanız açık gökyüzünde `GPS:FIX` bekleyin. **DIGI** kullanıyorsanız doğru sabit koordinatları ve WIDE yolunu ayarlayın.
6. Test için USB'de `p` / `mode?` ile durumu, gönderimden sonra yerel TX kaydını ve başka bir istasyondan gerçekten duyulup duyulmadığını kontrol edin.

**Önemli:** Geçerli çağrı işareti yoksa `NOCALL` durumunda RF TX engellenir. İlk geçerli çağrı işaretinin kaydı AUTO'yu açabilir; daha sonra bilerek ayarlanan AUTO OFF tercihi korunur.

## 3. Fiziksel düğme ve OLED ekranı

**Menü düğmesi**, UF2 bootloader'a geçiren **RESET düğmesinden farklıdır**.

| Düğme hareketi | Ana ekran | Menüde | Ayar sayfasında |
|---|---|---|---|
| **Kısa basış** | Manuel konum beacon isteği (TX koşullarına bağlı) | Sonraki madde | Seçeneği değiştirir |
| **Uzun basış (yaklaşık 1,2 sn)** | Menüyü açar | Seçili maddeye girer | Kaydeder/onaylar veya geri döner |

OLED uykuya geçmişse ilk kısa basış sadece ekranı uyandırır; yanlışlıkla TX yapmaz. Menüde 30 saniye işlem yapılmazsa ana ekrana dönülür.

**Ana ekran** üzerindeki bilgiler: çağrı işareti ve GPS saati (UTC / UTC+3), `GPS:FIX` veya `GPS:SEARCH`, `SAT` uydu sayısı, `SPD` hız (km/sa), `LAT` / `LON`, `BAT` yaklaşık pil gerilimi ve `RX` son duyulan çağrı işareti. GPS koordinatları henüz güncel değilse yerlerinde `--` görünür. DIGI modundayken ana ekranda yine GPS alanları gösterilebilir; **DIGI'nin kendi beacon'ı kayıtlı sabit koordinatları kullanır**.

### V0.9.24 OLED menüsü

BME280 bulunursa **11**, bulunmazsa **10** madde gösterilir.

| Menü | İşlev |
|---|---|
| `BME280 WX` (sensör algılanırsa) | Sıcaklık, nem, basınç |
| `AUTO BEACON` | TRACKER otomatik beacon açık/kapalı |
| `POS SYMBOL` | Hazır araç / yaya / ev simgesi |
| `UTC OFFSET` | UTC veya UTC+3 |
| `DISPLAY` | SH1106 / SSD1306 ekran sürücüsü |
| `OLED TIMER` | Ekran otomatik kapanma süresi |
| `TX POWER` | 1–22 dBm hedef TX gücü |
| **`BLE ON/OFF`** | Bluetooth bağlantısını aç/kapat (**V0.9.24 menüsünde var**) |
| `LORA INFO` | RF/RX, son RSSI-SNR ve paket sayıları |
| `DEVICE INFO` | Cihaz/sürüm bilgisi, V0.9.24 |
| `EXIT` | Ana ekrana dön |

TRACKER/DIGI mod seçimi **OLED menüsünde yoktur**; Web Ayar Paneli veya USB seri komutuyla yapılır.

### OLED tipi ve kurtarma

- Fiziksel `DISPLAY` menüsünde SH1106/SSD1306 seçilir; **10 saniyelik** önizleme sırasında uzun basarak onaylanır. Onay verilmezse eski sürücüye döner.
- USB konsolunda `oled sh1106` veya `oled ssd1306` önizlemesi **20 saniye** sürer; `oled confirm` / `oled cancel` kullanılır. USB web panelinin kaydetme akışı önizleme/onayı kendi yürütür.
- BLE panelinde sürücü değişimi **önizleme olmadan doğrudan kaydedilir**. Yanlış sürücü seçilirse açılışta **menü düğmesini kesintisiz 3 saniye basılı tutarak** alternatif sürücüye geçebilirsiniz. Bu kurtarma için RESET'e çift basılmaz.
- `OLED TIMER`: sürekli açık, sürekli kapalı, **15 sn**, **30 sn (varsayılan)**, 1 dk, 2 dk veya 5 dk. Sadece OLED kapanır; GPS, LoRa ve beacon çalışmaya devam eder.

## 4. TRACKER: SMART ve FIXED Beaconing

**TRACKER + AUTO ON + geçerli çağrı işareti + güncel GPS FIX + hazır LoRa** koşullarında otomatik konum gönderimi yapılır.

| GPS hızı | SMART varsayılan konum aralığı |
|---|---:|
| 0–7 km/sa | 600 saniye |
| 10 km/sa | 90 saniye |
| 20 km/sa | 45 saniye |
| 49 km/sa | 30 saniye |
| 60 km/sa | 20 saniye |
| 90 km/sa ve üzeri | 15 saniye |

Ara hızlar arasında yazılım süre hesaplar. Altı noktanın süresi panelden veya örneğin `smart 600,90,45,30,20,15` komutuyla değiştirilebilir; hızlı noktaların süreleri daha yavaş noktalarınkinden uzun olmamalıdır. İlk değer en az 15 saniye, diğerleri en az 10 saniye; üst sınır 3600 saniyedir.

- **İlk hareket yayını:** Özellik açıkken GPS hızı 8 km/sa ve üstünde art arda doğrulanırsa, ilk konum gönderildikten sonra erken yeni konum istenebilir.
- **Park:** 5 km/sa ve altındaki duruş park adayıdır. Park onayı 1, 3 (varsayılan) veya 5 dakika olabilir; parkta ayrı sembol kullanılır.
- **Viraj:** Geçerli hız ve yönle, en az 10 km/sa hız ve son gönderilen yönle en az 35° fark yeni konum isteği doğurabilir.
- **FIXED:** Hızdan bağımsız 1 / 3 / 5 / 10 / 15 dakikalık zaman aralıklarından biri kullanılır.

**V0.9.24 ortak RF koruması en az 10 saniyedir** (eski belgelerdeki 15 saniye bu sürüm için yanlıştır). Gönderim süresi, iGate alımını veya aprs.fi görünürlüğünü garanti etmez.

### Hareket ve park sembolleri

Hareket ve park sembolleri **ayrı ayrı** kaydedilir. Örnekler: `/>` araç, `/[` yaya, `/-` ev; park varsayılanı `\P`. Her kod iki karakterdir: ilk karakter `/` veya `\`, ikincisi APRS sembolüdür. USB: `symbol />`, `parksymbol \P`.

## 5. APRS konum, Comment, Status, WX ve pil

| Özellik | Nasıl çalışır? |
|---|---|
| **Position / konum** | TRACKER'da güncel GPS ile; DIGI'de kayıtlı sabit koordinatlarla gönderilir. |
| **Comment** | **Ayrı RF paketi değil**, uygun konum beacon'ının sonuna eklenen metindir. Kapalı, her konum paketinde veya 15/30/60 dk aralıkta seçilir. |
| **Status** | Ayrı APRS Status paketi olarak gönderilir; kapalı/15/30/60 dk aralığı vardır. |
| **WX** | BME280 ile sıcaklık/nem/istasyon basıncı; kapalı/15/30/60 dk aralığı. Geçerli GPS FIX ve taze sensör verisi gerektirir. |
| **Pil voltajı** | Açık ise geçerli yaklaşık ölçüm `Batt=3.92V` gibi konum yorumuna eklenir; ayrı RF paketi üretmez. |

TRACKER ilk açılışında uygun koşullarda önce konum (Comment dahil), ardından en az 10 saniyelik RF korumadan sonra ayrı Status paketi gönderilebilir. WX/Status zamanlayıcıları birbirinden bağımsızdır. DIGI modunda otomatik TRACKER konum/WX/Status döngüsü çalışmaz; bunun yerine DIGI'nin **sabit beacon zamanlayıcısı** kullanılır. Manuel USB `t`, `w`, `v` komutları ayrıca yayın koşullarına bağlıdır.

**BME280 sınırlaması:** Rüzgâr ve yağmur sensörü olmadığından bu değerler ölçülmüş gibi üretilmez; basınç deniz seviyesine düzeltilmemiş istasyon basıncıdır. `w` için GPS FIX ve geçerli BME280 okuması gerekir.

Comment/Status metinleri **1–62 ASCII karakter** olmalı; `|` ve `~` kullanılamaz. Türkçe özel harfler yerine ASCII metin girin.

## 6. SABİT DIGIPEATER (DIGI)

**DIGI modunda** cihaz 433.775 MHz LoRa APRS kanalında uygun WIDE yoluna sahip gelen paketleri RF üzerinden tekrarlar, kendi çağrı işaretini tekrarlanan yol üzerinde işaretler ve kayıtlı sabit koordinatından beacon gönderir.

| Ayar | Seçenek |
|---|---|
| **Çalışma modu** | TRACKER veya DIGI |
| **Sabit konum** | Ondalık derece ile enlem ve boylam (sıfır olmayan değerler) |
| **DIGI PATH 1** | Yalnız `WIDE1-1` |
| **DIGI PATH 2** | `WIDE1-1`, `WIDE2-1`, `WIDE2-2` |
| **Kendi beacon aralığı** | 10 / 15 / 30 / 60 dakika (varsayılan 15) |
| **Mükerrer filtre** | Aynı kaynak çağrı işareti + APRS bilgi içeriği için 30 saniye |
| **Kuyruk / RF** | En fazla 4 paket; gönderimler arasında en az 10 saniye; bekleyen paket için zaman aşımı 45 saniye |

Örnek (temsili):

`TA1FM-9>APZTFM,WIDE2-2:!...` → `TA1FM-9>APZTFM,TA1FM-6*,WIDE2-1:!...`

**DIGI ayar sırası:** Geçerli çağrı işaretini girin → sabit enlem/boylamı kaydedin → WIDE yolunu ve beacon aralığını seçin → `mode digi` veya Web Ayar Paneli'nden DIGI'yi etkinleştirin. Sabit konum için GPS FIX gerekmez.

USB üzerinden örnek:

```text
digi lat 40.97500
digi lon 27.50400
digi path 2
digi beacon 15
mode digi
mode?
```

Bu koordinatlar **örnektir**; gerçek istasyon koordinatlarınızı girin. Geri dönüş için `mode tracker`. TRACKER'ın SMART/FIXED ayarları korunur. Seri monitör `digi?` ve `p` ile `Eligible`, `TX`, `TX_ERR`, `DUP`, `REJECT`, `QUEUE_FULL`, `EXPIRED`, `RX_RECOVER` ve `RX_RECOVER_ERR` sayaçlarını görebilirsiniz.

**Önemli:** `[DIGI TX OK]` cihazın yerel RadioLib TX sonucudur; karşı taraftaki alımı kanıtlamaz. NOGATE/RFONLY gibi yollar tekrarlamaya uygun kabul edilmez. 30 saniyelik DIGI filtresi ve RX toparlanması bu sürümde mevcuttur; **SX1268 TRACKER saha testli olmasına rağmen yeni DIGI mantığının ve SX1262'nin uzun süreli saha doğrulaması ayrıca gereklidir.**

## 7. USB ve Bluetooth BLE Web Ayar Paneli

**Güncel/uyumlu panel:** https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html

Panel adı/protokolü V0.9.23 olarak kalmış olabilir; **V0.9.24'ün ayar protokolüyle uyumludur**. Firmware'deki `[V23]` ve `CFG|BEGIN|V0.9.23` alanları geriye uyumluluk için bilerek korunmuştur; bunlar firmware'in eski olduğu anlamına gelmez.

| İşlem | USB | Bluetooth BLE |
|---|---|---|
| Bağlanma | Bilgisayarda Chrome/Edge ve USB veri kablosu | Android Chrome, HTTPS, cihaz adı `nRF52_Lora_Tracker` |
| Ayar okuma | **Cihaza bağlan → Cihazdan oku (p)** | **Bluetooth ile bağlan → BLE Ayarları Oku** |
| Kayıt | **Ayarları cihaza yaz** | **Kaydetme açık → BLE Ayarları Kaydet** |
| Canlı temel bilgi | GPS/RX/TX, son RX, seri kayıtlar | Çağrı işareti, çip, GPS FIX, uydu, RF ve batarya |
| Manuel konum/WX/Status TX | USB bölümündeki `t` / `w` / `v` düğmeleri | **V0.9.24'te BLE üzerinden desteklenmez** |
| Yedek | Ayarları indir (JSON) / yedekten forma yükle | Aynı sayfanın formuyla kullanılabilir; **içe alma cihazı kendiliğinden güncellemez** |

**BLE güvenliği:** Kaydetme anahtarı bir PIN/şifre veya kimlik doğrulaması **değildir**. Eşleştirmesiz NUS bağlantısında kayıt izni açıldığında yakın erişim riski vardır; başarılı kayıt ve bağlantı kesiminde anahtar kapanır. BLE kaydı değişiklikleri RAM taslağında hazırlar ve **tek Direct Flash A/B işlemiyle** yazar.

BLE'den **Bluetooth OFF** kaydedilirse bağlantı kesilir; OLED `BLE ON/OFF` menüsü veya USB `ble on` ile yeniden açılır. LoRa çipi değiştirilirse **yeniden başlatma zorunludur**; BLE'de çip ve TX gücünü aynı kayıtta birlikte değiştirmeyin.

USB Seri Monitör ile USB Web Ayar Paneli aynı COM portunu aynı anda kullanamaz. Bluetooth canlı izleme USB'den bağımsızdır.

## 8. Seri konsoldan hızlı kontroller

**USB seri:** 115200 baud, satır sonu **Newline**. Ayrıntılı, eksiksiz liste için [Seri Konsol Komut Rehberi](./SERI_KONSOL_KOMUT_REHBERI_V0_9_24_TR.md).

| Komut | İşlev |
|---|---|
| `p` / `?` | Genel durum, GPS/RF/TX, mod ve ayarlar |
| `g` / `r` / `b` | GPS / LoRa / BME280 tanılama |
| `mode?` / `digi?` | TRACKER/DIGI, yol ve DIGI sayaçları |
| `a on` / `a off` | TRACKER AUTO yayını |
| `f smart` / `f fixed` | TRACKER konum zamanlaması |
| `smart?` / `power?` / `chip?` | SMART, TX güç ve çip sorgusu |
| `t` / `w` / `v` | Manuel konum / WX / Status (gerçek RF TX) |
| `ble on` / `ble off` / `ble?` | Bluetooth durumu |
| `oled?` / `sleep 0..6` | OLED ve uyku ayarları |
| `flash?` | Direct Flash A/B durumu |
| `flash init` | **Yalnız geçerli A/B kayıt bulunmayan ilk kurulumda**, ileri seviye |

**Asla rutin güncelleme için `flash init` çalıştırmayın.** Kaydı olmayan cihazda, doğru çip ve ayarları belirleyip yedek aldığınızda ilk kayıt için kullanılır. Sıradan firmware yükseltmesinde flash silici/format gerekmez.

## 9. Sorun giderme

| Belirti | Bakılacak yer |
|---|---|
| `GPS:SEARCH`, konum gönderilmiyor | TRACKER modunu, GPS antenini ve gökyüzünü; `g` ve `p` çıktısını kontrol edin. |
| DIGI modunda GPS yok | Sabit koordinat kullanıldığı için tek başına sorun değildir; `mode?` ile DIGI LAT/LON ve çağrı işaretini kontrol edin. |
| Haritada paketler eksik | Gerçek TX ile uzaktaki iGate alımını ayırın; kapsama, anten, SF/BW ve WIDE yollarını kontrol edin. |
| `[DIGI TX OK]` var, aprs.fi yok | Bir iGate'in paketi RF üzerinden alıp APRS-IS'e aktarması gerekir. |
| `mode digi` kabul edilmiyor | Geçerli çağrı işareti ve iki sabit koordinat kaydetmeden DIGI açılamaz. |
| LoRa RF hata / çip yanlış | Gerçek SX1262/SX1268'i seçin, kaydedip yeniden başlatın. |
| OLED siyah | SH1106/SSD1306 uyumu; cihaz açılışında menü düğmesiyle 3 sn kurtarma. |
| BLE bulunamıyor | OLED'de BLE ON mu? Android Chrome + HTTPS, yakın bağlantı, başka BLE istemcisi bağlı mı? |
| Ayarlar saklanmıyor | `flash?` ile DIRECT ACTIVE, USB/BLE SAVE sonucu ve yeniden okumayı inceleyin. |
| USB panel bağlanmıyor | USB veri kablosu, Chrome/Edge ve COM portunu kullanan Seri Monitör'ü kontrol edin. |
| WX gönderilmiyor | BME280 algılandı mı, GPS FIX geçerli mi, 10 sn RF koruması doldu mu? |

## 10. Güvenlik, test ve güncelleme kapsamı

- Bu firmware her nRF52840 kartına uygun değildir. MiniMesh Lite NRF varyantı, UF2 bootloader/SoftDevice, çip ve pin eşleşmesi doğrulanmalıdır.
- Anten veya uygun RF yükü bağlı olmadan TX yapmayın; 1–22 dBm yazılım ayarı gerçek anten çıkışı değildir. Ülkenizin amatör telsiz tahsislerine ve donanım sınırlarına uyun.
- Direct Flash A/B + CRC32 kullanılır; LittleFS kullanılmaz. Normal güncellemede ayarları silmeyin.
- **STABLE .ino ve .uf2**, sahada test edilmiş V0.9.24 BETA dosyalarının birebir aynısıdır. Başlangıç seri çıktısında `BETA` yazısı kalabilir; OLED/INFO sürümü V0.9.24'tür.
- **Saha doğrulaması:** 15 Kasım 2026'da yaklaşık 10 km/30 dakika **SX1268 TRACKER** modu test edildi. DIGI'nin yeni filtre/toparlanması ve SX1262 ayrıca saha testi gerektirir.

## 11. Diğer belgeler

- [Kurulum ve ilk ayar rehberi — V0.9.24](./KURULUM_TR.md)
- [DIGIPEATER ayarları ve WIDE örnekleri](./DIGIPEATER_V0_9_24_TR.md)
- [Tüm USB seri konsol komutları](./SERI_KONSOL_KOMUT_REHBERI_V0_9_24_TR.md)
- [V0.9.24 STABLE sürüm notları](./V0_9_24_STABLE_NOTLARI.md)
- [GitHub Release v0.9.24](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v0.9.24)
- [Pin bağlantıları](./PIN_BAGLANTILARI_TR.md)

**TA1FM nrf52 Tracker + Digipeater · V0.9.24 STABLE — 73 de TA1FM!**
