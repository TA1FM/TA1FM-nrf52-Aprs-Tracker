# TA1FM nrf52 Tracker + Digipeater — Pin Bağlantıları (V0.9.24 STABLE)

Bu sayfa, **TA2DMX MiniMesh Lite NRF** için kullanılan firmware pinlerini ve **nRF52840 Pro Micro ile kendin yap (DIY)** görselini bir arada sunar. Pro Micro'nun kart revizyonu, silkscreen pinleri, RF/TCXO donanımı ve Arduino kart varyantı **kurulumdan önce ayrıca doğrulanmalıdır**. Pro Micro'nun aynı işlemciyi taşıması, hazır MiniMesh UF2'nin her karta doğrudan uygun olduğu anlamına gelmez.

## 🖼️ nRF52840 Pro Micro — Bağlantı Şeması

Aşağıdaki görsel, ilk hazırlanan şemanın **kullanıcı tarafından düzeltilip yeniden yüklenmiş** hâlidir.

[![TA1FM nRF52 Tracker Pro Micro bağlantı şeması](images/TA1FM%20nRF52%20Tracker%20Ba%C4%9Flant%C4%B1%20%C5%9Eemas%C4%B1.png)](images/TA1FM%20nRF52%20Tracker%20Ba%C4%9Flant%C4%B1%20%C5%9Eemas%C4%B1.png)

[**Görseli tam boy aç**](images/TA1FM%20nRF52%20Tracker%20Ba%C4%9Flant%C4%B1%20%C5%9Eemas%C4%B1.png)

> **Kablolama öncesi:** Bu görsel, fiziksel kart üzerinde bağımsız olarak test edilmiş bir PCB bağlantı şeması değildir. V0.9.24 firmware'in pin tablosunu esas alın; özellikle OLED/BME280 I²C hatlarını **SDA=P1.04, SCL=P0.11** olarak karşılaştırın. RF anahtarı/TCXO ve güç devresi kullanılan LoRa modülüne göre doğrulanmalıdır.

## fakeTec V4 PCB uyumluluğu ve DIY kurulum

