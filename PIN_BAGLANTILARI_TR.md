# TA1FM nrf52 Tracker — Pin Bağlantıları (SX1262 / SX1268)

Bu belge, **TA2DMX MiniMesh Lite NRF / nRF52840** için hazırlanmış TA1FM Tracker yazılımlarının **kullanılan Arduino kart tanımı** ile pin eşlemesini anlatır. Ayrı bir nRF52840 modülüyle kendi devresini kuracak kişiler, nRF52840 üzerindeki **P0.xx/P1.xx** pin adlarını esas almalıdır. Parantez içindeki Arduino numaraları farklı kart çekirdeklerinde değişebilir.

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

**Önemli:** `RXEN` nRF52840 kartındaki RF anahtarına bağlı olabilir; çıplak SX1262/SX1268 modülünde bu adla bir pin bulunmayabilir. RF anahtarı, TCXO gereksinimi ve anten devresi modüle göre ayrıca doğrulanmalıdır. **SX1262 için V0.9.6, SX1268 (Ra-01S) için V0.9.14 firmware kullanın.**

## 2. OLED ekran: SH1106 / SSD1306 (I²C)

| OLED pini | nRF52840 pini | Arduino pin no. |
|---|---|---:|
| SDA | P1.04 | 36 |
| SCL | P0.11 | 11 |
| VCC | Ekranın desteklediği 3,3 V besleme | — |
| GND | GND | — |

- Yazılımın OLED çözünürlüğü **128 × 64**, I²C adresi **0x3C**.
- SH1106 / SSD1306 yazılım ya da cihaz butonundan seçilebilir.
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
3. SX1262 ve SX1268 için farklı firmware gerekir; LLCC68, bu projede kullanılan 433.775 MHz / SF12 / BW125 APRS moduyla uyumlu değildir.
4. Tablo, çalışan MiniMesh yazılımının pin eşlemesidir; sıfırdan kart yaparken **güç devresi, batarya şarj devresi, RF anahtarı, anten, TCXO ve bootloader/board variant** ayrıca tasarlanmalı ve doğrulanmalıdır.
5. İlk testte radyo TX'ten önce I²C cihazlarını, GPS UART'ını ve güç hatlarını ayrı ayrı doğrulayın.

## Kaynak

- [TA1FM Tracker GitHub](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker)
- [SX1262 V0.9.6 firmware kaynak kodu](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/TA1FM_nrf52_Tracker_V0_9_6_OLED_WEB.ino)
- [Meshtastic nrf52_promicro_diy_tcxo kart tanımı](https://github.com/meshtastic/firmware/blob/develop/variants/nrf52840/diy/nrf52_promicro_diy_tcxo/variant.h)

**73 de TA1FM**

