> **✅ GÜNCEL SÜRÜM: v1.0.0 STABLE (17.11.2026)** — [Release ve UF2/INO dosyaları](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.0.0) · [Güncel USB + BLE kontrol paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/). **Bu belgenin aşağıdaki ayrıntılı talimatları eski v0.9.24 sürümünü temel alır ve arşiv amaçlı korunur.** v1.0.0 ayrıca USB/BLE APRS mesajlaşmasını destekler; eski sürüm bağlantılarını güncel indirme olarak kullanmayın.

# TA1FM nrf52 Tracker + Digipeater — Kurulum ve İlk Ayarlar (Türkçe)

**Hedef sürüm:** V0.9.24 STABLE · **Donanım:** TA2DMX MiniMesh Lite NRF / nRF52840 · **LoRa APRS:** 433.775 MHz / SF12 / BW125 / CR 4/5

Bu belge, eski sürüm kılavuzlarının üstüne eklenmiş notlar yerine, **V0.9.24 STABLE kaynak koduna ve uyumlu ayar paneline göre yeniden yazılmış eksiksiz kurulum akışıdır**. TRACKER ve yeni **sabit DIGIPEATER (DIGI)** kurulumunu birlikte kapsar.

**Önce şunları açın:**
- **[V0.9.24 STABLE UF2 — hazır firmware](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.uf2)**
- **[V0.9.24 STABLE .ino — kaynak kod](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.ino)**
- **[V0.9.24 uyumlu USB + BLE Web Ayar Paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html)**
- [V0.9.24 kullanım rehberi](./KULLANIM_TR.md) · [Seri konsolun tüm komutları](./SERI_KONSOL_KOMUT_REHBERI_V0_9_24_TR.md)

## 1. Uyumluluk ve hazırlık

| Bileşen | Gereksinim |
|---|---|
| Kart | TA2DMX MiniMesh Lite NRF / projeye uygun nRF52840 kart varyantı |
| LoRa | Fiziksel modüle göre **SX1262 veya SX1268** (LLCC68 bu proje ayarları için desteklenmez) |
| GPS | ATMG336H VEYA NEO6MV2  gibi / uygun GPS modülü, firmware'de `Serial1` 9600 baud |
| OLED | 128×64 **SH1106 veya SSD1306**, I²C `0x3C` |
| BME280 | İsteğe bağlı WX sensörü; I²C `0x76` veya `0x77` |
| USB/bootloader | Kartla eşleşen nRF52840 **UF2 bootloader** ve doğru pin tanımı |
| RF | 433.775 MHz e uygun UHF anten |

**Her nRF52840 kartına bu UF2'yi yüklemeyin.** Bootloader, SoftDevice, SPI, RF anahtarı, TCXO, GPS, tuş ve OLED pinlerinin bu MiniMesh projesiyle uyumlu olması gerekir. TX öncesinde anten bağlayın.

**Yedek hazırlığı:** Çalışan .uf2 dosyanızı saklayın; [Web Ayar Paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html) içinde **Ayarları indir (JSON)** ile cihaz ayarlarını dışa aktarın. JSON yalnızca ayar değerlerini içerir; firmware yedeğinin yerine geçmez.

## 2. V0.9.24 STABLE UF2 yükleme

