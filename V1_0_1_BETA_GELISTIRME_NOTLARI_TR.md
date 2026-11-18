# TA1FM nRF52 Tracker — V1.0.1 BETA Geliştirme Notları

Tarih: 18 Kasım 2026. Durum: Plan; uygulama yapılmadı. V1.0.0 STABLE korunacak.

## 1. İki kanallı Base91 APRS telemetri
- A1: Pil voltajı (V).
- A2: Sıcaklık (°C): yalnızca cihazda geçerli sensör verisi varsa.
- Ölçümleri normal APRS konum paketine kısa Base91 telemetri biçiminde gömme; her ölçüm için ayrı RF paketi oluşturma.
- Konum sonundaki uzun Batt=... ve sıcaklık metinlerinin yerini daha kısa standart telemetri alabilir.
- Standart PARM, UNIT, EQNS, BITS tanımları; ölçekler, sıra sayacı ve aprs.fi grafik uyumluluğu gerçek testle doğrulanacak.
- Sensör bulunmayan cihazlarda uydurma sıcaklık gönderilmeyecek; ikinci kanalın doğru davranışı tasarlanacak.
- Telemetri sıklığı ve Aç/Kapat ayarı sonraki tasarımda değerlendirilecek.
- Mevcut Comment metni korunacak; Base91 telemetri ve normal Comment aynı paket alanında sorunsuz biçimde birleştirilecek.

## 2. RF paket gönderim sırası
1. APRS mesaj ACK ve zaman açısından kritik mesaj işlemleri: en yüksek öncelik.
2. Zamanı gelen Status: normal Smart Beacon konumunu beklemeyecek; ilk uygun TX fırsatında öne alınacak.
3. Ayrı telemetri RF paketi gerekirse zamanı geldiğinde normal konuma göre öncelikli olacak; konuma gömülü Base91 için ayrı TX olmayacak.
4. Normal periyodik Smart Beacon konumu: diğer önemli gönderimler yoksa; gerektiğinde bu tur atlanacak, eski konum kuyruğa biriktirilmeyecek.
5. Viraj, hareket başlangıcı ve park geçişi gibi kritik konum olaylarının önemi ayrıca korunacak.
6. Comment: ayarlanan aralıkta uygun konum paketine eklenecek; ayrı gönderim olmayacak.
7. Mevcut ortak en az 10 saniye RF TX koruması korunacak. Sırası gelen paket zorunlu güvenlik aralığından sonra gönderim girişimi yapacak; sürekli ertelenmeyecek.
8. Başarısız gönderim için sınırlı yeniden deneme ve taze GPS verisi politikası olacak; TX OK iGate tarafından alındı anlamına gelmez.
9. DUE / WAIT GUARD / TX START / TX OK / TX ERROR kayıtları ile tanılama yapılacak.

Örnek: Hızlı giderken Status zamanı geldiyse normal 10-15 saniyelik konum atlanabilir; Status gönderilir, sonraki konum en güncel GPS verisiyle çıkar.

## 3. Diğer V1.0.1 BETA konuları
- APRS konum paketine GPS yön/hız (ccc/sss: derece ve knot) ekleme; aprs.fi'de doğrulama.
- Comment 15/30/60 dakika zamanlamasını saha gözlemleriyle inceleme.
- Status zamanlamasını ve Smart Beacon ile etkileşimini inceleme.
- Aktif buzzer: yalnızca cihaza adreslenmiş yeni APRS mesajında çift bip; web panelinden Aç/Kapat.
- Hazır APRS mesajları: web panelinde | ayırıcıyla giriş, kalıcı cihaz hafızasına kayıt, listeden seçim; OLED kullanımı daha sonra.

## 4. Daha sonraki değerlendirmeler
- OLED üzerinden hazır mesaj gönderimi.
- Gelen APRS mesajlarını kalıcı hafızada tutma.
- TA1FM-10 iGate için besleme voltajı ve varsa sıcaklık sensörü telemetrisi, mümkünse APRS-IS üzerinden.

Bu belge yalnızca nottur. Firmware ve çalışan web paneli değiştirilmedi.
