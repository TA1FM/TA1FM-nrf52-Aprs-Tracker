# TA1FM nrf52 Tracker + Digipeater — V0.9.24 STABLE

**Yeni özellik: APRS LoRa DIGIPEATER (DIGI) modu.** Aynı nRF52840 / MiniMesh Lite NRF cihazında **TRACKER** veya **sabit DIGI** modu seçilebilir. İki mod aynı anda değil, seçilen moda göre çalışır.

## DIGI modu ne yapar?

- 433.775 MHz LoRa APRS frekansında, uygun WIDE yoluna sahip paketleri RF üzerinden tekrar yayınlar.
- **WIDE1-1** veya seçime göre **WIDE1-1 / WIDE2-1 / WIDE2-2** yönlendirmesini destekler. `WIDE2-2` yolunu uygun durumda kendi çağrı işaretiyle tekrar edip kalan yolu `WIDE2-1` yapar.
- Tekrarladığı pakette kendi çağrı işaretini yıldızlı (`TA1FM-6*` örneği) işaretler. Çağrı işaretiniz istasyona özel olmalıdır.
- Kaynak çağrı işareti + APRS bilgi alanı aynı olan paketleri **30 saniye boyunca mükerrer sayar**; farklı içerikler ayrı değerlendirilir.
- **4 paketlik kuyruk**, **10 saniyelik RF gönderim koruması** ve kuyruk zaman aşımı vardır.
- Sabit kaydedilmiş enlem/boylam üzerinden kendi konum beacon'ını **10 / 15 / 30 / 60 dakika** aralıkla gönderebilir. Kuyrukta aktarılacak paketler önceliklidir.
- Seri monitörde kabul edilen, tekrarlanan, mükerrer ve atlanan paket sayaçlarını gösterir; RX yeniden başlatma hatasında kontrollü toparlanmayı dener.

> DIGI modu **APRS-IS'e doğrudan gönderim yapan internet iGate değildir**. APRS.fi'de görünürlük için başka bir iGate'in RF yayını alıp APRS-IS'e aktarması gerekir. `[DIGI TX OK]` yalnızca cihazın yerel RadioLib gönderim sonucudur; uzaktaki alımı garanti etmez.

## DIGI modunu açma

**Tercih edilen yöntem:** [V0.9.24 uyumlu USB + BLE Web Ayar Paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html). Önce mevcut ayarları okuyun, geçerli çağrı işaretinizi, sabit enlem-boylamı, beacon aralığını ve DIGI yolunu ayarlayın; ardından çalışma modunu **DIGI** olarak kaydedin.

**USB seri komutları (115200 baud)** — aşağıdakiler sadece örnektir; kendi yetkili çağrı işaretinizi ve gerçek istasyon koordinatlarınızı kullanın:

```text
digi lat 40.97500
digi lon 27.50400
digi path 2
digi beacon 15
mode digi
mode?
```

- `digi path 1`: yalnızca **WIDE1-1** için tekrar.
- `digi path 2`: **WIDE1-1**, **WIDE2-1**, **WIDE2-2** için tekrar.
- `digi beacon 10|15|30|60`: sabit DIGI beacon aralığı (dakika).
- `mode?`, `digi?` veya `p`: aktif modu ve DIGI sayaçlarını sorgular.
- TRACKER'a dönmek için `mode tracker`. TRACKER'ın SMART/FIXED ayarları mod değiştirildiğinde silinmez.

**Önemli:** DIGI'ye geçiş için geçerli çağrı işareti ve her iki sabit koordinat kaydedilmiş olmalıdır. Veri kaydı Direct Flash A/B + CRC32 üzerindedir; sıradan güncellemede `flash init` kullanmayın. Anten olmadan TX yapmayın.

## APRS yol örnekleri

```text
Giriş:  TA1FM-9>APZTFM,WIDE1-1:!...
Çıkış: TA1FM-9>APZTFM,TA1FM-6*:!...

Giriş:  TA1FM-9>APZTFM,WIDE2-2:!...
Çıkış: TA1FM-9>APZTFM,TA1FM-6*,WIDE2-1:!...
```

Bu örneklerde `TA1FM-6` DIGI çağrı işaretidir; cihazı kullanan istasyon kendi çağrı işaretini ve SSID'sini ayarlamalıdır.

## Sürüm durumu

V0.9.24 STABLE kaynak kodu ve UF2, saha testinde kullanılan V0.9.24 BETA ile **birebir aynı** dosyalardır. **15 Kasım 2026'da yaklaşık 10 km / 30 dakika boyunca SX1268 TRACKER modu test edilmiştir.** Yeni 30 saniyelik DIGI filtresinin, RX toparlanmasının ve SX1262 kullanımının ayrı saha testleri gereklidir. DIGI özelliği mevcuttur ancak bu test kapsamını aşan performans garantisi verilmez.

- [V0.9.24 STABLE UF2](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.uf2)
- [V0.9.24 STABLE Arduino kaynak kodu](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.ino)
- [GitHub v0.9.24 Release](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v0.9.24)

73 de TA1FM