1. [V0.9.24 STABLE UF2](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.uf2) dosyasını indirin. Bilgisayara doğru dosyayı kaydettiğinizi kontrol edin.
2. Kartı **veri aktarabilen USB kablosu** ile bilgisayara bağlayın.
3. Kartın **RESET** düğmesine art arda **iki kez** basın. Bilgisayarda kartın UF2 sürücüsü görünmelidir (sürücü adı bootloader'a göre değişebilir).
4. `TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.uf2` dosyasını UF2 sürücüsüne kopyalayın. İşlem tamamlanınca kart yeniden başlar.
5. Anten bağlıyken OLED başlangıç ekranında **V0.9.24** ve LoRa durumunu kontrol edin; USB seri konsolda 115200 baud ile başlangıç mesajlarını okuyabilirsiniz.
6. İlk kurulum ayarlarını cihazdan okuyarak doğrulayın (bölüm 3 ve 4). **LoRa çipini değiştirirseniz cihazı yeniden başlatın.**

**Normal güncellemede Eraser kullanmayın, flash'ı formatlamayın, `flash init` çalıştırmayın.** Kalıcı ayarlar **Direct Flash A/B + CRC32** yapısındadır; LittleFS başlatılmaz. STABLE UF2 sahada denenmiş V0.9.24 BETA ile birebir aynı dosyadır; USB açılış satırında `BETA` yazması bu nedenle mümkündür.

## 3. USB ile ilk ayar: bilgisayar

1. Masaüstü **Chrome veya Edge** kullanın: [V0.9.24 Web Ayar Paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html).
2. Arduino Seri Monitör, başka terminal veya USB Web Ayar Paneli'nin ikinci sekmesi aynı COM portunu kullanıyorsa kapatın.
3. **Cihaza bağlan** seçin, doğru USB seri portunu açın; **Cihazdan oku (p)** ile mevcut değerleri çekin.
4. `Direct Flash: ACTIVE` durumunu ve **SX1262/SX1268 gerçek donanım seçimini** kontrol edin. Panel yalnızca başarılı okuma yaptıktan sonra ayar yazmak için kullanılmalıdır.
5. Çağrı işareti, çalışma modu, APRS sembolleri, AUTO, SMART/FIXED, WX/Status/Comment, OLED ve TX güç seçeneklerini düzenleyin.
6. **Ayarları cihaza yaz** seçin. Panel değişen USB ayarlarını seri komutlarla gönderir; logdaki kayıt başarı/hata mesajlarını inceleyin. Sonra tekrar **Cihazdan oku** yapın.
7. Yeni ayarların güç kesilip açıldıktan sonra da saklandığını doğrulayın.

**USB bağlantısı üzerinden:** Manüel `t` (konum), `w` (WX), `v` (Status) düğmeleri **gerçek RF gönderimi yapar**. Anten bağlı ve test koşulları uygunsa kullanın. **USB panelindeki canlı GPS/LoRa/TX/RX bölümü USB'ye özeldir.**

**LoRa çipi değişimi:** USB seri konsolda `chip sx1262` / `chip sx1268` komutları desteklenir; panelde çip değişimini BLE kaydıyla yapmak da mümkündür. Değişen çip ayarı **yeniden başlatılana kadar aktif olmaz**; yanlış çip seçimi RF haberleşmesini engeller.

## 4. Bluetooth BLE ile ilk ayar: Android

1. Telefonda Bluetooth'u açın; **Android Chrome** ile paneli **HTTPS** üzerinden açın: [V0.9.24 uyumlu panel](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html).
2. **Bluetooth ile bağlan** → listeden **`nRF52_Lora_Tracker`** cihazını seçin. Başka BLE uygulaması bağlıysa önce bağlantısını kesin.
3. **BLE Ayarları Oku** ile cihazdan gerçek değerleri okuyun. Canlı BLE bölümünde çağrı işareti, LoRa çipi, GPS FIX, uydu, RF durumu ve pil bilgisi bulunur; yaklaşık 10 saniyede bir durum verisi gönderilir.
4. Değiştireceğiniz alanları düzenleyin. Kaydetmeden önce **Kaydetme açık** anahtarını AÇIK konuma getirin.
5. **BLE Ayarları Kaydet** seçin. Yazılım farklı alanları RAM taslağına alır ve **tek Direct Flash A/B işlemiyle** kaydeder.
6. Başarıdan sonra anahtar otomatik kapanır; **BLE Ayarları Oku** ile değerleri yeniden doğrulayın.

**BLE şifre/PIN uyarısı:** Kaydetme anahtarı bir güvenlik/kimlik doğrulama sistemi değildir. Bağlantı **eşleştirmesiz Nordic UART Service** kullanır; yakındaki istemcilerle ilgili erişim riskini dikkate alın. Kaydetme izni başarılı işlem veya bağlantı kesiminden sonra kapanır.

- **BLE OFF** kaydederseniz Bluetooth bağlantısı kesilir. USB `ble on` veya OLED **BLE ON/OFF** menüsü ile tekrar açabilirsiniz.
- **OLED tipi BLE'de doğrudan kaydedilir**, 20 sn önizleme yapılmaz. Hatalı ekran seçimi için cihaz açılırken **menü düğmesini kesintisiz 3 sn** tutarak kurtarma yapın.
- **LoRa çipi değişimi** yeniden başlatma gerektirir; BLE'de çip ve TX gücünü **aynı kayıt işleminde** değiştirmeyin.
- **BLE üzerinden manuel `t` / `w` / `v` RF TX desteklenmez**. Bunlar USB seri komutlarıdır. BLE'de RX son istasyon ve TX/WX sayaçları USB'deki kadar ayrıntılı gösterilmez.
- Panelde `V0.9.23` etiketleri görünmesi normaldir: firmware, geriye uyumluluk için `CFG|BEGIN|V0.9.23` / `[V23]` protokol işaretlerini korur.

## 5. İlk kurulum ortak ayar kontrol listesi

| Alan | Doğru seçim / dikkat |
|---|---|
| Çağrı işareti | Kendi yetkili çağrı işareti/SSID; `NOCALL` durumunda TX engellenir |
| Çalışma modu | **TRACKER** (GPS) veya **DIGI** (sabit tekrarlayıcı) |
| LoRa çipi | Gerçek **SX1262/SX1268** donanımıyla eşleşsin; değişince yeniden başlat |
| BLE | İlk ayarda açık; istenirse OLED/USB/BLE ile kapatılabilir |
| OLED tipi | SH1106 veya SSD1306; yanlış sürücüde açılış tuşuyla 3 sn kurtarma |
| OLED kapanma | Sürekli açık/kapalı; 15 sn, **30 sn varsayılan**, 1, 2 veya 5 dk |
| TX gücü | **1–22 dBm hedefi**; gerçek RF çıkışını ve yasal sınırları ayrı doğrulayın |
| TRACKER AUTO | ON/OFF; GPS FIX, LoRa ve çağrı işareti koşullarına bağlı |
| TRACKER konum | SMART hız aralıkları veya FIXED 1/3/5/10/15 dk |
| APRS sembolleri | Hareket/park ayrı iki karakter; ör. `/>` ve `\P` |
| WX / Status / Comment | WX ve Status için OFF/15/30/60 dk; Comment için OFF/EACH/15/30/60 dk |
| DIGI | Gerçek sabit enlem/boylam + PATH 1/2 + 10/15/30/60 dk beacon |

**V0.9.24 RF koruması: en az 10 saniye.** Önceki eski kılavuzlarda görünen 15 saniye bilgisi bu sürüme ait değildir.

### TRACKER için ilk çalıştırma

1. `Çalışma modu = TRACKER`, geçerli çağrı işareti, `AUTO = ON` ve `SMART` (veya istenirse FIXED) ayarını seçin.
2. GPS anteninin gökyüzünü görmesini sağlayın; OLED üzerinde **`GPS:FIX`** görünmeli. `GPS:SEARCH` iken konum TX engellenir.
3. SMART varsayılanları: **7/10/20/49/60/90 km/sa** için sırasıyla **600/90/45/30/20/15 sn**. İlk hareket tetiklemesi ve park sembolü seçeneklerini kontrol edin.
4. WX göndermek istiyorsanız BME280 olmalı ve GPS FIX geçerli olmalıdır. Comment ayrı bir paket değil konum paketinin metnidir; pil voltajı da konum yorumuna eklenebilir.
5. USB'de `p`, `g`, `r` ve gerektiğinde `t` ile testi yapın. Yerel TX OK mesajı, başka iGate'in paketi aldığı anlamına gelmez.

### SABİT DIGIPEATER için ilk çalıştırma

1. DIGI istasyonu için geçerli çağrı işaretinizi seçin.
2. Gerçek istasyonun **enlem ve boylamını ondalık dereceyle** girin. Sıfır koordinatlar DIGI moduna geçişte reddedilir. Sabit DIGI beacon'ı GPS FIX gerektirmez.
3. **DIGI PATH 1:** sadece `WIDE1-1`. **DIGI PATH 2:** `WIDE1-1`, `WIDE2-1` ve `WIDE2-2`. İlk denemede yerel trafiğe uygun `PATH 1` düşünülebilir.
4. **DIGI Beacon**: sabit konum yayını için **10, 15 (varsayılan), 30 veya 60 dk**.
5. Son olarak **Çalışma modu = DIGI** kaydedin. TRACKER ve DIGI aynı anda çalışmaz.
6. İkinci bir LoRa APRS istasyonundan uygun WIDE paketini gönderin; seri konsolda `[DIGI QUEUE]` / `[DIGI TX OK]` ve `digi?` sayaçlarını inceleyin. Gerçek RF alımını ayrıca doğrulayın.

**USB seri örneği** (koordinatlar yalnız örnek, gerçek konumu kullanın):

```text
c TA1FM-6
digi lat 40.97500
digi lon 27.50400
digi path 2
digi beacon 15
mode digi
mode?
```

DIGI'nin mükerrer filtresi **30 sn**, kuyruğu **4 paket**; arka arkaya TX arasında **10 sn** koruma vardır. `WIDE2-2` uygun pakette çağrı işareti eklenir ve kalan `WIDE2-1` yoluna düşürülür. Bu **internet iGate** değildir. Ayrıntılar: [DIGIPEATER rehberi](./DIGIPEATER_V0_9_24_TR.md).

## 6. Direct Flash A/B ve ayar koruma

- Cihaz ayarları **Direct Flash A/B + CRC32** yapısında tutulur; **LittleFS** kullanılmaz.
- USB seri monitörü 115200 baud ve **Newline** satır sonuyla açın; `flash?` komutuyla durumu kontrol edin. Başarılı kayıt için **ACTIVE** beklenir.
- **ACTIVE ise `flash init` çalıştırmayın.** Firmware zaten aktifken bu komutu reddeder; ancak rutin güncellemede kullanmaya çalışmak gereksizdir.
- Geçerli A/B kayıt **hiç yoksa**, ayarları ve çipi dikkatle belirledikten ve yedek aldıktan sonra **yalnız USB seri üzerinden** `flash init` bir kez kullanılabilir. BLE paneli ilk flash başlatma işlemini desteklemez.
- Kayıt sonrası `p` / `flash?` ve panelin **Cihazdan oku / BLE Ayarları Oku** seçenekleriyle tekrar doğrulayın.
- İlk geçerli çağrı işaretinin kaydı AUTO'yu açabilir. Sonradan kasıtlı olarak seçilen OFF durumu yeniden başlatmada korunur.

**Ayar kaydı hatalarında** `SAVE FAILED`, `DIRECT INACTIVE`, `STALE SETTINGS` gibi mesajları inceleyin. Önce okuma-yedek ve uygun mod/kayıt durumunu düzeltin; rastgele flash silme işlemi yapmayın.

## 7. OLED, BLE ve RF ile ilgili güvenli kurtarma

| Belirti | Güvenli ilk kontrol |
|---|---|
| OLED görüntüsü kayboldu | Kartı kapatıp açarken **menü düğmesini 3 sn** basılı tutun; OLED sürücüsü değiştirilir |
| OLED kendi kendine kapanıyor | `OLED TIMER` seçimi; ekran uykusu APRS/GPS/LoRa'yı durdurmaz |
| BLE bulunamıyor | OLED'den **BLE ON/OFF**, USB `ble?` / `ble on`; Android Chrome/HTTPS ve başka BLE bağlantıları |
| USB panel COM portu açamıyor | Veri kablosu, Seri Monitör/ikinci sekme kapalı mı? |
| Ayar yazma kapalı | Önce cihazdan oku; `flash?` → DIRECT ACTIVE mi? |
| LoRa RX ERROR | Gerçek SX1262/SX1268 çipi, anten/RF anahtarı, yazılımdaki çip seçimi; değiştiyse yeniden başlat |
| TRACKER GPS yok | GPS besleme/anten, açık gökyüzü, `g` komutu |
| DIGI paket tekrarlamıyor | `mode?`, geçerli çağrı işareti, sabit koordinatlar, WIDE PATH 1/2, `digi?` sayaçları |
| `[TX] RadioLib TX OK` / `[DIGI TX OK]` ama aprs.fi yok | RF TX başarılı olabilir; bir iGate'in gerçekten duyup APRS-IS'e göndermesi gerekir |
| WX yok | BME280, GPS FIX, WX aralığı ve 10 sn RF korumasını kontrol edin |

## 8. Arduino IDE ile kaynak koddan derleme (ileri seviye)

**Hazır UF2 kullananların bu bölümü uygulaması gerekmez.**

- Kaynak: [`TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.ino`](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.ino). Arduino IDE bu isimde bir klasör oluşturmayı isteyebilir.
- **Adafruit nRF52 core 1.7.0** ve projeye uygun **özel MiniMesh / TA1FM board variant**, nice!nano ile uyumlu UF2 bootloader / Nordic **SoftDevice S140 6.1.1** konfigürasyonu.
- **RadioLib 7.8.1**, Adafruit GFX, Adafruit SH110X, Adafruit SSD1306, TinyGPSPlus, Adafruit BME280, Adafruit Unified Sensor; core kaynaklı Adafruit TinyUSB/Bluefruit ve InternalFileSystem/flash_nrf5x başlıkları. InternalFileSystem başlığı kullanılması, ayarların LittleFS'e kaydedildiği anlamına gelmez.
- Önce **Derle / Doğrula**. Bellek bölümleri, kart tanımı, pinler, RF anahtarı, TCXO ve bootloader düzeni doğru değilse **hazır çalışan karta yanlış derleme yüklemeyin**.
- Bu doküman yeni derlemenin ayrıca test edildiğini iddia etmez. GitHub'daki STABLE UF2, kullanıcı tarafından test edilmiş V0.9.24 BETA ile aynı binary dosyasıdır.

## 9. Temel bağlantılar / pin yönlendirmesi

Ayrıntılı donanım tablosu: **[PIN_BAGLANTILARI_TR.md](./PIN_BAGLANTILARI_TR.md)**. Aynı projenin MiniMesh pin eşlemesinden kısa özet:

| Hat | nRF52840 bağlantısı / açıklama |
|---|---|
| LoRa SPI | SCK P1.11, MISO P0.02, MOSI P1.15 |
| LoRa kontrol | CS P1.13, DIO1 P0.10, RESET P0.09, BUSY P0.29 |
| Kart RF anahtarı | RXEN P0.17; çıplak SX1262/SX1268'in genel pini olarak düşünmeyin |
| OLED + BME280 I²C | SDA P1.04, SCL P0.11 |
| GPS UART | GPS TX → MCU RX P0.22; GPS RX ← MCU TX P0.20; GPS_EN P0.24 |
| Kullanıcı düğmesi | P1.00 (Arduino 32) |
| Batarya ADC | P0.31 (Arduino 31), **uygun gerilim bölücü** ile |

**Dikkat:** Bunlar mevcut **kart varyantına özgü** bağlantılardır; başka nRF52840 tanımında Arduino numaraları değişebilir. 3,3 V GPIO sınırlarını ve batarya ADC ölçeklemesini doğrulayın. Antensiz TX yapmayın.

### DIY PCB seçeneği: fakeTec V4

[**fakeTec V4 PCB tasarımı**](https://github.com/gargomoma/fakeTec_pcb/issues/16), TA1FM V0.9.24'ün **GPIO haritası** ile (LoRa SPI/kontrol, OLED/BME280 I²C, GPS UART/EN, buton ve batarya ADC) eşleşir. Hazır bir **nRF52840 Pro Micro DIY PCB adayıdır**, ancak **V0.9.24 UF2'nin bu kartta çalıştığı fiziksel olarak henüz test edilmedi**. Kullanılacak LoRa modülünün SX1262/SX1268 çipi, TCXO/RF anahtarı, GPS için GPIO 24 MOSFET yapısı, 3,3 V besleme, bootloader/SoftDevice ve batarya R1/R2 bölücüsü ayrıca doğrulanmalıdır.

**Kaynak ve dosyalar:** [Orijinal fakeTec deposu](https://github.com/gargomoma/fakeTec_pcb) · [fakeTec V4 Issue #16](https://github.com/gargomoma/fakeTec_pcb/issues/16) · [V4 Gerber ZIP](https://github.com/gargomoma/fakeTec_pcb/blob/main/gerbers/lupusworax_fakeTec_pcb_v4_GERBER.zip) · [TA1FM pin eşleştirme ve güvenli montaj kontrol listesi](./PIN_BAGLANTILARI_TR.md#faketec-v4-pcb-uyumlulugu-ve-diy-kurulum).

## 10. Kurulum sonrası son kontrol

1. **Firmware:** OLED'de V0.9.24, USB'de sürüm satırı (bazı metinlerde BETA kalabilir).
2. **Bellek:** `flash?` ile Direct Flash ACTIVE.
3. **RF:** Doğru SX1262/SX1268, anten, `r` komutunda RX OK.
4. **İstasyon:** Yetkili çağrı işareti, doğru TRACKER veya DIGI modu.
5. **TRACKER:** GPS FIX, AUTO, SMART/FIXED, Comment/Status/WX ve semboller.
6. **DIGI:** Sabit LAT/LON, PATH 1/2, beacon aralığı ve başka istasyondan RF testi.
7. **Kalıcı ayar:** Kaydet → tekrar oku → gerektiğinde yeniden başlat → tekrar doğrula.
8. **Yedek:** Çalışan UF2 ve JSON ayar dosyasını ayrı sakla.

## 11. Belgeler ve doğrulama notu

- [Günlük kullanım ve özellikler (V0.9.24)](./KULLANIM_TR.md)
- [USB seri konsol komutlarının tamamı](./SERI_KONSOL_KOMUT_REHBERI_V0_9_24_TR.md)
- [DIGIPEATER WIDE yönlendirme ve kurulum](./DIGIPEATER_V0_9_24_TR.md)
- [Pin bağlantılarının tamamı](./PIN_BAGLANTILARI_TR.md)
- [V0.9.24 STABLE sürüm notları](./V0_9_24_STABLE_NOTLARI.md)
- [GitHub Release / UF2 + .ino indirme](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v0.9.24)

**Saha test durumu:** V0.9.24, yaklaşık 10 km / 30 dakikalık **SX1268 TRACKER** denemesinde kullanılmıştır. **SX1262 ve DIGI'nin güncel tekrar filtresi/toparlanması ayrıca sahada doğrulanmalıdır.** Belgeler bu ayrımı saklamaz.

**TA1FM nrf52 Tracker + Digipeater · V0.9.24 STABLE — 73 de TA1FM!**
