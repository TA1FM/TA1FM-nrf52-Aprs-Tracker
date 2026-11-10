# TA1FM nRF52 APRS Tracker — Türkçe kurulum

## Desteklenen donanım
Bu sürüm **TA2DMX MiniMesh Lite NRF (nRF52840)** kartı ve kullanılan özel pin eşlemesi içindir. Genel nRF52840 kartlarına doğrudan yüklemeyin. Test ortamında nice!nano uyumlu UF2 bootloader v0.11.0 ve S140 6.1.1 bulunmaktadır.

## Hazır UF2 ile yükleme
1. Çalışan firmware ve ayarlarınızı yedekleyin.
2. Kartın bootloader / SoftDevice uyumunu doğrulayın.
3. RESET tuşuna art arda iki kez basıp UF2 USB sürücüsünü açın.
4. `TA1FM_nrf52_Tracker_V0_9_2_STABLE.uf2` dosyasını bu sürücüye kopyalayın.
5. Cihaz normal açıldığında GPS, LoRa ve ayar durumlarını doğrulayın.

**Normal güncellemede silici UF2, LittleFS formatı veya tam flash silme işlemi uygulamayın.** Mevcut ayarlar korunacak şekilde tasarlanmıştır.

## Web arayüzünden ayarlama
[TA1FM Web Configurator](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/) sayfasını **masaüstü Chrome/Edge** ile açın. USB'den bağlanın; cihazdan okuyun, alanları düzenleyin, cihaza yazın. JSON yedekleme ve seri monitör de bulunmaktadır.

## Arduino kaynak kodundan derleme
Arduino IDE, **Adafruit nRF52 core 1.7.0** ve projeye ait **TA1FM MiniMesh Lite NRF (TA2DMX)** özel kart tanımı kullanılmıştır. Kütüphaneler: **RadioLib 7.8.1**, Adafruit TinyUSB (core ile birlikte), Adafruit GFX, Adafruit SH110X, Adafruit SSD1306, TinyGPSPlus, Adafruit BME280, Adafruit Unified Sensor, Adafruit LittleFS / InternalFileSystem (core). Özel kart tanımı/pin varyantı yoksa sıradan nRF52840 kartı seçip derlemek veya yüklemek doğru değildir.

## Seri port komutları (115200 baud)

| Komut | Görevi |
|---|---|
| `p` | Kaydedilmiş ve anlık durum bilgilerini yazdırır |
| `a on` / `a off` | Otomatik Beacon aç/kapat, kalıcı olarak kaydet |
| `f smart` / `f fixed` | Smart veya sabit konum Beacon |
| `i 1`, `i 3`, `i 5`, `i 10`, `i 15` | Sabit konum aralığı (dakika) |
| `x off`, `x 15`, `x 30`, `x 60` | WX aralığı |
| `z off`, `z 15`, `z 30`, `z 60` | Status aralığı |
| `j off`, `j 15`, `j 30`, `j 60` | Comment eklenme aralığı; bağımsız RF paketi değildir |
| `k 1`, `k 3`, `k 5` | Park algılama bekleme süresi |
| `s car`, `s walker`, `s house` | Bu sürümde desteklenen APRS sembolleri |
| `comment METIN` | Konum Beacon'ına eklenecek yorum |
| `status METIN` | APRS status içeriği |
| `comment?` / `status?` | Kayıtlı metni sorgula |
| `t` | Manuel konum gönderimi (uygun GPS/callsign gerekli) |
| `w` | Manuel WX gönderimi (BME280 gerekli) |
| `v` | Manuel Status gönderimi |

**Not:** Motosiklet sembolü bu **V0.9.2** sürümünde henüz eklenmedi; daha sonraki sürümün geliştirme hedefidir. V0.9.2'de `s car`, `s walker`, `s house` desteklenir. Seri komutta ilk çağrı işareti tanımlanırken `c TA1FM-7` örnek olarak kullanılabilir; herkes kendi yetkili çağrı işaretini kullanmalıdır.

## Kalıcı kayıt hakkında
V0.9.2, LittleFS'te geçici dosya / doğrulama / değiştirme yaklaşımıyla ayarları kaydeder. AUTO ON, Comment ve Status kayıtları çalışır durumda test edilmiştir. Cihaz kapatılıp açıldığında ayarların geri geldiği doğrulanmıştır. Kayıt hatası görülürse tekrarlı yazma denemesi yapmayın; flash sorunlarını araştırın.

## Güvenlik
Kullanıcılar kendi amatör telsiz ruhsatlarını, çağrı işaretlerini, bölgesel frekans/güç kurallarını ve anten güvenliğini gözetmelidir.
