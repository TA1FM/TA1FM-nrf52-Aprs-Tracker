# TA1FM nRF52 LoRa APRS Tracker — Kullanım Kılavuzu (Türkçe)

**Firmware:** V0.9.5 STABLE  
**Donanım:** TA2DMX MiniMesh Lite NRF (nRF52840 / SX1262)  
**LoRa APRS:** 433.775 MHz · SF12 · BW125 kHz · CR 4/5  
**Web ayar paneli:** https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/

> Bu kılavuz GitHub'daki **V0.9.5 kaynak kodu** esas alınarak güncellenmiştir. V0.9.5'in saha testi proje sahibi tarafından başarılı olarak bildirilmiştir. `TA1FM-7` örnek çağrı işaretidir; kendi yetkili çağrı işaretinizi kullanın. OLED'deki sürüm numarası kısa biçimde **V0.9.5** gösterilir.

## 1. İlk kurulum ve yazılım yükleme

1. Çalışan eski UF2 dosyanızı ve mümkünse mevcut ayarların JSON yedeğini saklayın.
2. Donanımın **TA2DMX MiniMesh Lite NRF** ve uyumlu nRF52840 UF2 bootloader'ına sahip olduğunu doğrulayın.
3. Kart üzerindeki **RESET** düğmesine iki kez basın. Bilgisayarda UF2 sürücüsü açılınca `TA1FM_nrf52_Tracker_V0_9_5_SMART_BATT.uf2` dosyasını bu sürücüye kopyalayın.
4. Cihaz yeniden başladıktan sonra USB veri kablosunu bağlı tutun; masaüstü **Chrome veya Edge** ile [TA1FM Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/) sayfasını açın.
5. **USB Bağlan → Cihazdan Oku** adımlarını uygulayın. Çağrı işareti, AUTO, Smart Beacon, TX POWER, pil voltajı, Comment ve Status gibi ayarları kontrol edin.
6. Değişikliklerden sonra **Cihaza Yaz** deyin; gerçek kayıt mesajını ve **Cihazdan Oku** sonucunu kontrol edin. Gerekirse bir kez enerji kesip yeniden açarak kalıcılığını doğrulayın.
7. Anteni bağlı tutun, cihazı gökyüzünü görebilen bir yere yerleştirin ve `GPS:FIX` görünmesini bekleyin.

**Normal sürüm güncellemesinde Eraser kullanmayın veya kartı formatlamayın.** Geçerli çağrı işareti girilmeden (`NOCALL`) APRS TX engellenir. İlk kez geçerli çağrı işareti kaydedilmesi AUTO'yu açabilir; daha sonra bilinçli seçilmiş AUTO OFF tercihi yeniden başlatmada korunur.

## 2. Tek fiziksel tuşla kullanım

| İşlem | Ana ekran | Menü | Ayar / bilgi ekranı |
| --- | --- | --- | --- |
| Kısa bas | Manuel konum beacon isteği | Sonraki menü maddesi | Seçeneği/değeri değiştir; bilgi ekranında işlem yapma |
| Uzun bas (yaklaşık 1,2 sn) | Ana menüyü aç | Seçili maddeye gir | Kaydet/onayla veya bilgi sayfasından geri dön |

**Önemli:** OLED süre sonunda kapandıysa **ilk kısa basış yalnızca ekranı uyandırır**; yanlışlıkla manuel beacon göndermez. Ekranı uyandırdıktan sonra normal tuş işlevleri devam eder. Ekran kapalıyken uzun basışla menü de açılabilir.

- Menüde 30 saniye işlem yapılmazsa ana ekrana dönülür; ekran sürücüsü testi kendi onay süresini kullanır.
- Tuşla gönderim isteği, geçerli çağrı işareti, taze GPS konumu, hazır LoRa ve RF gönderim korumasına tabidir.
- **Menü tuşu** ile **RESET/UF2 tuşu** aynı işlev değildir.

## 3. OLED ana ekranı

Örnek görünüm (değerler temsilidir):

```text
TA1FM-7        18:42:15
---------------------
GPS:FIX       SAT:10
SPD:  32.5 km/h
LAT: 40.97500
LON: 27.50400
BAT:3.92V  RX:TA1FM-9
```

