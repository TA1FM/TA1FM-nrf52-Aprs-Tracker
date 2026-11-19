# TA1FM nrf52 Tracker — USB Seri Konsol Komut Rehberi (V1.1.0 STABLE)

**Hedef firmware:** [V1.1.0 STABLE](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/releases/tag/v1.1.0) · **Port:** USB Serial **115200 baud** · **Satır sonu:** Newline.

Bu komutlar V1.1.0 STABLE kaynak kodundaki USB komut sistemi temel alınarak düzenlenmiştir. Seri Monitör ile USB web panelini aynı COM portunda aynı anda açmayın. `TA1FM-7` sadece örnek çağrı işaretidir.

## 1. Sürüm, durum ve tanılama

| Komut | Açıklama |
|---|---|
| `version?` | `VERSION\|1.1.0\|STABLE` sürüm bilgisi |
| `p` / `?` | Cihaz, GPS, LoRa, TX sayaçları, AUTO ve komut özeti |
| `g` | GPS karakter/fix/uydu durumu |
| `r` | LoRa RX ve alınan paket sayısı |
| `b` | BME280 ve ölçümler (sensör varsa) |
| `flash?` | Direct Flash A/B kayıt durumu |
| `chip?` | Radyo çipi ve kayıt durumu |
| `power?` | TX gücü |
| `smart?` | SMART hız aralıkları |
| `mode?` / `digi?` | TRACKER/DIGI, konum ve WIDE ayarları |
| `oled?` | OLED sürücü ve uyku bilgisi |
| `comment?` / `status?` | Kayıtlı APRS Comment / Status metni |

## 2. Radyo ve mod

| Komut | Açıklama |
|---|---|
| `c TA1FM-7` | Geçerli çağrı işaretini kaydet |
| `chip sx1262` / `chip sx1268` | Gerçek LoRa çipini seç; **restart gerekir** |
| `power 1` … `power 22` | TX gücü (dBm) |
| `mode tracker` / `mode digi` | Çalışma modunu değiştir |
| `ble on` / `ble off` | Bluetooth'u aç/kapat |
| `u` | UTC ve UTC+3 seçimini değiştir |

## 3. TRACKER / SMART ve APRS aralıkları

| Komut | Açıklama |
|---|---|
| `a on` / `a off` | Otomatik beacon açık/kapalı |
| `f smart` / `f fixed` | SMART veya sabit konum |
| `i 1` / `i 3` / `i 5` / `i 10` / `i 15` | FIXED konum aralığı (dk) |
| `smart 600,90,45,30,20,15` | Altı hız eşiği için aralıklar (saniye) |
| `motion on` / `motion off` | İlk hareket beacon'ı |
| `batt on` / `batt off` | Konum paketinde pil gerilimi |
| `k 1` / `k 3` / `k 5` | Park bekleme süresi (dk) |
| `x off` / `x 15` / `x 30` / `x 60` | WX gönderim aralığı |
| `z off` / `z 15` / `z 30` / `z 60` | Status gönderim aralığı |
| `j off` / `j each` / `j 15` / `j 30` / `j 60` | Comment ekleme aralığı |
| `comment METIN` / `status METIN` | Metin kaydet |
| `t` | Manuel konum TX |
| `w` | Manuel WX TX (GPS+BME280 gerekli) |
| `v` | Manuel Status TX |
| `symbol />` | APRS hareket sembolü (tablo+sembol) |
| `parksymbol \\P` | APRS park sembolü |
| `s car` / `s walker` / `s house` | Hazır normal sembol |

GPS yön/hız (`ccc/sss`) geçerli ölçüm olduğunda konum paketine otomatik eklenir. Status rutin WX/konumdan önce değerlendirilir; RF TX ortak 10 saniye koruma aralığına tabidir.

## 4. DIGIPEATER

| Komut | Açıklama |
|---|---|
| `digi lat 40.97500` | DIGI enlemi (örnek) |
| `digi lon 27.50400` | DIGI boylamı (örnek) |
| `digi path 1` / `digi path 2` | Tekrarlama WIDE yol politikası |
| `digi beacon 10` / `15` / `30` / `60` | DIGI beacon dakikası |
| `mode digi` | Geçerli çağrı işareti + iki koordinat gerektirir |

## 5. OLED

| Komut | Açıklama |
|---|---|
| `oled sh1106` / `oled ssd1306` | Sürücü önizlemesi |
| `oled confirm` / `oled cancel` | Önizleme onayı / iptal |
| `oled wake` | Ekranı uyandır |
| `sleep 0` … `sleep 6` | OLED uyku seçimi |

## 6. APRS mesajları (USB)

| Komut | Açıklama |
|---|---|
| `msg hello` | Mesajlaşma protokol bilgisi |
| `msg live` | Anlık durum |
| `msg inbox` / `msg list` | Gelen mesajları göster |
| `msg status` / `msg?` | Çıkış/ACK durumunu göster |
| `msg send TA1FM-7 SELAM` | 1 toplam RF denemesi |
| `msg send2 TA1FM-7 SELAM` | 2 toplam RF denemesi |
| `msg send3 TA1FM-7 SELAM` | 3 toplam RF denemesi |
| `msg cancel` | Bekleyen mesajı iptal |
| `msg clear` | Gelen kutusunu temizler |
| `testmsg TA1FM-7 TEST` | Yerel test enjeksiyonu; RF TX değildir |

Mesaj ACK'sı cihaz/protokol düzeyindedir; bir kişi tarafından okunma onayı değildir. Gelen kutusu 10 mesajlık RAM alanıdır.

## 7. Firmware / Flash

| Komut | Açıklama |
|---|---|
| `dfu` | USB'den **AdaDFU** bootloader'a yeniden başlat |
| `version?` | Merkezi firmware sürümünü bildir |
| `flash?` | Flash kayıt durumunu sorgula |
| `flash init` | **Yalnızca gerekliyse** Direct Flash başlat; normal güncellemede **KULLANMAYIN** |

**BLE DFU:** Android Nordic DFU uygulamasında **ZIP + AdaDFU**, **PRN 4**, **Request High MTU OFF**. nice!nano UF2 0.10.0/S140 6.1.1 test edilmiştir. Kesintide USB UF2 kurtarma gerekebilir; detaylar [Kurulum](./KURULUM_TR.md).

**ÖNEMLİ:** Eski V0.9.24 konsol rehberi arşivdir; bu belge **V1.1.0 STABLE** içindir. RF TX için anten ve geçerli lisanslı çağrı işareti kullanın.
