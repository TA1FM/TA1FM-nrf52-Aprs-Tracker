# TA1FM nrf52 Tracker — SX1268 V0.9.14 (Direct Flash + OLED Web)

## Donanım
- nRF52840 MiniMesh Lite NRF
- Ra-01S / SX1268 LoRa
- SH1106 veya SSD1306 OLED

## Doğrulanan özellikler
- GPS konum tespiti, APRS LoRa TX/RX ve otomatik gönderim
- Smart Beacon ve 15 dakikalık Comment gönderimi
- Web Configurator üzerinden ayar kaydetme
- OLED sürücü tipi seçimi (cihaz butonu ve güncel web paneli)
- Direct Flash A/B kalıcı kayıt; enerji kesilip verilince ayarların korunması

## Kritik notlar
- Eski V0.9.7 SX1268 TEST sürümünde LittleFS kayıt sorunu vardı; SX1268 için V0.9.14 kullanılmalıdır.
- SX1262 için çalışan V0.9.5 SMART_BATT sürümü ayrıdır ve değiştirilmemelidir.
- Mevcut ayarlar kayıtlıyken `flash init` komutunu kullanmayın.
- Web paneli ayrı depodadır: https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker-Configurator
- Test edilmiş cihazda yeni sürüm arşiv amaçlıdır; flash bellek adreslerinin farklı bootloader veya kartlarda çakışmadığı ayrıca doğrulanmalıdır.

## Dosyalar
- `.ino`: Arduino kaynak kodu
- `.uf2`: nRF52840 NICE!NANO UF2 bootloader ile yükleme dosyası