| Gösterge | Anlamı |
| --- | --- |
| Çağrı işareti | Cihaz kimliği (`NOCALL` ise konum TX engelli) |
| Saat | GPS zamanı, menüden UTC veya UTC+3 |
| `GPS:FIX` / `GPS:SEARCH` | Güncel GPS konumu hazır / aranıyor |
| `SAT` | GPS uydu sayısı |
| `SPD` | GPS hızı, km/sa |
| `LAT` / `LON` | Enlem ve boylam |
| `BAT` | Yaklaşık ölçülmüş pil gerilimi (kalibrasyon hassasiyeti sınırlıdır) |
| `RX` | Son duyulan LoRa APRS istasyonu |

## 4. OLED menüsü — V0.9.5

BME280 sensörü algılanırsa **11**, algılanmazsa **10** menü seçeneği vardır. BME280 bulunmuyorsa yalnızca `BME280 WX` maddesi gizlenir.

| Menü | Ne işe yarar? | Kullanım |
| --- | --- | --- |
| `BME280 WX` (varsa) | Sıcaklık, nem ve basınç bilgisi | Uzun bas → geri |
| `AUTO BEACON` | Otomatik APRS yayınını aç/kapat | Kısa bas → değiştir; uzun bas → kaydet |
| `POS SYMBOL` | Konum simgesi | Kısa bas → sıradaki; uzun bas → kaydet |
| `UTC OFFSET` | UTC / UTC+3 | Kısa bas → değiştir; uzun bas → kaydet |
| `DISPLAY` | OLED sürücüsü SH1106 / SSD1306 | Kısa bas → değiştir; uzun bas → görüntü testi |
| **`OLED TIMER`** | Ekranı otomatik kapatma | Kısa bas → süre; uzun bas → kaydet |
| **`TX POWER`** | LoRa çıkış gücü hedefi, 1–22 dBm | Kısa bas → +1 dBm; uzun bas → kaydet |
| `GPS INFO` | Uydu, FIX, GPS karakter sayısı | Uzun bas → geri |
| `LORA INFO` | LoRa/RX, RSSI-SNR ve AUTO durumu | Uzun bas → geri |
| `DEVICE INFO` | Cihaz bilgileri, **V0.9.5** | Uzun bas → geri |
| `EXIT` | Ana ekrana dön | Uzun bas → çık |

### 4.1 OLED TIMER — ekranın otomatik kapanması

| Menü seçimi | Davranış |
| --- | --- |
| `ALWAYS ON` | Ekran sürekli açık |
| `ALWAYS OFF` | Ekran normal kullanımda kapalı tutulur; tuşla geçici uyandırılabilir |
| `15 SEC` | Ana ekranda 15 saniye sonra kapat |
| `30 SEC` | 30 saniye sonra kapat (**varsayılan**) |
| `1 MIN` | 1 dakika sonra kapat |
| `2 MIN` | 2 dakika sonra kapat |
| `5 MIN` | 5 dakika sonra kapat |

Ana ekrandayken süre dolduğunda yalnızca OLED kapatılır. **GPS, LoRa, APRS, otomatik beacon ve diğer işlemler sürer.** Tuş ekranı yeniden uyandırır. Seçilen değer LittleFS'de kalıcıdır. `ALWAYS OFF` modunda kod, ekranı uyandıktan yaklaşık 15 saniye sonra yeniden kapatır; bu mod cihazı bütünüyle uyutmaz.

### 4.2 TX POWER — ayarlanabilir çıkış gücü

Ana ekranda uzun bas → menüde `TX POWER` → uzun bas. Kısa basışlar gücü **1 dBm adımlarla 1–22 dBm** arasında değiştirir; 22'den sonra 1'e döner. Uzun basış seçimi uygular ve kaydeder.

| Ayar | İdeal çip gücü (yaklaşık) |
| --- | --- |
| 10 dBm | 10 mW |
| 14 dBm | 25 mW |
| 17 dBm | 50 mW |
| 20 dBm | 100 mW |
| 22 dBm | 158 mW |

**Dikkat:** Bunlar dBm–mW dönüşümüdür, anten soketinde ölçülmüş güç değildir. Kartın RF katı, besleme koşulları ve yasal güç sınırları ayrıca değerlendirilmelidir. En yüksek ayar için uzun süreli kullanım öncesi kart uygunluğunu doğrulayın. Değer Web Configurator'den veya `power 1..22` komutuyla da değiştirilebilir; seçilen ayar LittleFS'de saklanır.

