# TA1FM nrf52 Tracker + Digipeater — Pin Bağlantıları (V0.9.24 STABLE)

Bu sayfa, **TA2DMX MiniMesh Lite NRF** için kullanılan firmware pinlerini ve **nRF52840 Pro Micro ile kendin yap (DIY)** görselini bir arada sunar. Pro Micro'nun kart revizyonu, silkscreen pinleri, RF/TCXO donanımı ve Arduino kart varyantı **kurulumdan önce ayrıca doğrulanmalıdır**. Pro Micro'nun aynı işlemciyi taşıması, hazır MiniMesh UF2'nin her karta doğrudan uygun olduğu anlamına gelmez.

## 🖼️ nRF52840 Pro Micro — Bağlantı Şeması

Aşağıdaki görsel, ilk hazırlanan şemanın **kullanıcı tarafından düzeltilip yeniden yüklenmiş** hâlidir.

[![TA1FM nRF52 Tracker Pro Micro bağlantı şeması](images/TA1FM%20nRF52%20Tracker%20Ba%C4%9Flant%C4%B1%20%C5%9Eemas%C4%B1.png)](images/TA1FM%20nRF52%20Tracker%20Ba%C4%9Flant%C4%B1%20%C5%9Eemas%C4%B1.png)

[**Görseli tam boy aç**](images/TA1FM%20nRF52%20Tracker%20Ba%C4%9Flant%C4%B1%20%C5%9Eemas%C4%B1.png)

> **Kablolama öncesi:** Bu görsel, fiziksel kart üzerinde bağımsız olarak test edilmiş bir PCB bağlantı şeması değildir. V0.9.24 firmware'in pin tablosunu esas alın; özellikle OLED/BME280 I²C hatlarını **SDA=P1.04, SCL=P0.11** olarak karşılaştırın. RF anahtarı/TCXO ve güç devresi kullanılan LoRa modülüne göre doğrulanmalıdır.

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

