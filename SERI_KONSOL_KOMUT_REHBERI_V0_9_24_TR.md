# TA1FM nRF52 Tracker + Digipeater — V0.9.24 STABLE

## USB Seri Konsol Komut Rehberi (Türkçe)

**Kaynak kod doğrulaması:** 15 Kasım 2026 · **USB:** 115200 baud · **Satır sonu:** Newline (Yeni satır)

Bu rehber doğrudan [V0.9.24 STABLE .ino kaynak kodundaki](./TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_24_STABLE.ino) seri komutları esas alır. Komutları tek tek Enter ile gönderin. USB seri monitör ile USB Web Configurator aynı COM portunu eşzamanlı kullanamaz.

**Başlangıç:** `p` komutu cihazın ayrıntılı durumunu, `mode?` ise TRACKER / DIGI durumunu gösterir.

**Önemli:** `flash init` rutin güncellemelerde kullanılmaz. SX1262/SX1268 çip değişimi sonrası yeniden başlatma gerekir. LoRa TX için anten bağlı olmalıdır.

## 01. Durum ve tanılama

| Komut | Açıklama |
|---|---|
| `p / ?` | Geniş cihaz durumu, beacon ve mod bilgileri, mevcut komut yardım satırları |
| `g` | GPS FIX, uydu sayısı ve çözümlenen GPS karakterleri |
| `b` | BME280 sıcaklık/nem/basınç; sensör yoksa durumu |
| `r` | LoRa RX durumu, hata kodu ve alınan paket sayısı |
| `u` | UTC ↔ UTC+3 seçimini değiştirir ve kaydeder |
| `power?` | Ayarlanan TX gücünü gösterir |
| `smart?` | SMART hız aralıkları, hareket ve pil bilgisi |
| `chip?` | Seçili radyo çipi ve kayıt durumu |
| `flash?` | Direct Flash A/B kaydı ve ACTIVE durumu |

## 02. Çağrı işareti, RF ve çip

| Komut | Açıklama |
|---|---|
| `c TA1FM-8` | Örnek çağrı işaretini kaydeder; kendi yetkili çağrı işaretinizi kullanın |
| `chip sx1262` | SX1262 seçer; değişiklik sonrası yeniden başlatmak gerekir |
| `chip sx1268` | SX1268 seçer; değişiklik sonrası yeniden başlatmak gerekir |
| `power 1..22` | 1–22 dBm hedef gücü ayarlar; örneğin power 20 |

## 03. TRACKER / SMART Beacon

| Komut | Açıklama |
|---|---|
| `a on / a off` | Otomatik beacon sistemini açar / kapatır |
| `a parked / a always` | Kodda a on ile aynı davranan alternatifler |
| `f smart / f fixed` | Hıza bağlı SMART veya sabit aralıklı konum yayını |
| `i 1\|3\|5\|10\|15` | FIXED konum beacon aralığı (dakika) |
| `smart 600,90,45,30,20,15` | 7/10/20/49/60/90 km/sa için altı aralık (saniye) |
| `motion on / motion off` | İlk harekette erken beacon isteğini açar / kapatır |
| `batt on / batt off` | Konum paketine pil voltajı eklemeyi açar / kapatır |
| `k 1\|3\|5` | Park algılama bekleme süresi (dakika) |

## 04. APRS yayın, WX, Comment ve Status

| Komut | Açıklama |
|---|---|
| `t` | Manuel APRS konum yayını; TRACKER modunda GPS FIX gerekir |
| `w` | Manuel WX yayını; GPS FIX ve geçerli BME280 verisi gerekir |
| `v` | Manuel APRS Status; GPS FIX zorunlu değildir |
| `x off\|15\|30\|60` | WX aralığı: kapalı / 15, 30 veya 60 dakika |
| `z off\|15\|30\|60` | Status aralığı: kapalı / 15, 30 veya 60 dakika |
| `j off\|each\|15\|30\|60` | Comment ekleme sıklığı: kapalı, her konum paketi veya dakika |
| `comment METİN` | Konum Comment metnini kaydeder |
| `comment?` | Kayıtlı Comment metnini gösterir |
| `status METİN` | Status metnini kaydeder |
| `status?` | Kayıtlı Status metnini gösterir |

## 05. APRS harita sembolleri