### 4.3 DISPLAY — OLED tipi ve kurtarma

1. `DISPLAY` menüsüne girin, kısa basarak `SH1106` veya `SSD1306` seçin.
2. Uzun basarak **10 saniyelik görüntü testini** başlatın.
3. Görüntü doğruysa süre dolmadan yeniden **uzun basarak onaylayın**.
4. Onay verilmezse önceki sürücü seçimi geri gelir.

**Ekran görünmüyorsa:** Cihazı açarken **menü tuşunu** basılı tutun ve ilk **3 saniye** boyunca bırakmayın. Yazılım kayıtlı ekran sürücüsü tercihini diğer tipe geçirip saklamak üzere kurtarma davranışı uygular. Bu, UF2 için çift RESET işleminden farklıdır.

### 4.4 Diğer OLED ayarları

- **AUTO BEACON:** `ON` otomatik TX'i etkinleştirir; `OFF` kapatır. Diğer yayına uygunluk koşulları yine geçerlidir.
- **POS SYMBOL:** `CAR />`, `WALKER /[`, `HOUSE /-` seçenekleri mevcuttur. Park algılandığında firmware park simgesi kullanabilir.
- **UTC OFFSET:** UTC veya UTC+3 seçimi kaydedilir.

## 5. Smart Beacon — V0.9.5 hız tablosu

**Yeni Smart Beacon, şehir içinde daha sık konum güncellemesi için tasarlanmıştır.** SMART modunda hız noktaları sabittir; karşılık gelen altı beacon süresi Web Configurator'den değiştirilebilir.

| GPS hızı | Varsayılan konum beacon aralığı |
| --- | --- |
| 0–7 km/sa | **10 dakika (600 sn)** |
| 10 km/sa | **90 sn** |
| 20 km/sa | **45 sn** |
| 49 km/sa | **30 sn** |
| 60 km/sa | **20 sn** |
| 90 km/sa ve üzeri | **15 sn** |

- **7–10, 10–20, 20–49, 49–60 ve 60–90 km/sa** aralarında yazılım doğrusal ara değer hesaplar. Örneğin 35 km/sa hızda hedef yaklaşık **37 saniyedir**.
- **İlk hareket beacon'ı (varsayılan AÇIK):** GPS hızı **8 km/sa ve üzerinde art arda 3 örnekle** hareketi doğrularsa ve cihaz daha önce ilk konumunu göndermişse SMART modda yeni konum gönderimini isteyebilir. GPS FIX ve RF bekleme kuralları uygulanır.
- **Durma/park:** 5 km/sa ve altı durma adayıdır; varsayılan park onay süresi 3 dakikadır. 5–8 km/sa bandında hareket durumuna ilişkin histerezis uygulanır.
- **Viraj tetiklemesi:** Hareket doğrulandıysa, hız **en az 10 km/sa** ve son gönderilen yönle mevcut yön arasında **en az 35°** fark varsa konum isteği oluşabilir.
- **RF koruması:** Konum paketlerinde önceki RF yayınına göre en az **10 sn**, WX/Status gibi yardımcı yayınlarda **30 sn** ortak bekleme uygulanır. Dolayısıyla tabloda yazan süreler kesin yayın veya alım garantisi değildir.
- **FIXED modu:** Hızdan bağımsız 1, 3, 5, 10 veya 15 dakikalık konum periyodu seçilebilir. Bu tablonun değişiklikleri FIXED periyodunu değiştirmez.

**SMART varsayılanlarını değiştirme:** Web panelinde altı süreyi **saniye** olarak girin. Yazılım ilk noktada **15–3600 sn**, diğerlerinde **10–3600 sn** değerlerine izin verir; hız arttıkça süre artmamalıdır. Böylece örneğin `600,90,45,30,20,15` kabul edilir. Başarılı kayıt LittleFS'de saklanır.

### Saha testinde konumlar seyrek görünürse

1. Web panelinde **AUTO=ON**, **POS=SMART** ve doğru sürelerin yüklü olduğunu doğrulayın.
2. `GPS:FIX` durumunu ve GPS hızının güncel olup olmadığını kontrol edin.
3. Seri konsolda `p` veya `smart?` ile hız tablosu, AUTO, GPS ve hareket bilgisini inceleyin.
4. Konumun gerçekten **TX** edilmesi ile bir iGate tarafından **alınması / internete aktarılması** farklıdır; kapsama ve RF çakışmaları haritada boşluk oluşturabilir.

