# TA1FM nrf52 Tracker — V0.9.23 BETA Test Notları

> **BETA / TEST SÜRÜMÜ.** Kararlı V0.9.22 korunmaktadır. Bu BETA henüz kararlı sürüm / GitHub Release olarak yayımlanmamıştır.

## Dosyalar

- Firmware: `TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_23_BETA.ino`
  - **Not:** Dosya adı BETA olarak bırakıldı; içerik önceki `BETA_FIX1` düzeltmesini içeren ve kullanıcının derleyip cihaza yüklediği koddur.
- USB + BLE panel: [beta_v0_9_23.html](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker-Configurator/blob/main/beta_v0_9_23.html)
- Kararlı firmware: `TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_22.ino` (aynen korunur).
- Kararlı web paneli: Configurator deposundaki `index.html` (aynen korunur).

## V0.9.23 BETA yenilikleri

- TRACKER ve SABIT DIGI modları; sabit koordinatlar GPS koordinatlarından bağımsız.
- DIGI otomatik konum beacon aralıkları 10 / 15 / 30 / 60 dakika; **varsayılan 15 dakika**.
- Gelen uygun LoRa APRS paketlerinde digipeater tekrarı, WIDE yol işleme ve mükerrer paket kontrolü.
- Her konumda Comment gönderme (`j each`) ve önceki zaman aralıklarını koruma.
- RF TX ortak koruması 10 saniye.
- BLE AÇ/KAPAT (OLED + USB/BLE paneli); OLED'de GPS INFO menüsü yerine BLE kontrolü.
- Direct Flash A/B kayıt ve geri yükleme sistemi korunur.

## Kullanıcı donanım testi — 15 Kasım 2026

Kullanıcının **SX1268** MiniMesh cihazında Arduino derlemesi ve karta yükleme yapılmıştır. Seri kayıtlarında:

| Test | Durum / kanıt |
|---|---|
| Cihazın başlaması, GPS FIX, LoRa RX | Başarılı |
| Direct Flash A/B ve kayıt sürekliliği | Başarılı; her iki slot geçerli, kaydetme onayları var |
| TRACKER ↔ SABIT DIGI geçişi | Başarılı |
| Digi sabit koordinattan kendi beacon'ı | `[DIGI] Own fixed beacon sent` ve `RadioLib TX OK` |
| TA1FM-9'dan WIDE1-1 gelen paketlerin tekrarı | `[DIGI TX] TA1FM-9>APZTFM,TA1FM-6*:...` |
| Mükerrer paket algılama | `[DIGI] Eligible=4 TX=4 DUP=1 REJECT=0` kaydı |
| `j each` Comment | TRACKER konum paketlerinin her birinde metin görüldü |
| Hareket / park sembolleri | `/>` ve `\\P` kullanımı doğrulandı |
| BLE kapatıp yeniden açma | Kullanıcı işlevsel olduğunu doğruladı |

**Henüz tamamlanmayan testler:**
- WIDE2-2 ile **gelen** paketin yolunun doğru azaltılıp RF'de tekrarlanması (yalnızca yerel mantık testi geçildi).
- Varsayılan 15 dakikalık Digi beacon zamanlamasının uzun süreli gerçek ortam doğrulaması (kullanıcı test için 10 dakikaya değiştirmişti).
- Digipeat paketinin **başka bir alıcı veya iGate tarafından duyulduğunun** bağımsız teyidi; `RadioLib TX OK` tek başına bunu kanıtlamaz.
- Uzun süreli RF/BLE/GPS kararlılığı.

## Bilinen küçük mesele

BME280 sensörü olmayan cihazlarda seri terminale tekrar tekrar `BME280 bulunamadi; BME menusu gizli.` mesajı yazılıyor. Şu aşamada işlevsel arıza tespit edilmedi; logu sadeleştirmek için sonraki düzeltmeye bırakıldı.

## Kritik notlar

- Bu dosyayı denemeden önce çalışan V0.9.22 kaynak ve UF2 yedeğini saklayın.
- DIGI denemelerinde bulunduğunuz bölgedeki frekans, yol ve trafik kurallarına uyun; kontrollü test yapın.
- **Bu BETA için henüz yeni bir UF2/release üretildiği doğrulanmadı.** GitHub'daki mevcut v0.9.22 Release kararlı sürümdür.

73 de TA1FM