| Komut | Açıklama |
|---|---|
| `s car` | Araç sembolü (/>) |
| `s walker` | Yaya sembolü (/[) |
| `s house` | Ev sembolü (/-) |
| `symbol />` | Normal hareket sembolü (2 karakter) |
| `parksymbol \P` | Park halindeki sembol (2 karakter) |

## 06. DIGIPEATER komutları — V0.9.24

| Komut | Açıklama |
|---|---|
| `mode? / digi?` | Aktif mod, DIGI aralığı, yol, koordinatlar ve sayaçlar |
| `mode tracker` | TRACKER moduna geç |
| `mode digi` | Sabit DIGI moduna geç; geçerli çağrı işareti ve koordinatlar gerekir |
| `digi lat 40.97500` | Örnek sabit DIGI enlemi; gerçek koordinatınızı girin |
| `digi lon 27.50400` | Örnek sabit DIGI boylamı; gerçek koordinatınızı girin |
| `digi path 1` | Yalnızca WIDE1-1 tekrarı |
| `digi path 2` | WIDE1-1, WIDE2-1, WIDE2-2 tekrarı |
| `digi beacon 10\|15\|30\|60` | Sabit DIGI beacon aralığı, dakika |

## 07. OLED

| Komut | Açıklama |
|---|---|
| `oled?` | OLED sürücüsü ve uyku durumu |
| `oled sh1106` | SH1106 sürücüsünü 20 sn önizlemeye alır |
| `oled ssd1306` | SSD1306 sürücüsünü 20 sn önizlemeye alır |
| `oled confirm` | Önizlemeyi 20 sn içinde kaydeder |
| `oled cancel` | Önizlemeyi iptal eder |
| `oled wake` | Ekranı uyandırır |
| `sleep 0` | Ekran sürekli açık |
| `sleep 1` | Ekran sürekli kapalı |
| `sleep 2` | 15 sn sonra uyku |
| `sleep 3` | 30 sn sonra uyku |
| `sleep 4` | 1 dk sonra uyku |
| `sleep 5` | 2 dk sonra uyku |
| `sleep 6` | 5 dk sonra uyku |

## 08. Bluetooth BLE

| Komut | Açıklama |
|---|---|
| `ble?` | BLE durumu ve mod bilgileri |
| `ble on` | BLE'yi etkinleştirir |
| `ble off` | BLE'yi devre dışı bırakır |

## 09. Direct Flash

| Komut | Açıklama |
|---|---|
| `flash?` | A/B kayıt durumunu gösterir |
| `flash init` | Yalnızca geçerli kayıt olmayan cihazda ilk kayıt oluşturur |

## DIGIPEATER kurulum örneği

**Örnek:** Çağrı işareti ve koordinatlar örnektir; kendi yetkili çağrı işaretinizi ve gerçek sabit konumunuzu girin.

```text
c TA1FM-6
digi lat 40.97500
digi lon 27.50400
digi path 2
digi beacon 15
mode digi
mode?
```

DIGI, aynı anda TRACKER değil, **seçilen ayrı sabit istasyon modudur**. Uygun WIDE paketlerini LoRa RF üzerinden tekrarlar; APRS-IS iGate değildir.

## SMART hız tablosu

| Hız noktası | Örnek aralık |
|---|---:|
| 7 km/sa ve altı | 600 s |
| 10 km/sa | 90 s |
| 20 km/sa | 45 s |
| 49 km/sa | 30 s |
| 60 km/sa | 20 s |
| 90 km/sa ve üzeri | 15 s |

SMART aralıkları hızla artmamalıdır; en az 10, en çok 3600 saniye (ilk değer en az 15 s). Aradaki hızlarda süreler hesaplanır.

## DIGI sayaçları

| Sayaç | Açıklama |
|---|---|
| `Eligible` | Tekrarlanabilir paket kuyruğa alındı |
| `TX` | RadioLib başarılı DIGI iletim |
| `TX_ERR` | DIGI iletim hatası |
| `DUP` | Mükerrer paket |
| `REJECT` | Geçersiz / tekrara uygun olmayan paket |
| `QUEUE_FULL` | Dört elemanlı kuyruk dolu |
| `EXPIRED` | Kuyrukta 45 saniye sonunda gönderilemeyen paket |
| `RX_RECOVER` | Başarılı RX toparlanması |
| `RX_RECOVER_ERR` | RX toparlanma hatası |

## Dikkat edilmesi gerekenler

- Bütün RF gönderimleri arasında en az **10 saniyelik ortak TX koruması** vardır; `RadioLib TX OK` karşıdaki iGate'in aldığını kanıtlamaz.
- DIGI mükerrer filtresi **kaynak çağrı işareti + APRS bilgi alanı** temelinde **30 saniye** çalışır. Dört paketlik kuyruk vardır.
- `comment` ve `status` metinleri 1–62 ASCII karakterdir; `|` ve `~` kabul edilmez. Comment uygun konum paketine eklenir.
- BLE üzerinden kullanılan `cfg?`, `cfg begin`, `cfg save` ve `set ...` mesajları USB seri komut listesine dahil değildir.
- Bu firmware'de `restart`, `reset`, `help`, `cad` isimli USB seri komutları tanımlı değildir.
- Test edilen saha konfigürasyonu SX1268 TRACKER modudur; DIGI iyileştirmeleri ve SX1262 ayrıca saha doğrulaması gerektirir.

## Proje bağlantıları

- [V0.9.24 STABLE GitHub Release](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v0.9.24)
- [USB/BLE Web Ayar Paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/beta_v0_9_23.html)
- [DIGIPEATER kurulum rehberi](./DIGIPEATER_V0_9_24_TR.md)

73 de TA1FM