## 6. APRS konum paketinde pil voltajı

V0.9.5 ile konum beacon'ının **comment metnine pil voltajı isteğe bağlı eklenebilir**. Varsayılan **AÇIK**; Web Configurator'den kapatılabilir.

Örnek paket yorum parçası:

```text
TA1FM nrf52 Tracker Batt=3.92V
```

- `Batt=...V` bilgisi normal Comment gönderim aralığından **bağımsızdır**; uygun geçerli pil ölçümü varsa normal **konum beacon'ına** eklenir.
- **Ek pil yayını oluşturulmaz.** Bu nedenle yalnızca pil bilgisini açmak ayrı bir RF paket trafiği oluşturmaz.
- Ölçüm yazılımın ADC kalibrasyonuna dayanan yaklaşık değerdir; hassas ölçüm için multimetre ile karşılaştırın.
- Bu, APRS **yorum alanında görünen metindir**; standart yapılandırılmış APRS telemetri kanalı değildir.
- Yazılım pil bilgisini yalnızca **2.0–5.5 V aralığında geçerli bir ölçüm** varsa ekler.

## 7. Hava durumu (WX), Status ve Comment

- **WX:** BME280 algılanırsa sıcaklık, nem ve basınç APRS hava durumu formatında gönderilebilir; rüzgâr/yağmur sensörü olmayan alanlara ölçülmüş gibi değer uydurulmaz. WX zamanlayıcısı bağımsızdır; varsayılanı 60 dakika olabilir, kayıtlı değer önceliklidir.
- **Status:** Bağımsız APRS durum paketi; zamanlayıcısı ayrı yönetilir.
- **Comment:** Bağımsız yayın değildir; belirlenen aralıkta konum beacon'ının sonuna eklenen metindir. **Pil voltajı seçeneği Comment zamanlamasından ayrı çalışır.**
- **Park hâli:** Konum simgesini etkileyebilir; WX gönderimini tek başına engellemez.
- Yardımcı TX istekleri, ortak **30 saniyelik** RF bekleme koşulundan etkilenir.

## 8. Web Configurator ve kalıcı ayarlar

**[TA1FM Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)**

1. Masaüstü Chrome/Edge'de paneli açın; USB üzerinden cihazın COM portuna bağlanın.
2. **Cihazdan Oku** ile gerçek ayarları alın. Sadece ekranda bir varsayılanın görünmesi cihaza kayıtlı olduğu anlamına gelmez.
3. AUTO ve POS modunu, **Smart Beacon sürelerini (6 değer)**, **ilk hareket beacon'ını ON/OFF**, **pil voltajı gönderimini ON/OFF**, **TX POWER (1–22 dBm)**, diğer beacon ve metin ayarlarını düzenleyin.
4. **Cihaza Yaz** deyin ve seri yanıtında başarılı kayıt sonucunu doğrulayın.
5. Tekrar **Cihazdan Oku** uygulayın. Gerekirse JSON yedek alın ve cihazı yeniden başlatarak değerlerin korunduğunu kontrol edin.

USB Seri Monitör ve Web Configurator **aynı COM portuna aynı anda bağlanamaz**. Portu kullanan diğer programı kapatın. Firmware yeni Smart Beacon ayarlarını `/ta1fm_smart_v095.cfg`, TX gücünü `/ta1fm_tx_power_v094.cfg`, OLED zamanlayıcısını `/ta1fm_oled_sleep.cfg` dosyasında saklar; daha eski OLED/UTC ve çağrı işareti kayıtları ayrı tutulur.

## 9. Seri konsol komutları (ileri seviye)

**Baud:** 115200. Komutları satır sonuyla gönderin. Normal kullanım için web paneli önerilir.

