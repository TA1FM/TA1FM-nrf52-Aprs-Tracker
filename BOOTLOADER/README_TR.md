# TA1FM nRF52 — nice!nano UF2 Bootloader Arşivi

Bu klasör, **TA1FM nrf52 Tracker + Digipeater** projesinde kullanılan nice!nano uyumlu nRF52840 kartlarının bootloader **güncelleme dosyasını** kaybetmemek ve gerektiğinde kolayca bulabilmek için oluşturulmuştur.

## Arşivdeki dosya

**[update-nice_nano_bootloader-0.11.0_nosd.uf2](./update-nice_nano_bootloader-0.11.0_nosd.uf2)**

| Özellik | Açıklama |
| --- | --- |
| Hedef | **nice!nano / nRF52840-nicenano** ile uyumlu olduğu doğrulanan kartlar |
| Sürüm | **0.11.0** |
| Dosya türü | **UF2 bootloader güncelleme paketi** |
| Kullanım amacı | **Zaten çalışan bir UF2 bootloader'ını güncellemek** |
| İlk kez boş çipe yükleme | **Hayır; SWD programlayıcı ve karta uygun HEX/ilk programlama dosyaları gerekir** |
| TA1FM uygulama yazılımı | **Değildir**; Tracker/Digipeater firmware'inden ayrıdır |

> **Önemli:** Dosya adındaki `nosd` ifadesi, bunun SoftDevice içeren tam bir ilk-programlama HEX paketi olduğu anlamına **gelmez**. Mevcut SoftDevice'i kurduğunu veya her karttaki bellek düzenini otomatik düzelttiğini varsaymayın. Kartın `INFO_UF2.TXT` içindeki **Board-ID** ve **SoftDevice** bilgilerini güncelleme öncesinde ve sonrasında ayrı ayrı kontrol edin. TA1FM V0.9.24 BLE firmware'i için S140 6.1.1 tabanlı uyumlu yapı önemlidir.

## Güncellemeye gerçekten ihtiyaç var mı?

**Çalışan cihazda, yalnızca daha yeni sürüm var diye bootloader güncellemeyin.** Yanlış kartın bootloader'ını yüklemek veya güncellemeyi yarıda kesmek USB/UF2 erişimini bozabilir; yeniden SWD programlama gerekebilir.

Öncelikle mevcut sürümü öğrenin:

1. Kartı **veri aktarabilen USB kablosuyla** bilgisayara bağlayın.
2. Kartın `RST` ucunu `GND` ile **hızlıca iki kez** kısa süreli temas ettirerek UF2 moduna girmeyi deneyin (kartın kendi reset yöntemi farklıysa onu izleyin).
3. Bilgisayarda açılan USB sürücüsünde genellikle `INFO_UF2.TXT` dosyası bulunur.
4. **Bootloader sürümü, Model, Board-ID ve SoftDevice** satırlarını okuyun. `Board-ID` değerini bu dosyanın hedeflediği **nRF52840-nicenano** ailesiyle karşılaştırın. **Model adı, kart sürümü veya SoftDevice/bellek düzeni uyumsuzsa güncellemeyi uygulamayın.**

Örnek olarak dosyada `Board-ID: nRF52840-nicenano` ve `SoftDevice: S140 version 6.1.1` görülebilir; **bunlar sizin cihazınızın gerçek bilgisi değil, kontrol edilecek örnek alanlardır.**

## Uyumlu kartta USB üzerinden güncelleme

**Yalnız mevcut UF2 bootloader çalışıyorsa ve güncelleme dosyasının kartla uyumunu doğruladıysanız:**

1. Önce çalışan **TA1FM uygulama UF2'sini** ve mümkünse web ayar panelinden **JSON ayar yedeğini** saklayın.
2. `INFO_UF2.TXT` içeriğinin fotoğrafını veya kopyasını alın.
3. Kartı UF2 moduna alın (çoğu nice!nano uyumlu kartta hızlı çift RESET). `NICENANO` benzeri bir USB sürücüsü görünür.
4. **`update-nice_nano_bootloader-0.11.0_nosd.uf2`** dosyasını UF2 sürücüsüne kopyalayın. İşlem bitene ve kart yeniden başlayana kadar bağlantıyı kesmeyin.
5. Tekrar UF2 moduna girin; `INFO_UF2.TXT` içindeki **sürümün 0.11.0** olduğunu, **Board-ID** ve **SoftDevice** bilgilerinin beklenen yapıda kaldığını doğrulayın.
6. Gerekirse proje için doğru uygulama firmware'ini yeniden yükleyin. GPS, LoRa, USB ve BLE işlevlerini test edin.

**Dosyayı kopyalamak tek başına başarılı güncellemeyi kanıtlamaz.** Yeniden açılan `INFO_UF2.TXT` ve uygulama testiyle doğrulama yapın.

## Bootloader hiç yoksa veya USB/UF2 açılmıyorsa

Bu `.uf2` dosyası **tamamen boş bir nRF52840'a ilk bootloader yükleyemez**. Önce USB veri kablosu, besleme ve reset yöntemini eleyin. Gerçekten bootloader yoksa **SWDIO / SWCLK / GND / VTref** bağlantısıyla J-Link veya uygun bir SWD programlayıcı ve **doğru kart için hazırlanmış bootloader/SoftDevice HEX** gerekir.

Bu klasördeki UF2 güncelleme dosyasını bir **SWD HEX dosyasıymış gibi kullanmayın**. **`Erase All` / `Recover`** işlemlerini de çalışan TA1FM kartlarına rastgele uygulamayın.

## TA1FM v1.2.0 STABLE REBUILD1 ile ilişkisi

Bu klasördeki dosya, **uygulama firmware'i değil**, ayrı bir kurtarma/güncelleme kaynağıdır. **TA1FM v1.2.0 STABLE REBUILD1 için de mevcut çalışan bootloader'ı yalnızca bu arşivde dosya var diye değiştirmeyin.** Uyumlu USB UF2 ve BLE DFU yapısı cihazın kendi bootloader/SoftDevice düzenine bağlıdır. Otomatik ilk ayar kaydı için bootloader güncellemesi gerekmez.

- [v1.2.0 STABLE REBUILD1 uygulama UF2](../TA1FM_nrf52_Tracker_v1.2.0_STABLE.ino.uf2)
- [Güncel kurulum rehberi](../KURULUM_TR.md)
- [Güncel kullanım kılavuzu](../KULLANIM_TR.md)
- [TA1FM nrf52 Tracker GitHub ana sayfası](../README.md)

## Başvuru kaynakları

- [nice!nano — Bootloader sorun giderme / SWD kurtarma](https://nicekeyboards.com/docs/nice-nano/troubleshooting/)
- [Adafruit nRF52 Bootloader — kaynak kodu](https://github.com/adafruit/Adafruit_nRF52_Bootloader)
- [UF2 bootloader sürümü ve INFO_UF2.TXT hakkında bilgi](https://circuitpython.org/board/nice_nano/)

---

**Arşiv notu:** Kartın üzerinde görünen **Board-ID, SoftDevice sürümü ve fiziksel Pro Micro varyantı** doğrulanmadan bu dosyanın *her* nRF52840 Pro Micro klonuna uygun olduğu iddia edilmez.

**TA1FM nrf52 Tracker + Digipeater · Bootloader Arşivi · 73 de TA1FM! 📡**