**Sonuç: GPIO/pin haritası düzeyinde örtüşme var.** [fakeTec PCB açık kaynak donanım projesi](https://github.com/gargomoma/fakeTec_pcb) içindeki [fakeTec V4 tasarımı (Issue #16)](https://github.com/gargomoma/fakeTec_pcb/issues/16), nRF52840 **Pro Micro / nice!nano tarzı** işlemci kartı ve LoRa modülü için kullanılan pin düzenini gösterir. V0.9.24 STABLE firmware'inin pinleri bu düzenle karşılaştırılmıştır. Böylece **fakeTec V4, TA1FM için uygun olabilecek DIY PCB adaylarından biri** olarak değerlendirilebilir.

| İşlev | TA1FM V0.9.24 nRF52840 GPIO'ları | fakeTec V4 karşılaştırması |
|---|---|---|
| LoRa SPI | SCK **P1.11**, MOSI **P1.15**, MISO **P0.02**, CS **P1.13** | Eşleşiyor |
| LoRa kontrol | DIO1 **P0.10**, BUSY **P0.29**, RESET **P0.09** | Eşleşiyor |
| RF anahtarı | RXEN **P0.17** | GPIO eşleşiyor; fiziksel RF yolu/modül kontrol edilmeli |
| OLED / BME280 I²C | SDA **P1.04**, SCL **P0.11** | Eşleşiyor |
| GPS UART | MCU RX **P0.22** (GPS TX), MCU TX **P0.20** (GPS RX) | Eşleşiyor |
| GPS açma | GPS_EN **P0.24** (firmware HIGH) | V4'te GPIO 24 ile sürülen MOSFET var; bağlantı/topoloji doğrulanmalı |
| Kullanıcı butonu | **P1.00** (Arduino 32) | Eşleşiyor |
| Batarya ADC | **P0.31** (Arduino 31) | ADC pini aynı; **ölçüm katsayısı ayrıca ayarlanabilir** |

### fakeTec V4 hazırlık kontrol listesi

1. **PCB revizyonu:** `fakeTec V4` ile daha eski/yeni V3, V5 vb. sürümleri karıştırmayın. [Asıl V4 issue'sunu](https://github.com/gargomoma/fakeTec_pcb/issues/16) ve [resmî Gerber arşivini](https://github.com/gargomoma/fakeTec_pcb/tree/main/gerbers) karşılaştırın. **[V4 Gerber ZIP](https://github.com/gargomoma/fakeTec_pcb/blob/main/gerbers/lupusworax_fakeTec_pcb_v4_GERBER.zip)** depoda ayrıca bulunur; üretim öncesinde son düzeltmeleri kontrol edin.
2. **Radyo modülü ve RF:** Kullanılan modülün gerçekten **SX1262 veya SX1268** olduğunu doğrulayın. TCXO'lu ve kristalli (XTAL) modüller aynı elektriksel ihtiyaçlara sahip değildir. fakeTec README'si **TCXO / XTAL için ayrı kart varyantlarından** söz eder. TA1FM yazılımında **DIO2 RF switch** ve **P0.17 RXEN kontrolü** vardır; modülün RF switch/anten devresine karşılık geldiğini ayrıca ölçün.
3. **GPS güç kontrolü:** V4 issue'su **GPIO 24, 6 ve 8'i** harici cihazlar için MOSFET kontrol hatları olarak açıklar. TA1FM `GPS_PWR_PIN=P0.24` çıkışını **HIGH** yapar. V4'te MOSFET'in **eksi hattı anahtarladığı** belirtiliyor; GPS bağlantısını ve enable davranışını fiziksel devreye göre doğrulayın. `P0.24` doğrudan GPS beslemesi değildir.
4. **Batarya ölçümü:** fakeTec'te **R1/R2 gerilim bölücü** bulunur. V0.9.24 firmware'inde **`P0.31` analog okuması `1.73` çarpanıyla** voltaja dönüştürülür (MiniMesh'te test edilen yaklaşık kalibrasyon). fakeTec'in direnç oranına göre ölçüm **yanlış görünebilir**; multimetreyle karşılaştırın, gerekirse firmware kalibrasyonunu ayarlayın. Pilin pozitif/negatif uçlarını ADC'ye doğrudan bağlamayın.
5. **Besleme ve bootloader:** Pro Micro üzerindeki **B+/B− batarya**, **VCC 3,3 V**, **RAW**, **GND** uçlarını gerçek kart şemasına göre bağlayın. USB UF2 sürücüsünden **Board-ID / SoftDevice** uyumluluğunu kontrol edin; gerekmedikçe bootloader'a dokunmayın. [TA1FM Bootloader arşivi](./BOOTLOADER/README_TR.md) yalnızca uygun kartlarda güncelleme/kurtarma referansıdır.
6. **İlk test:** Önce USB/OLED/I²C/GPS ve batarya voltajını; sonra uygun LoRa anteni bağlıyken RX/TX fonksiyonunu sınayın. **V0.9.24 STABLE UF2'nin fakeTec V4 PCB üzerinde fiziksel çalışması henüz doğrulanmış değildir.**

**Kaynaklar ve atıf:** [gargomoma/fakeTec_pcb](https://github.com/gargomoma/fakeTec_pcb) · [lupusworax'ın fakeTec V4 tasarımı ve açıklamaları](https://github.com/gargomoma/fakeTec_pcb/issues/16) · [V4 Gerber arşivi](https://github.com/gargomoma/fakeTec_pcb/blob/main/gerbers/lupusworax_fakeTec_pcb_v4_GERBER.zip). Issue'da PCB çizimine **Severin**'in de katkısı belirtilir. TA1FM bu üçüncü taraf PCB tasarımının sahibi değildir; donanımı çoğaltmadan önce orijinal depodaki koşulları ve üretim dosyalarının güncelliğini kontrol edin.

## 1. LoRa modülü: SX1262 veya SX1268 (SPI)

| LoRa modülü pini | nRF52840 pini | Arduino pin no. |
|---|---|---:|
| SCK | P1.11 | 43 |
| MISO | P0.02 | 2 |
| MOSI | P1.15 | 47 |
| NSS / CS | P1.13 | 45 |
| DIO1 | P0.10 | 10 |
| NRST / RESET | P0.09 | 9 |
| BUSY | P0.29 | 29 |
| RXEN (kart RF anahtarı) | P0.17 | 17 |
| GND | GND | — |
| VCC | Uygun regüle 3,3 V | — |

**Önemli:** `RXEN` nRF52840 kartındaki RF anahtarına bağlı olabilir; çıplak SX1262/SX1268 modülünde bu adla bir pin bulunmayabilir. RF anahtarı, TCXO gereksinimi ve anten devresi modüle göre ayrıca doğrulanmalıdır. **Güncel V0.9.24:** SX1262/SX1268 aynı Universal firmware içinde seçilebilir. Önceki V0.9.6 ve V0.9.14 paketleri tarihsel ayrı sürümlerdir. Yanlış çip seçimi RF'yi devre dışı bırakabilir; çip değişimi sonrası yeniden başlatın.

## 2. OLED ekran: SH1106 / SSD1306 (I²C)

| OLED pini | nRF52840 pini | Arduino pin no. |
|---|---|---:|
| SDA | P1.04 | 36 |
| SCL | P0.11 | 11 |
| VCC | Ekranın desteklediği 3,3 V besleme | — |
| GND | GND | — |

- Yazılımın OLED çözünürlüğü **128 × 64**, I²C adresi **0x3C**.
- SH1106 / SSD1306 USB veya BLE web panelinden seçilebilir. BLE'de sürücü **doğrudan kalıcı kaydedilir**; hatalı seçimde açılışta **menü tuşunu 3 saniye basılı tutarak** kurtarma korunur.
- OLED ile BME280 aynı SDA/SCL hattını paylaşabilir.

## 3. GPS (UART)

| GPS bağlantısı | nRF52840 pini | Açıklama |
|---|---|---|
| GPS TX → MCU RX | P0.22 | GPS'ten gelen NMEA verisi |
| GPS RX ← MCU TX | P0.20 | MCU'nun GPS'e veri göndermesi |
| GPS_EN (güç kontrol) | P0.24 | Mevcut kartta HIGH etkin |
| VCC | GPS modülüne uygun regüle besleme | Modül özelliklerini kontrol edin |
| GND | GND | Ortak toprak |

- Firmware `Serial1` ve **9600 baud** kullanır.
- GPS TX/RX yönlerine dikkat edin: TX, karşı tarafın RX pinine gider.
- `GPS_EN` pininin **doğrudan modül beslemesi değil, bir güç anahtarı ya da enable hattı** olabileceğini dikkate alın; çıplak GPS modülüne bu pinden güç vermeyin.
- P0.20/P0.22 eşlemesi, MiniMesh tasarımının temel aldığı `nrf52_promicro_diy_tcxo` kart tanımına dayanır. Farklı kart tanımında mutlaka yeniden doğrulayın.

## 4. BME280 (isteğe bağlı, I²C)

| BME280 pini | nRF52840 pini |
|---|---|
| SDA | P1.04 |
| SCL | P0.11 |
| VCC | Modüle uygun 3,3 V besleme |
| GND | GND |

- Desteklenen I²C adresleri **0x76** ve **0x77**.
- BME280 yoksa yazılım hava durumu menüsünü gizler.
- I²C bağlantısı için gerekirse SDA/SCL pull-up dirençlerini ve modülde mevcut olup olmadıklarını kontrol edin.

## 5. Buton ve batarya ölçümü

| İşlev | nRF52840 pini | Arduino pin no. |
|---|---|---:|
| Kullanıcı butonu | P1.00 | 32 |
| Batarya voltaj ADC | P0.31 | 31 |

- Batarya gerilimini ADC pinine **doğrudan bağlamayın**. nRF52840 ADC giriş sınırlarına uygun bir **gerilim bölücü** kullanılmalıdır. Bölücü oranı ve yazılımdaki ölçek, kullanılan kartın gerçek devresine göre doğrulanmalıdır.
- Butonun pull-up/pull-down bağlantısını ilgili kart şemasına ve firmware koduna göre uygulayın.

## Güvenlik ve uyumluluk notları

1. nRF52840 GPIO pinleri 3,3 V mantık düzeyindedir; 5 V sinyal uygulamayın.
2. LoRa modülüne anten bağlamadan RF yayın testi yapmayın.
3. **V0.9.24 STABLE Universal firmware** SX1262 ve SX1268 seçimini destekler; LLCC68 bu projenin 433.775 MHz / SF12 / BW125 APRS moduyla uyumlu değildir.
4. Tablo, çalışan MiniMesh yazılımının pin eşlemesidir; sıfırdan kart yaparken **güç devresi, batarya şarj devresi, RF anahtarı, anten, TCXO ve bootloader/board variant** ayrıca tasarlanmalı ve doğrulanmalıdır.
5. İlk testte radyo TX'ten önce I²C cihazlarını, GPS UART'ını ve güç hatlarını ayrı ayrı doğrulayın.

## Kaynak

- [TA1FM Tracker GitHub](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker)
- [V0.9.24 STABLE Universal BLE kaynak kodu](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.ino)
- [V0.9.24 STABLE UF2 firmware](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.uf2)
- [Türkçe kullanım kılavuzu](./KULLANIM_TR.md)
- [Meshtastic nrf52_promicro_diy_tcxo kart tanımı](https://github.com/meshtastic/firmware/blob/develop/variants/nrf52840/diy/nrf52_promicro_diy_tcxo/variant.h)

**73 de TA1FM**

