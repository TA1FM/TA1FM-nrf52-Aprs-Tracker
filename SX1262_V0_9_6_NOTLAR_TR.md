# TA1FM nrf52 Tracker — SX1262 V0.9.6 OLED WEB

**Donanım:** TA2DMX MiniMesh Lite NRF (nRF52840) ve **SX1262** LoRa çipi.

**Bu firmware SX1262 içindir.** SX1268 / Ra-01S için ayrı V0.9.14 Direct Flash firmware kullanılmalıdır. LLCC68, bu projenin 433.775 MHz SF12 / BW125 APRS yapılandırmasıyla uyumlu değildir.

## V0.9.6 OLED WEB

- USB seri arayüzü üzerinden OLED sürücüsü: SH1106 / SSD1306
- 20 saniyelik OLED önizleme, onay ve iptal desteği
- OLED ekran kapanma süresi ve geçici uyandırma komutları
- GPS, APRS ve Smart Beacon özellikleri
- SX1262 sürümünde kalıcı ayarlar için LittleFS

## Saha durumu

TA1FM tarafından SX1262 cihazda kullanıldığı ve enerji kesilip verildiğinde ayarların korunduğu bildirilmiştir. Kaynak kodu ve UF2 dosyası bu arşive eklenmiştir.

**Önemli:** V0.9.5 sürümünü geçmiş sürüm olarak saklayın. SX1268 cihazına bu UF2 dosyasını yüklemeyin.

73 de TA1FM
