# TA1FM nrf52 Tracker + Digipeater — Kullanım Rehberi (V1.1.0 STABLE)

**[Firmware Release](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.1.0)** · **[V1.1.0 Web Paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html)** · [Kurulum](./KURULUM_TR.md) · [Seri Konsol](./SERI_KONSOL_KOMUT_REHBERI_TR.md)

## 1. Genel bakış

MiniMesh nRF52840 üzerinde iki çalışma modu vardır:

| Mod | İşlev |
|---|---|
| **TRACKER** | GPS'ten konum, SMART/FIXED beacon, hareket/park sembolleri, konum + Comment |
| **DIGIPEATER** | Sabit koordinat, WIDE paket tekrarı ve belirlenen aralıkta kendi RF beacon'ı |

433.775 MHz LoRa APRS: SF12 / BW125 / CR4/5. SX1262/SX1268 ve SH1106/SSD1306 seçimi yapılabilir. BME280, WX bilgisi için isteğe bağlıdır.

## 2. İlk çalıştırma

1. RF TX öncesi uygun anteni takın ve geçerli amatör telsiz çağrı işareti/SSID ayarlayın.
2. [Web panelini](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_V1_1_0_STABLE.html) açın. USB için masaüstü Chrome/Edge, BLE için Android Chrome gibi Web Bluetooth destekli HTTPS tarayıcısı kullanın.
3. **Önce cihazdan oku**; gerekiyorsa **Kaydetme Açık** anahtarını açıp ayarları yazın; sonra yeniden okuyun.
4. `GPS FIX`, `LoRa RX OK`, pil gerilimi, radyo çipi ve APRS gönderim sayacını kontrol edin.
5. Sık değişen ayarlardan önce JSON yedeği alın; gelen APRS mesajlarını cihaz RAM'de tuttuğu için RESET sonrası silineceklerini unutmayın.

## 3. TRACKER: konum, hız, Comment, Status, WX

- **SMART Beacon:** GPS hızı ve dönüş koşullarına göre konum gönderim sıklığını ayarlar; **FIXED** seçilirse sabit süreler kullanılır.
- **Hareket / Park:** İki karakterli APRS sembolü ayrı ayrı seçilebilir (ör. hareket `/>`, park `\\P`).
- **Yön ve hız:** Geçerli ve güncel GPS kurs/hız bilgisi varsa APRS konumuna `ccc/sss` (derece / knot) eklenir; eski veya geçersiz veri uydurulmaz.
- **Comment:** Belirlenen süre geldiğinde normal konum paketine eklenir, ayrıca RF paketi oluşturulmaz.
- **Status:** Bağımsız APRS `>` paketi olarak gönderilir; V1.1.0'da rutin WX ve konum paketlerinden önce değerlendirilir.
- **WX:** Geçerli BME280 verisi ve GPS gerekiyorsa yayınlanabilir; BME280 yoksa hayalî hava durumu üretilmez.
- **TX güvenliği:** Her RF gönderimi ortak en az 10 saniyelik koruma aralığına tabidir. `RadioLib TX OK` yalnızca yerel radyo aktarımı başarılı anlamına gelir; diğer istasyonun aldığına garanti değildir.

## 4. APRS mesajları

1. Panelde **Mesajlar** sekmesini açın ve doğru USB/BLE cihazını seçin.
2. Alıcının tam APRS çağrı işaretini (ör. `TA1FM-7`) ve mesajı yazın.
3. **1, 2 veya 3 toplam RF denemesi** belirleyip gönderin.
4. `QUEUED`, `ACK`, `ACK_TIMEOUT` veya gönderim durumunu panelden izleyin.
5. Son **10 gelen mesaj** RAM'de tutulur; OLED/USB/BLE gelen kutusundan görüntülenir, enerji kesilince kaybolur.

**ACK**, karşı cihazın protokol yanıtıdır; alıcının mesajı okuduğu anlamına gelmez. APRS.fi görünürlüğü uygun bir iGate'e ulaşılmasına bağlıdır.

## 5. DIGI modu

- `mode digi` öncesi **geçerli çağrı işareti**, `digi lat` ve `digi lon` kayıtlı olmalıdır.
- `digi path 1`: WIDE1-1; `digi path 2`: desteklenen WIDE1/WIDE2 yolları.
- `digi beacon 10|15|30|60` ile sabit beacon aralığını (dk) ayarlayın.
- Modu değiştirdikten sonra konum/yol ve paketleri RF üzerinden test edin. Digi bir APRS-IS iGate değildir.

## 6. USB, BLE ve OLED

- **USB:** 115200 baud; Arduino Seri Monitör ve USB web panelini aynı COM portunda eşzamanlı kullanmayın.
- **Bluetooth:** cihaz adı `nRF52_Lora_Tracker`. Panelde okuma/yazma kilidini kontrol edin; PIN'siz kaydetme seçeneği güçlü kimlik doğrulama değildir.
- **OLED:** SH1106/SSD1306 tipi ayarlanabilir. USB'de `oled sh1106` / `oled ssd1306` önizleme ardından `oled confirm`; hatalı seçimde kurtarma yöntemini kullanın.
- **Firmware sürümü:** USB'de `version?`; BLE üzerinden `VERSION|1.1.0|STABLE` yanıtı; OLED'de `V1.1.0`.

## 7. Kablosuz firmware güncelleme (BLE DFU)

1. Release'ten **`.zip`** indirin.
2. V1.1.0 panelinden BLE ile bağlanın; **Kaydetme Açık** → **BLE DFU Moduna Geç** → onay.
3. Tracker **`AdaDFU`** adıyla görünür. Android **Nordic nRF Device Firmware Update** uygulamasını açın.
4. ZIP ve AdaDFU seçin; **PRN=4**, **Request High MTU=OFF** kullanın.
5. Gücü kesmeden işlemi tamamlayın; normal Tracker'a döndüğünü kontrol edin.

**Uyumluluk:** test edilen nice!nano UF2 0.10.0 / S140 6.1.1. **Legacy single-bank DFU** başarısızlığında USB UF2 kurtarma gerekebilir. Panel ZIP'i göndermez; yalnızca AdaDFU moduna geçirir. USB seri `dfu` komutu alternatifidir.

## 8. Sık komutlar

```text
version?
p
g
r
chip?
mode?
smart?
flash?
msg hello
msg inbox
msg status
msg send TA1FM-7 MERHABA
dfu
```

Tüm komutlar ve açıklamaları için [Seri Konsol Komut Rehberi](./SERI_KONSOL_KOMUT_REHBERI_TR.md).

## 9. Sorun giderme ve yedek

| Sorun | Öneri |
|---|---|
| GPS FIX yok | GPS kablolama, gökyüzü görüşü ve güncel uydu verilerini kontrol edin |
| LoRa RX hatası | SX1262/SX1268 seçimini ve yeniden başlatmayı doğrulayın |
| BLE bağlanamıyor | HTTPS, Bluetooth izinleri, başka bağlantı, cihazın AdaDFU'da olup olmadığı |
| OLED görünmüyor | Ekran sürücüsü ve OLED kurtarma |
| DFU aktarımı kopuyor | PRN 4, High MTU kapalı, güç; gerekirse USB UF2 ile kurtarma |
| Mesaj inbox sıfırlandı | RAM tabanlıdır; yeniden açılışta temizlenir |

**Önceki sürüm notları** Releases ve Git geçmişinde tutulur; güncel çalışma kılavuzu bu belgedir.
