# Configuration Eraser (third-party / üçüncü taraf)

**Original author:** Ricardo Guzman — CA2RXU  
**Official source:** https://github.com/richonguzman/nrf52840-web-config  
**Original UF2 download:** https://raw.githubusercontent.com/richonguzman/nrf52840-web-config/main/firmwares/NRF52840_Delete_Config.uf2  
**Original repository license:** GPL-3.0

Upstream's firmware listing describes this as **Erase/Delete All Configuration**.

## TA1FM test / test notu

TA1FM reports that `NRF52840_Delete_Config.uf2` cleared configuration
on the TA2DMX MiniMesh Lite NRF board; the UF2 bootloader continued to
work and the Tracker firmware could be installed afterwards.

**Limits / Sınırlar:** The exact flash pages erased at runtime have not
been audited. Full flash erasure, universal compatibility, and recovery
of a corrupted bootloader are **not** claimed.

## Kullanım / Usage

1. Önce ayarlarını yedekle / Back up your settings.
2. Uyumlu MiniMesh Lite NRF kartı iki kez RESET ile UF2 moduna al.
3. Orijinal bağlantıdan dosyayı indirip UF2 sürücüsüne kopyala.
4. Ardından TA1FM Tracker UF2 firmware dosyasını yükle.
5. Web Configurator ile ayarları tekrar yapılandır.

**Normal firmware güncellemelerinde silici kullanmak gerekli değildir.**

## Redistribution / Yeniden dağıtım

The original repository includes a GPLv3 license but the corresponding
eraser source code was not found there. This documentation **links** to
the original file instead of redistributing the compiled binary until
source-code distribution requirements can be verified with CA2RXU.

TA1FM firmware: https://github.com/TA1FM/TA1FM-nrf52-Aprs-Tracker
