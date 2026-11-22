# TA1FM nrf52 Tracker — Seri Konsol Komut Rehberi (v1.2.0 STABLE REBUILD1)

**USB 115200 baud · Satır sonu: Newline.** [Kurulum](./KURULUM_TR.md) · [Kullanım](./KULLANIM_TR.md)

Komutlar güncel [v1.2.0 kaynak koduna](./TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino) göredir. Aynı COM portunu hem Arduino Seri Monitör hem web panelinde açmayın. Örnek çağrı işaretlerini kendi lisanslı çağrı işaretinizle değiştirin.

## 1. Sürüm ve tanı

| Komut | İşlev |
|---|---|
| `version?` | `VERSION|1.2.0|STABLE_REBUILD1` |
| `p` / `?` | Genel cihaz durumu ve komut özeti |
| `g` / `r` / `b` | GPS / LoRa / varsa BME280 bilgileri |
| `gnss?` / `gnss baud?` | GNSS tanı ve otomatik baud durumu |
| `gnss debug` / `gnss gsv on` | GNSS ayrıntı/GSV tanılama |
| `gnss probe` | GNSS modül probu; çalışırken dikkatli kullanın |
| `chip?` / `power?` | Radyo çipi / TX gücü |
| `mode?` / `digi?` | Çalışma modu / sabit DIGI bilgileri |
| `smart?` | SMART Beacon süreleri |
| `oled?` | OLED sürücü/uyku durumu |
| `comment?` / `status?` | APRS Comment/Status |
| `flash?` | Direct Flash A/B ayar kayıtları |
| `preset?` | Kaydedilmiş OLED hızlı mesaj listesi özeti |
| `notify?` | Buzzer GPIO/aktiflik ve OLED bildirim durumu |
| `telem?` | Telemetri açık/kapalı ve sayaç |

## 2. Çağrı işareti, radyo ve mod

| Komut | İşlev |
|---|---|
| `c TA1FM-7` | Örnek çağrı işaretini kaydet |
| `chip sx1262` / `chip sx1268` | LoRa çipini seç; ardından yeniden başlatmak gerekebilir |
| `power 1` ... `power 22` | RF TX gücü (dBm) |
| `mode tracker` / `mode digi` | Çalışma modu |
| `ble on` / `ble off` | BLE reklamını aç/kapat |
| `u` | UTC / UTC+3 görünümü |

## 3. Beacon, semboller, WX

| Komut | İşlev |
|---|---|
| `a on` / `a off` | Otomatik beacon |
| `f smart` / `f fixed` | SMART / FIXED |
| `i 1` / `i 3` / `i 5` / `i 10` / `i 15` | Sabit konum aralığı (dk) |
| `smart 600,90,45,30,20,15` | SMART hız aralıkları (s) |
| `motion on` / `motion off` | Hareket tetikleyicisi |
| `batt on` / `batt off` | Konum paketinde pil değeri |
| `k 1` / `k 3` / `k 5` | Park beklemesi (dk) |
| `x off` / `x 15` / `x 30` / `x 60` | WX aralığı |
| `z off` / `z 15` / `z 30` / `z 60` | Status aralığı |
| `j off` / `j each` / `j 15` / `j 30` / `j 60` | Comment aralığı |
| `comment METIN` / `status METIN` | APRS metinleri |
| `symbol />` / `parksymbol \\P` | Hareket / park sembolü |
| `s car` / `s walker` / `s house` | Mevcut 3 OLED sembol kısayolu; **motorcycle henüz eklenmedi** |
| `t` / `w` / `v` | Manuel position / WX / Status RF gönderimi |

RF anteni olmadan TX yapmayın. Ortak TX koruması en az 10 saniyedir.

## 4. DIGIPEATER

| Komut | İşlev |
|---|---|
| `digi lat 40.97500` / `digi lon 27.50400` | Sabit koordinatlar |
| `digi path 1` / `digi path 2` | WIDE tekrarlama yolları |
| `digi beacon 10` / `15` / `30` / `60` | DIGI beacon aralığı (dk) |
| `mode digi` | DIGI modu: geçerli çağrı işareti + koordinatlar |

## 5. OLED ve APRS mesajları

