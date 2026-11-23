# TA1FM nrf52 Tracker v1.3.0 BETA1 — Test planı

**Kaynak firmware:** [BETA1 .ino](TA1FM_nrf52_Tracker_v1.3.0_BETA1.ino)  
**BETA panel:** [TA1FM v1.3.0 BETA1 HTML](https://ta1fm.github.io/TA1FM-nrf52-Aprs-Tracker-Configurator/BETA/TA1FM_Tracker_Control_Panel_v1.3.0_BETA1.html)

Bu sürümde OLED **DFU MODE** (iki ayrı uzun basış ve kısa basışla iptal), BLE panelden **tek onay** ile DFU ARM/GO, **CAR /> / MOTORCYCLE /< / WALKER /[**, yalnızca ilk kurulumda **varsayılan SX1268** ve OLED **SYSTEM STATUS** (LoRa/GPS/BLE/Direct Flash A/B/BAT) vardır.

### Kontrol edilecekler

1. Arduino IDE'de doğru nRF52840 kart/variant ayarlarıyla kaynak **derlenmeli**. Bu kaynak burada derlenip karta yüklenerek test edilmedi.
2. Seri monitörde `V1.3.0 BETA1` görünmeli. Geçerli Direct Flash ayarıyla çalışan cihazda kayıtlı **SX1262/SX1268 değişmemeli**.
3. OLED menüsünde **SYSTEM STATUS** görünmeli, ekrandan uzun basışla dönülebilmeli. 'FLASH A/B' yalnızca aktif ayar slotunu gösterir.
4. **POS SYMBOL** sıralaması CAR / MOTORCYCLE / WALKER olmalı; APRS motosiklet kodu `/<` olmalı. Kullanıcı sembol kaydetmedikçe mevcut APRS sembolü korunmalı.
5. **DFU MODE** seçimi ilk uzun basışla onay ekranına gelmeli, kısa basış iptal etmeli; ikinci uzun basış yaklaşık 2,2 sn sonra `AdaDFU` bootloader'a geçmeli.
6. Web BETA panelinde **Kaydetme açık** ayarı etkinleştirilince BLE DFU düğmesi yalnızca **bir kez** onay istemeli, `DFU|ARMED|15` bekledikten sonra otomatik `dfu go` göndermeli. Yanıt `DFU|STARTING|AdaDFU` olmalı.
7. GPS, LoRa RX/TX, APRS mesajları, OLED standby ve kayıtlı Direct Flash ayarları regresyon testinden geçmeli.

**Bu pakette yok:** Tarayıcı içinden doğrudan Legacy DFU ZIP yükleme.  
**Önemli:** OLED DFU, mevcut `enterOTADfu()` mekanizmasını kullanır. Daha önce pille kullanımda gözlenen düzensiz bootloader girişinin kök nedeni henüz çözülmedi.  
**STABLE:** v1.2.0 STABLE REBUILD1 dosyaları korunmuştur. BETA henüz doğrulanmış STABLE değildir. UF2/USB kurtarma planı hazır olsun.