| Komut | İşlev |
| --- | --- |
| `p` veya `?` | Genel durum, TX gücü, GPS, Smart Beacon ve AUTO bilgileri |
| `c TA1FM-7` | Örnek çağrı işaretini kaydet |
| `a on` / `a off` | AUTO Beacon açık / kapalı |
| `f smart` / `f fixed` | Konum modunu seç |
| `i 1`, `i 3`, `i 5`, `i 10`, `i 15` | FIXED konum aralığı, dakika |
| **`smart 600,90,45,30,20,15`** | 7/10/20/49/60/90 km/sa için altı süreyi saniye olarak kaydet |
| **`smart?`** | Altı süreyi, ilk hareket ve pil gönderimi durumunu oku |
| **`motion on` / `motion off`** | İlk hareket konum beacon'ı açık / kapalı |
| **`batt on` / `batt off`** | Konum beacon'ına pil voltajı ekle / kaldır |
| **`power 17`** | TX güç hedefini 17 dBm yap (1–22 aralığı) |
| **`power?`** | TX güç ayarını oku |
| `k 1`, `k 3`, `k 5` | Park onayı için dakika |
| `s car`, `s walker`, `s house` | APRS simgesini seç |
| `x off`, `x 15`, `x 30`, `x 60` | WX aralığı |
| `z off`, `z 15`, `z 30`, `z 60` | Status aralığı |
| `j off`, `j 15`, `j 30`, `j 60` | Comment ekleme aralığı |
| `comment METIN` / `status METIN` | Yorum / durum metni kaydet |
| `comment?` / `status?` | Metinleri oku |
| `t` / `w` / `v` | Manuel konum / WX / Status isteği |

**Metin sınırı:** Comment/Status için yazılımda ASCII, 1–62 karakter; `|` ve `~` karakterlerine izin verilmez. TX için GPS/LoRa uygunluğu ve bekleme koruması her zaman geçerlidir.

## 10. Sorun giderme

| Belirti | İlk kontrol |
| --- | --- |
| `GPS:SEARCH` | GPS anten konumu, açık gökyüzü, enerji beslemesi ve GPS veri akışı |
| Konum haritada seyrek | AUTO=ON, POS=SMART, `smart?`, GPS hız/fix; iGate kapsaması |
| İlk hareket beacon'ı yok | `motion on`, hareketin 8 km/sa üzerinde doğrulanması, ilk konum TX, GPS ve RF bekleme |
| Beacon'da pil yok | `batt on`, güncel pil ölçümü, APRS konum paketi alımı |
| TX POWER görünmüyor | Cihazdaki sürümün gerçekten **V0.9.5** olduğuna bakın; BME280 varsa 11, yoksa 10 madde |
| TX gücü değişmiyor | `power?` ve konsol hata mesajlarını kontrol edin; cihaz/radyo hazır mı? |
| Ekran kendi kendine kapanıyor | `OLED TIMER` süresini kontrol edin; tuşla uyandırın |
| Ekran tamamen görünmüyor | SH1106/SSD1306 seçimi; 3 saniyelik menü tuşu OLED kurtarma işlemi |
| BME280 WX menüsü görünmüyor | Sensör algılanmamış olabilir; I2C bağlantısı/adresleri |
| Ayarlar saklanmıyor | Seri portta `saved` / `SAVE FAILED`, LittleFS durumunu ve yeniden okuma sonucunu inceleyin |
| Web paneli porta bağlanamıyor | Veri USB kablosu, Chrome/Edge, COM portunu kullanan başka uygulama |
| Konum gönderimi yok | Çağrı işareti `NOCALL` mı? GPS FIX güncel mi, LoRa hazır mı, AUTO durumu ne? |

## 11. Donanım uyumu ve güvenlik

Bu firmware **her nRF52840 kartına uygun değildir**. Proje donanımı TA2DMX MiniMesh Lite NRF, SX1262, L76K GPS ve SH1106/SSD1306 OLED seçenekleri için hazırlanmıştır. Projede kullanılan kart varyantı ve bootloader/SoftDevice eşleşmesini kontrol edin. Radyo frekansı ve çıkış gücünü bulunduğunuz bölgenin amatör telsiz mevzuatına ve cihazın güvenli RF sınırlarına uygun seçin.

## 12. Faydalı bağlantılar

- [TA1FM nrf52 Tracker GitHub — kaynak kodu ve UF2](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker)
- [V0.9.5 kaynak kodu](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/TA1FM_nrf52_Tracker_V0_9_5_SMART_BATT.ino)
- [V0.9.5 UF2 firmware](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/TA1FM_nrf52_Tracker_V0_9_5_SMART_BATT.uf2)
- [TA1FM Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/)

---

**TA1FM nrf52 Tracker · V0.9.5 STABLE — 73 de TA1FM!**