| Komut | İşlev |
|---|---|
| `oled sh1106` / `oled ssd1306` | OLED önizleme |
| `oled confirm` / `oled cancel` | Onay / iptal |
| `oled wake` | Ekranı uyandır |
| `sleep 0` ... `sleep 6` | OLED uyku seçimi |
| `msg hello` / `msg live` | Mesaj protokolü / canlı durum |
| `msg inbox` / `msg list` | **Son 3** gelen mesaj, RAM |
| `msg status` / `msg?` | Gönderim/ACK durumu |
| `msg send TA1FM-7 SELAM` | Tek RF denemesi |
| `msg send2 TA1FM-7 SELAM` | En fazla 2 RF denemesi |
| `msg send3 TA1FM-7 SELAM` | En fazla 3 RF denemesi |
| `msg cancel` / `msg clear` | Bekleyen gönderim / gelen RAM kutusunu temizleme |
| `testmsg TA1FM-7 TEST` | Yerel mesaj enjeksiyonu; RF değildir |
| `msg flash?` | RAM-only mesaj kaydını doğrular, flash'a yazmaz |

### OLED hazır mesaj yönetimi

| Komut | İşlev |
|---|---|
| `preset?` | Hazır mesaj ve alıcı özeti |
| `preset get 0` ... `preset get 3` | Kayıtlı listeyi parça parça okumak için panel protokolü |
| `preset begin CALL LEN` | Hazır mesaj aktarımını başlat |
| `preset part INDEX CHUNK` | Numaralı ASCII parça gönder (0–3) |
| `preset commit` / `preset cancel` | Değişiklikleri tek kayıtla tamamla / iptal et |

**Öneri:** Hazır mesajları elle komutlamak yerine güncel birleşik paneli kullanın. Liste `|` ile ayrılır; en fazla **6 mesaj**, **200 ASCII byte**, mesaj başına **62 karakter**. Alıcı CALL-SSID zorunludur. BLE yazmada kayıt izni gereklidir.

## 6. Buzzer ve OLED otomatik uyanma

| Komut | İşlev |
|---|---|
| `notify?` | Kalıcı buzzer / OLED ayarları |
| `notify save 6 1 1 30` | GPIO **6**, buzzer açık, OLED bildirim açık, **30 sn**; kalıcı ayar yazımı |
| `notify save -1 0 1 30` | Buzzer kapalı, OLED uyanma açık |
| `buzzer test` | Çift/çoklu bip donanım testi |
| `buzzer?` / `buzzer on` / `buzzer off` | Anlık buzzer durumu ve çalışma testi (**kalıcı ayar değildir**) |

İzinli pinler **-1 (kapalı), 6, 8, 33, 34, 39**. **GPIO 6 / P0.06** MiniMesh kartında test edilmiştir. Diğer pinlerin fiziksel uygunluğunu doğrulayın. OLED uyanma süresi **15/30/60 sn**.

## 7. Base91 telemetri

| Komut | İşlev |
|---|---|
| `telem on` / `telem off` | RAM'de telemetri aç/kapat |
| `telem?` | Durum |
| `telem defs` | APRS PARM/UNIT/EQNS (3 RF paket) sıralaması |
| `telem cancel` | Tanım paketlerini iptal et |

Telemetri aktifliği **yeniden başlatmada sıfırlanır**. RF tanım paketleri, ortak 10 saniyelik TX korumasına ve ACK önceliğine uyar.

## 8. Firmware ve kalıcı ayarlar

| Komut | İşlev |
|---|---|
| `version?` | Firmware sürümü |
| `flash?` | A/B kayıt ve sıra numarası |
| `dfu` | USB üzerinden AdaDFU bootloader giriş talebi |
| `flash init` | **Eski/ileri düzey komut:** REBUILD1 ilk kurulumu artık otomatik yapar; normal kullanımda gerekli değil |

İlk açılışta `[DIRECT] AUTO INIT OK` beklenir; `NOCALL` durumunda APRS TX başlamaz. Geçerli eski ayarlar açılışta yazılmadan korunur. BLE DFU'da `dfu arm` (15 sn) / `dfu go`; Android Nordic DFU uygulamasında **ZIP, PRN 4, High MTU OFF**.

**73 de TA1FM 📡**
