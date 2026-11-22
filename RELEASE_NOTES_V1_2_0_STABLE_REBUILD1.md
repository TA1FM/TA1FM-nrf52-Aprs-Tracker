# TA1FM nrf52 Tracker — v1.2.0 STABLE REBUILD1

**Tarih / Date:** 22 Kasım / November 2026  
**Repo:** https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker

## Özet / Summary

Bu yeniden derleme, v1.1.8 BETA GPIO68 FIX1 sürümünde kullanıcı tarafından test edilen APRS/LoRa/GPS/BLE/OLED/telemetri/buzzer özelliklerini korur ve **ilk açılışta otomatik kalıcı ayar başlatma** davranışını ekler.

The rebuild retains the tested application features and adds **automatic first-run settings initialization**.

## Güncel özellikler

- TRACKER SMART/FIXED beacon, GNSS otomatik baud algılama; SX1262/SX1268 radyo seçimi.
- Sabit DIGI, WIDE tekrarlama, OLED SH1106/SSD1306.
- APRS mesaj RX/TX, ACK ve sınırlı yeniden gönderim; **gelen son 3 mesaj yalnızca RAM'de**.
- OLED **QUICK MSG**: 6 mesaj, toplam 200 ASCII bayt, mesaj başına 62 karakter; `|` ayraç, tek önceden seçilmiş alıcı CALL-SSID; fiziksel butonla gönderim.
- Aktif buzzer ve OLED mesaj bildirimi; **GPIO 6 / P0.06 saha testinde doğrulandı**, diğer izinli GPIO: 8, 33, 34, 39.
- İsteğe bağlı APRS Base91 pil/BME280 telemetri ve manuel PARM/UNIT/EQNS tanımları.
- USB ve BLE ile tek bağlantıda sekmeli panel; mevcut A/B kalıcı ayar kaydı + CRC32.

## REBUILD1 değişikliği: otomatik ilk kurulum

1. Geçerli A/B kayıt varsa **aynen yükler**, açılışta yeniden yazmaz.
2. Geçerli kayıt yoksa `0xF2000` ve `0xF3000` ayar bölgesini kullanarak BLE/SoftDevice hazır olduktan sonra **bir ilk ayar kaydı** oluşturur; `flash init` artık ilk kurulum için gerekmez.
3. Çağrı işareti `NOCALL` olduğunda APRS TX devre dışıdır. Kullanıcı kendi çağrı işaretini kaydedince cihaz kullanılabilir.
4. Yeni MiniMesh hedefinde SX1262 varsayılır. Önce başka yazılım kullanıldıysa bu iki **özellikle ayrılmış ayar sayfasındaki** eski veriler değiştirilebilir; bu bir tüm-chip erase işlemi değildir.

## Dosyalar / Downloads

- [Arduino kaynak dosyası](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino)
- [USB UF2 firmware](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.uf2)
- [Android BLE DFU ZIP](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.zip)

**v1.2.0 STABLE REBUILD1 USB sürüm çıktısı:** `VERSION|1.2.0|STABLE_REBUILD1`. BLE PING yanıtı: `PONG|TA1FM|V1.2.0_STABLE_REBUILD1`.

## Test kapsamı ve sınırlamalar

- **Önceki çalışan temel:** v1.1.8 BETA GPIO68 FIX1 üzerinde cihaz sahibi LoRa, GPS, BLE, ACK mesajlaşma, OLED hazır mesaj, telemetri ve P0.06 buzzer testlerinin başarılı olduğunu bildirdi.
- **REBUILD1 ilk kurulum kodu:** Kaynak düzeyinde kontrol edildi; başka bir cihazda temiz ilk kurulum ve eski ayarlarla güç kesme senaryolarının ayrıca gerçek donanım üzerinde doğrulanması gerekir.
- Kaynak, UF2 ve ZIP dosyaları ayrı dosya türleridir; bir dosyanın adının yeni olması otomatik olarak ikili derlemenin kaynakla birebir eşleştiğini kanıtlamaz.
- Gelen mesajlar flash'a yazılmaz; yeniden açılışta silinir. Telemetri etkinliği RAM-only'dir.
- Bu sürümde OLED sembol kısayolları `Car / Walker / House`. **Car / Motorcycle / Walker** değişikliği sonraki sürüme planlandı.
- BLE DFU `dfu arm` / `dfu go` ve 15 saniyelik bekleme penceresini kullanır. **Tek onaylı DFU** henüz uygulanmadı.
- **Güncel birleşik USB/BLE web paneli yayımlandı:** [v1.2.0 STABLE Panel](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/TA1FM_Tracker_Control_Panel_v1.2.0_STABLE.html) · [HTML kaynak kodu](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker-Configurator/blob/main/TA1FM_Tracker_Control_Panel_v1.2.0_STABLE.html). Eski v1.1.0 paneli yeni ayarların tamamını içermez.

## Güncelleme / Upgrade

Önce cihazdaki ayarları yedekleyin; USB UF2 veya uyumlu bootloader'da Android Nordic DFU ZIP ile güncelleyin. **Anten takılı olsun, `flash init`/format/erase komutlarını rutin güncelleme için çalıştırmayın.** BLE DFU'da test edilmiş örnek ayarlar **PRN 4 / High MTU OFF**. ACK bir insanın mesajı okuduğunu kanıtlamaz.

Ayrıntı: [Kurulum TR](./KURULUM_TR.md) · [Kullanım TR](./KULLANIM_TR.md) · [Installation EN](./INSTALL_EN.md) · [User Guide EN](./USER_GUIDE_EN.md).

**73 de TA1FM 📡**
