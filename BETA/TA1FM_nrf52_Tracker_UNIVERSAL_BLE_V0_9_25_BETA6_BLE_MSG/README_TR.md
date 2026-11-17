# TA1FM nrf52 Tracker — V0.9.25 BETA-6 BLE Mesajlaşma

**Deneysel sürüm, Arduino IDE derlemesi ve gerçek cihazda BLE/RF testi henüz yapılmadı.** BETA-5 ACK sürümünü temel alır. V0.9.24 STABLE ve BETA-1/2/3/4/5 dosyalarına dokunulmamıştır.

## Dosyalar

- **[BETA-6 firmware tam .ino](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker/blob/main/BETA/TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA6_BLE_MSG/TA1FM_nrf52_Tracker_UNIVERSAL_BLE_V0_9_25_BETA6_BLE_MSG.ino)**
- **[BLE Mesajlaşma Web Paneli (ayrı BETA HTML)](https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker-Configurator/blob/main/BETA/TA1FM_BLE_APRS_Mesaj_Paneli_BETA6.html)**
- [GitHub Pages BETA mesajlaşma paneli](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/BETA/TA1FM_BLE_APRS_Mesaj_Paneli_BETA6.html) (HTTPS yayını etkin ve yeni commit dağıtılmışsa)

Eski `beta_v0_9_23.html` ve `index.html` mevcut ayar panelleri **aynen korunuyor**. BLE BETA-6 paneli, V0.9.23 BLE ayar panelini temel alır; cihazın ayar okumayı/yazmayı sürdürebilmesi için eski protokol değiştirilmemiştir.

## Mesaj gönderimi

1. BETA-6 .ino dosyasını **aynı isimli klasörde** Arduino IDE ile aç ve **Derle/Doğrula**.
2. Derleme başarılıysa karta yükle. **BETA-6 cihazdaki eski firmware'in tamamının yerine geçer**; GitHub STABLE ve BETA-5 kaynakları korunur. Yüklemeden önce çalışan UF2 ve cihaz JSON ayarlarını yedekle.
3. Android Chrome'da HTTPS Web panelini aç, **Bluetooth ile bağlan** seçeneğine tıkla ve `nRF52_Lora_Tracker` cihazını seç. Tarayıcı ilk bağlanma için standart Bluetooth erişim izni ister.
4. **Alıcı çağrı işareti** örn. `TA1FM-10` ve **Mesaj** örn. `SELAM KANKA` yaz, **RF MESAJI GÖNDER** düğmesine bas.
5. **Ek cihaz TX izni YOK**: fiziksel buton/ARM komutu/ayrıca BLE Kaydetme açık anahtarı istemez. Gönder komutu BLE NUS üzerinden direkt cihazın RF mesaj kuyruğuna gider.
6. Cihazdan `MSG|QUEUED|TA1FM-10|0001` gelirse panel **giden** mesajı görüntüler; sonraki `MSG|OUT|` bildirimleri ile `WAIT_ACK`, `ACK_RECEIVED`, `ACK_TIMEOUT` vb. görünür.
7. `Gelen mesajları oku` düğmesi cihaza gelen en yeni 10 RAM mesajını getirir. Panel açıkken gelen yeni mesaj bildirimleri ve yaklaşık 15 saniyelik senkronizasyon da kullanılır.
8. `Tekrarları iptal et` düğmesi giden mesajı iptal eder; **daha önce anten üzerinden gönderilen yayını geri alamaz**.

## BLE protokolü

Nordic UART Service değişmedi: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`. Var olan BLE RX/TX GATT karakteristikleri ve **18 baytlık bölerek yazma** korunur.

| Tarayıcıdan cihaza | Cevap |
|---|---|
| `msg hello` | `MSG|HELLO|B6|BLE_APRS_MESSAGING` |
| `msg status` | `MSG|OUT|STATE|TARGET|ID|TRIES|3|CODE` |
| `msg inbox` | `MSG|INBOX|BEGIN|COUNT`, ardından `MSG|IN|N|FROM|ID|TEXT` satırları, sonra `MSG|INBOX|END` |
| `msg send TA1FM-10 SELAM KANKA` | `MSG|QUEUED|TA1FM-10|0001` veya `MSG|ERR|...` |
| `msg cancel` | `MSG|CANCELLED` veya `MSG|ERR|NO_PENDING_MESSAGE` |

Cihaz gelen kutuya mesaj eklediğinde `MSG|NEW|COUNT|UNREAD` bildirimi gönderir. Komutlar `\n` ile biter. `msg status` bilgileri bağlantı açıkken yaklaşık her 4 saniyede bir yayınlanır.

## Kurallar / sınırlar

- Metin uzunluğu **1–62 yazdırılabilir ASCII**; özel Türkçe karakter kullanılmaz. `|`, `~`, `{` yasak.
- TRACKER modu, geçerli gönderici ve alıcı callsign, LoRa radyo hazır olmalıdır. GPS FIX mesaj için gerekmez.
- İleti `{0001` benzeri 4 haneli APRS ID ile gider, ACK bekler; **60 saniye aralıkla toplam en fazla 3 RF denemesi** yapar, 10 saniyelik mevcut ortak RF TX aralığı uygulanır.
- TA1FM-10 iGate'in kendisine gelen RF mesajlara otomatik RF ACK verdiği henüz doğrulanmamıştır. Bu durumda **ACK_TIMEOUT** beklenebilir; mesajın iGate/aprs.fi'de görülmesi ile ACK aynı şey değildir.
- Yalnızca aktif/son gönderilen mesajın durumu ve 10 gelen mesaj **RAM** üzerinde; cihaz yeniden başladığında geçmiş temizlenir. Geçmişin kalıcı olması bu BETA'nın kapsamında değildir.
- BLE NUS **eşleştirmesiz**: yakındaki bağlantı kurabilen başka istemciler de RF mesaj gönderimi komutunu kullanabilir. Kullanıcının isteğiyle **ayrı TX izin ekranı ve geçici ARM sistemi eklenmemiştir**. Cihazın Bluetooth'u gerekmediğinde kapatmak erişimi sınırlar.
- Bu sürümde **hazır mesajlar YOK**. Önce serbest yazılan mesajların gönderimi/alımı ve ACK işi tamamlanacaktır.

**ÖNEMLİ:** Bu dosyalar kaynak kodu düzeyinde kontrol edilmiştir; **Arduino IDE derleme ve gerçek BLE cihaz testi henüz yapılmadığı için işlev garantisi veremeyiz.**

73 de TA1FM
