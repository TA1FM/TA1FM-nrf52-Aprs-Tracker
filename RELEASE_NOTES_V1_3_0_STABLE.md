# TA1FM nrf52 Tracker — v1.3.0 STABLE

**Tarih / Date:** 24 Kasım / November 2026

**Kaynak / Source:** [TA1FM_nrf52_Tracker_v1.3.0_STABLE.ino](./TA1FM_nrf52_Tracker_v1.3.0_STABLE.ino)

## Sürüm esası

Bu STABLE kaynak, cihaz sahibinin sahada test ederek başarılı olduğunu bildirdiği **v1.3.0 BETA3** sürümünün birebir işlevsel karşılığıdır. BETA3 tabanından STABLE'a geçerken **yalnızca sürüm başlık açıklaması ve `FW_CHANNEL` etiketi değiştirilmiştir**; GPS, LoRa, APRS, Flash, BLE, OLED ve diğer çalışan mantıklar değiştirilmemiştir.

- Açılış: `UNIVERSAL V1.3.0 STABLE`
- USB `version?`: `VERSION|1.3.0|STABLE`
- BLE `ping`: `PONG|TA1FM|V1.3.0_STABLE`

## v1.2.0 → v1.3.0 değişiklikleri

1. **OLED DFU MODE:** Fiziksel menüde açılma, ayrı basılı tutma onayı, AdaDFU geçişi; kullanıcı OLED DFU'nun açık kaldığını test etti.
2. **OLED SYSTEM STATUS:** GPS, LoRa, BLE, A/B flash ayar ve pil durumları.
3. **OLED simge kısayolları:** Car, Motorcycle, Walker (APRS sembolleri).
4. **SX1268 varsayılanı:** Yalnızca geçerli kayıt bulunmayan ilk kurulumda; daha önce kaydedilmiş SX1262/SX1268 tercihi korunur.
5. **BLE DFU web paneli:** ARM/GO tek kullanıcı onayıyla başlatılabilir; Nordic Legacy DFU firmware ZIP aktarmak için harici Nordic DFU uygulaması gereklidir.
6. **Telemetri düzeltmesi:** BME280 yoksa veya geçerli/fresh sensör verisi gelmiyorsa konumda **yalnızca pil voltajı Base91 telemetrisi** gönderilir; sahte sıcaklık/nem/basınç gönderilmez. BME280 varsa pil + sıcaklık + nem + basınç gönderilebilir. Pilin normal `Batt=...V` yorumu ayrı ayarla korunur.
7. **`telem defs`:** BME280 yokken `PARM.Batt`, `UNIT.V`, `EQNS.0,0.01,0`; sensör sağlıklıysa tam 5 analog kanala ait tanımlar. aprs.fi'deki eski tanımların düzeltilmesi için manuel gönderin.
8. **APRS Comment:** Etkin olduğunda her konum paketinde korunur; gereksiz ek RF paketi yoktur. Mevcut 15/30/60 dakika Comment seçimi bu sürümde ayrı tekrar aralığı olarak kullanılmaz, **açık/kapalı** davranışı sağlar.
9. **Status önceliği:** Status süresi dolduğunda rutin/stale konum beacon'larının sürekli öne geçmesini önlemeye yönelik zamanlama düzenlemesi. `RadioLib OK`, RF TX'in yerel başarısıdır; aprs.fi'de görülmeyi garanti etmez.
10. **Web panel mesajlaşma:** Birleşik BETA2 web panelinde gönderilen mesajlar da görünür (sıraya alındı / ACK bekleniyor / RF ACK), fakat paneldeki gönderim geçmişi **yalnızca açık tarayıcı oturumunda** saklanır. Gelen son 3 APRS mesajı cihaz RAM'indedir, resetle silinir.

## Uyumlu kontrol paneli

Bu sürüm için **ayrı panel değişikliği gerekmiyor**:

- [Birleşik web paneli v1.3.0 BETA2 — USB / BLE](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/BETA/TA1FM_Tracker_Control_Panel_v1.3.0_BETA2.html)

Panelin kendi `BETA2` adı, onun özellik geliştirme sürümünü belirtir; STABLE firmware'i sürüm eşleşmedi diye engellemez. Mevcut panel koduna dokunulmamıştır.

## Kullanım notları

- Telemetri aç/kapat anahtarı RAM-only'dir: yeniden başlatınca `telem on` gerekebilir. Tanım paketleri otomatik gönderilmez; `telem defs` manuel çalıştırılır.
- BME280 takıldığında veya çıkarıldığında farklı sayıda kanal kullanıldığı için yeni modun PARM/UNIT/EQNS tanımlarını tekrar gönderin. aprs.fi geçmişteki yanlış ölçümleri bir süre göstermeye devam edebilir.
- GNSS/LoRa/OLED butonları, mesaj TX/ACK, buzzer, ayar yedekleri, A/B direkt Flash düzeni ve OTA/UF2 donanım uyumluluğu korunmuştur.
- Yalnız MiniMesh Lite NRF / nRF52840 ile **doğru variant ve pin eşlemesi** üzerinde kullanılmalıdır.
- Önceki `v1.2.0 STABLE` ve `v1.3.0 BETA3` kaynakları arşiv olarak tutulmuştur; üzerine yazılmamıştır.

## Yayın durumu

**STABLE kaynak dosyası hazır ve GitHub'a kaydedildi.** Kaynak yalnızca sürüm kimliği değiştirilerek hazırlanmıştır. Bu oturumda Arduino derlemesi yapılmadı, bu nedenle **v1.3.0 STABLE UF2 ve Nordic BLE DFU ZIP dosyaları henüz oluşturulmadı** ve GitHub Releases üzerinde eksiksiz ikili paket yayımlanmış sayılmaz. Her iki paketin bu yeni STABLE kaynakla, doğru nRF52 board/bootloader seçenekleriyle derlenip yüklenmesi ve yeniden açılışta `V1.3.0 STABLE` göstermesi gerekir.

**73 de TA1FM 📡**
