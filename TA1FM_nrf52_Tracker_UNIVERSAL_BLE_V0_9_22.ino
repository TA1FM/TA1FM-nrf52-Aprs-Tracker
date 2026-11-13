/*
  TA1FM nrf52 Tracker - UNIVERSAL BLE SAVE SWITCH V0.9.22
  Temel: V0.9.14 SX1268; Direct Flash A/B ve CRC32 sistemi korunur.
  TEST: SX1262 / SX1268 secimi; mevcut GPS/OLED/APRS islevleri korunur.
  BLE: 18 bayt NUS, ayar okuma, sifresiz kaydetme anahtari, tek Direct Flash commit.
  APRS mesajlasma henuz YOKTUR.
  V0.9.14: Web uzerinden OLED SH1106/SSD1306 onizleme, onaylama,
  iptal, uyandirma ve ekran suresi ayarlari (Direct Flash kaydi).
  20 saniyelik onizlemede onay verilmezse onceki surucuye doner.
  APRS TX/RX, GPS, Smart Beacon ve A/B flash kayit sistemi korunur.
  ONEMLI: Sadece Direct Flash kullanilir; eski dosya sistemi acilmaz.
  MiniMesh Lite NRF (TA2DMX), nRF52840

  V0.6 bazinda: GPS, pil, BME280, menu, OLED kurtarma, LoRa RX.
  YENI: MANUEL KONUM ve WX APRS TX, GPS FIX ve kayitli CALLSIGN zorunlu.
  YENI: USB seri komutuyla cagrı isareti / SSID kaydi.
  Gecersiz veya ayarlanmamis cagrida TX KAPALI.

  GEREKLI KUTUPHANELER:
  Adafruit TinyUSB (Adafruit nRF52 core), Adafruit GFX,
  Adafruit SH110X, Adafruit SSD1306,
  TinyGPSPlus (Mikal Hart), Adafruit BME280 Library,
  Adafruit Unified Sensor (BME280 bagimliligi), RadioLib 7.8.1.

  Kart: TA1FM MiniMesh Lite NRF (TA2DMX), SoftDevice S140 6.1.1.
  Mevcut calisan V0.4 dosyalarini yedekleyin.

  V0.9: Position SMART/FIXED, independent WX and STATUS timers,
  COMMENT interval, serial text editing, OLED last APRS heard station.
  AUTO TX enabled on first valid callsign, manual a off is persistent
  state is respected. Initial position on GPS fix; WX independent of park.
  Park sembolu korunur; WX bagimsizdir. LoRa RF TX korumasi 15 saniye.
  Callsign, OLED, smart ve RF ayarlari Direct Flash A/B kaydinda korunur.
  Tum ayarlar Direct Flash A/B tek kaydinda saklanir.
  Test surumu: bu degisiklikler henuz kullanicinin cihazinda derlenip denenmedi.

*/

#include <Adafruit_TinyUSB.h>
#include <bluefruit.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// Adafruit nRF52 cekirdegindeki dusuk seviye flash_nrf5x surucusu,
// InternalFileSystem kutuphanesinin icindedir. Bu baslik, Arduino IDE'nin
// kutuphaneyi derlemeye eklemesini saglar. LittleFS ACILMAZ/KULLANILMAZ;
// kodda InternalFS.begin() veya format() cagrisi bulunmaz.
#include <InternalFileSystem.h>
extern "C" {
#include <flash/flash_nrf5x.h>
}
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <RadioLib.h>

// BLE Nordic UART Service (NUS); settings writes require physical unlock.
BLEUart ta1fmBleUart;

// Defined before the first sketch function so Arduino auto-prototypes
// can resolve DirectRecord references (directSnapshot/directValid).
struct DirectRecord {
  uint32_t magic;
  uint32_t schema;
  uint32_t seq;
  char callsign[12];
  uint8_t display;
  int8_t utc;
  uint8_t sleepMode;
  int8_t power;
  uint8_t autoMode, wxMin, parkMin;
  char symbolTable, symbol;
  uint8_t positionMode, fixedMin, statusMin, commentMin;
  uint16_t intervals[6];
  uint8_t firstMotion, battery;
  char comment[63];
  char status[63];
  uint8_t reserved[3];
  uint32_t crc;
};

static const uint8_t OLED_ADDR = 0x3C;
static const uint8_t OLED_W = 128;
static const uint8_t OLED_H = 64;
static const uint8_t BTN_PIN = 32;       // P1.00
static const uint8_t GPS_PWR_PIN = 24;   // P0.24 GPS_EN
static const uint8_t VBAT_PIN = 31;      // P0.31
static const uint8_t GPS_PWR_ACTIVE_LEVEL = HIGH;  // V0.2'de calisan seviye
static const uint32_t GPS_BAUD = 9600;
static const uint32_t LONG_PRESS_MS = 1200;
static const uint32_t BOOT_RECOVERY_MS = 3000;
static const uint32_t OLED_CONFIRM_MS = 10000;
static const uint32_t DEBOUNCE_MS = 35;
static const uint32_t MENU_TIMEOUT_MS = 30000;
// Ortak TX korumasi: Tum RF paketleri arasinda en az 15 saniye.
static const uint32_t POS_TX_GAP_MS = 15000UL;
static const uint32_t AUX_TX_GAP_MS = 15000UL;

// Meshtastic nrf52_promicro_diy_tcxo and tested MiniMesh variant.
static const int LORA_CS_PIN = 45;     // P1.13
static const int LORA_DIO1_PIN = 10;   // P0.10
static const int LORA_RST_PIN = 9;     // P0.09
static const int LORA_BUSY_PIN = 29;   // P0.29
static const int LORA_RXEN_PIN = 17;   // P0.17
// SPI map from variant: SCK=43, MISO=2, MOSI=47.
static const float LORA_FREQ_MHZ = 433.775f;
static const float LORA_BW_KHZ = 125.0f;
static const uint8_t LORA_SF = 12;
static const uint8_t LORA_CR = 5;    // 4/5
static const uint8_t LORA_SYNC = 0x12;
static const uint8_t APRS_HEADER[3] = {0x3C, 0xFF, 0x01};
int8_t loraTxPowerDbm = 10; // V0.9.4: saved TX level; SX1268 requested TX level 10..22 dBm
uint32_t lastTxAt = 0;
bool everTransmitted = false;
uint32_t txSuccesses = 0;
uint32_t wxSuccesses = 0;
int16_t txLastCode = 0;
char stationCall[12] = "NOCALL";
bool callsignValid = false;

// RF AUTO SETTINGS - stored separately from older V0.4 / V0.6 records.
// 0=OFF, 1=PARKED WX, 2=ALWAYS WX
uint8_t autoWxMode = 0;   // TX opt-in: OFF on first install
uint8_t wxIntervalMin = 60; // first-install default; saved preferences win
uint8_t parkWaitMin = 3;
uint8_t positionSymbolTable = '/';
char positionSymbol = '>'; // car
uint8_t parkedSymbolTable = '\\'; // APRS parked default \\P
char parkedSymbol = 'P';
bool autoSettingsStorageOK = false;
uint32_t autoEnabledAt = 0;
uint32_t lastAutoAttemptAt = 0;
uint32_t lastAutoPosAt = 0;
uint32_t lastAutoWxAt = 0;
uint32_t lastMotionSeenAt = 0;
uint32_t lastMotionSampleAt = 0;
uint8_t movingSamples = 0;
bool movingConfirmed = false;
bool parkedConfirmed = false;
static const uint32_t AUTO_POS_INTERVAL_MS = 180000UL;
// V0.9: 0-5 km/h durma adayi; 5-8 arasi belirsiz, durum ve sayac korunur.
// Hareket ancak 8+ km/h uc ARDISIK gecerli GPS hiz ornegiyle dogrulanir.
static const float MOVE_SPEED_KMH = 8.0f;
static const float STOP_SPEED_KMH = 5.0f;


// Tek firmware'de iki farkli RadyoLib surucusu bulunur.
// Ayni pinler kullanilir; secilen tek surucu etkinlestirilir.
static const uint8_t CHIP_SX1262 = 2;
static const uint8_t CHIP_SX1268 = 8;
uint8_t loraChip = CHIP_SX1268; // Eski V0.9.14 Direct Flash kayitlari icin uyumlu varsayilan
SX1262 sx1262Radio = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);
SX1268 sx1268Radio = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);
SX126x* radio = nullptr;
const char* loraChipName() { return loraChip == CHIP_SX1262 ? "SX1262" : "SX1268"; }
volatile bool loraRxFlag = false;
bool loraReady = false;
int16_t loraError = 0;
uint32_t loraPackets = 0;
uint32_t loraAprsPackets = 0;
float lastRssi = 0;
float lastSnr = 0;
uint32_t lastLoraRxAt = 0;
char lastHeardCall[12] = "---";
void onLoraRx() { loraRxFlag = true; }

void initLora() {
  loraReady = false;
  Serial.print("[LORA] Selected: "); Serial.print(loraChipName());
  Serial.print(" TX target: "); Serial.print(loraTxPowerDbm); Serial.println(" dBm");
  SPI.begin();
  // SX1262 / SX1268: ayri surucu sinifi, ortak fiziksel ozellikler.
  // begin(frequency,bw,sf,cr,syncWord,power,preamble,tcxoVoltage)
  if (loraChip == CHIP_SX1262) {
    radio = &sx1262Radio;
    loraError = sx1262Radio.begin(LORA_FREQ_MHZ, LORA_BW_KHZ, LORA_SF, LORA_CR,
                                   LORA_SYNC, loraTxPowerDbm, 8, 1.8f);
  } else {
    radio = &sx1268Radio;
    loraError = sx1268Radio.begin(LORA_FREQ_MHZ, LORA_BW_KHZ, LORA_SF, LORA_CR,
                                   LORA_SYNC, loraTxPowerDbm, 8, 1.8f);
  }
  if (loraError != RADIOLIB_ERR_NONE) {
    Serial.print("[LORA] begin ERR="); Serial.println(loraError);
    return;
  }
  // These settings were confirmed in the running Meshtastic startup log.
  int16_t sw = radio->setDio2AsRfSwitch();
  radio->setRfSwitchPins(LORA_RXEN_PIN, RADIOLIB_NC);
  int16_t crc = radio->setCRC(2);
  Serial.print("[LORA] DIO2 switch="); Serial.print(sw);
  Serial.print(" CRC="); Serial.println(crc);
  if (sw != RADIOLIB_ERR_NONE || crc != RADIOLIB_ERR_NONE) {
    loraError = (sw != RADIOLIB_ERR_NONE) ? sw : crc;
    return;
  }
  radio->setDio1Action(onLoraRx);
  loraError = radio->startReceive();
  loraReady = (loraError == RADIOLIB_ERR_NONE);
  Serial.print("[LORA] RX ");
  Serial.println(loraReady ? "OK 433.775 SF12 BW125 CR4/5" : "START FAILED");
  if (!loraReady) { Serial.print("[LORA] code="); Serial.println(loraError); }
}

void pollLora() {
  if (!loraReady || !loraRxFlag) return;
  loraRxFlag = false;
  size_t packetLen = radio->getPacketLength();
  if (packetLen > 255) packetLen = 255;
  uint8_t data[256] = {};
  int16_t state = radio->readData(data, packetLen);
  if (state == RADIOLIB_ERR_NONE) {
    loraPackets++;
    lastLoraRxAt = millis();
    lastRssi = radio->getRSSI();
    lastSnr = radio->getSNR();
    const bool aprs = packetLen >= 3 && memcmp(data, APRS_HEADER, 3) == 0;
    if (aprs) {
      loraAprsPackets++;
      // Only the source field before '>' is shown. Reject corrupted frames.
      char call[12] = {};
      size_t j = 0;
      for (size_t i = 3; i < packetLen && j < sizeof(call) - 1; ++i) {
        char c = (char)data[i];
        if (c == '>') break;
        if (c < 33 || c > 126) { j = 0; break; }
        call[j++] = (char)toupper((unsigned char)c);
      }
      // Do not display arbitrary RF noise as an APRS callsign.
      size_t base = 0;
      bool ok = (j >= 3 && j <= 9 && 3+j < packetLen && data[3+j] == '>');
      while (base < j && call[base] != '-') {
        if (!((call[base] >= 'A' && call[base] <= 'Z') ||
              (call[base] >= '0' && call[base] <= '9'))) ok = false;
        ++base;
      }
      if (base < 3 || base > 6) ok = false;
      if (base < j) {
        size_t digits = j - base - 1;
        if (digits < 1 || digits > 2) ok = false;
        for (size_t k = base + 1; k < j; ++k)
          if (call[k] < '0' || call[k] > '9') ok = false;
        if (ok && atoi(call + base + 1) > 15) ok = false;
      }
      if (ok) { strncpy(lastHeardCall, call, sizeof(lastHeardCall) - 1); }
    }
    Serial.print("[LORA RX] bytes="); Serial.print(packetLen);
    Serial.print(" RSSI="); Serial.print(lastRssi, 1);
    Serial.print(" SNR="); Serial.print(lastSnr, 1);
    Serial.print(aprs ? " APRS: " : " OTHER: ");
    size_t start = aprs ? 3 : 0;
    for (size_t i = start; i < packetLen; ++i) {
      char ch = (char)data[i];
      Serial.write((ch >= 32 && ch <= 126) ? ch : '.');
    }
    Serial.println();
  } else {
    Serial.print("[LORA RX] read error="); Serial.println(state);
  }
  loraError = radio->startReceive();
  if (loraError != RADIOLIB_ERR_NONE) {
    loraReady = false;
    Serial.print("[LORA] RX restart ERR="); Serial.println(loraError);
  }
}


Adafruit_SH1106G sh(OLED_W, OLED_H, &Wire, -1);
Adafruit_SSD1306 ss(OLED_W, OLED_H, &Wire, -1);
Adafruit_GFX *gfx = nullptr;
Adafruit_BME280 bme;
TinyGPSPlus gps;


uint8_t oledType = 1; // 1: SH1106, 2: SSD1306
bool oledReady = false;
int8_t utcOffset = 3; // Menu: UTC veya UTC+3
float batteryV = 0.0f;

bool bmeReady = false;
uint8_t bmeAddress = 0;
float tempC = NAN, humidity = NAN, pressureHPa = NAN;
bool bmeValuesOK = false;
// Last valid BME280 reading; declared before APRS WX functions.
uint32_t lastBmeGoodAt = 0;

// PAGE_MAIN / PAGE_MENU / PAGE_BME / PAGE_CLOCK / PAGE_GPS / PAGE_OLED
// Saat ve ekran secimi alt sayfalarda kisa basla degisir,
// uzun basla onaylanarak ana menuye donulur.
enum Page { PAGE_MAIN, PAGE_MENU, PAGE_BME, PAGE_CLOCK, PAGE_GPS, PAGE_LORA, PAGE_INFO, PAGE_OLED, PAGE_OLED_CONFIRM, PAGE_AUTO, PAGE_SYMBOL, PAGE_SLEEP, PAGE_TX_POWER };
Page page = PAGE_MAIN;
uint8_t menuIndex = 0;
int8_t pendingUtc = 3;
uint8_t pendingOledType = 1;
uint8_t pendingAutoWxMode = 0;
uint8_t pendingSymbolIndex = 0;
// OLED standby: 0=always ON, 1=OFF, 2=15s, 3=30s, 4=60s, 5=120s, 6=300s.
uint8_t oledSleepMode = 3;
uint8_t pendingSleepMode = 3;
int8_t pendingTxPowerDbm = 10;
bool oledSleeping = false;
uint32_t oledLastWakeAt = 0;
bool consumeWakeClick = false;
static const uint32_t OLED_SLEEP_TIMES[] = {0, 0, 15000UL, 30000UL, 60000UL, 120000UL, 300000UL};
const char* sleepModeName(uint8_t i) {
  static const char* const names[] = {"ALWAYS ON", "ALWAYS OFF", "15 SEC", "30 SEC", "1 MIN", "2 MIN", "5 MIN"};
  return names[i < 7 ? i : 3];
}

uint8_t previousOledType = 1;
uint32_t oledConfirmStarted = 0;
// Fiziksel butonla onay suresi 10 sn, web onizlemesi 20 sn.
uint32_t oledConfirmWindowMs = OLED_CONFIRM_MS;
bool oledPreviewFromUsb = false;

bool configStorageOK = false;
// Six adjustable intervals at fixed speed breakpoints 7,10,20,49,60,90 km/h.
static const uint8_t SMART_SPEED_KMH[6] = {7, 10, 20, 49, 60, 90};
uint16_t smartIntervalsSec[6] = {600, 90, 45, 30, 20, 15};
bool beaconOnFirstMove = true;
bool beaconBatteryVoltage = true;

// APRS beacon ayarlari Direct Flash A/B icerisindedir.
uint8_t positionMode = 1; // 0=FIXED, 1=SMART (default)
uint8_t fixedPositionMin = 5;
uint8_t statusIntervalMin = 15;  // 0=OFF, 15/30/60
uint8_t commentIntervalMin = 15; // 0=OFF, 15/30/60, how often in POS packet
char commentText[63] = "TA1FM nrf52 Tracker";
char statusText[63] = "TA1FM LoRa APRS Tracker 433.775 MHz";
uint32_t lastStatusTxAt = 0, lastCommentTxAt = 0;
uint32_t lastPositionTxAt = 0, lastPositionAttemptAt = 0;
uint32_t lastWxAttemptAt = 0, lastStatusAttemptAt = 0;
uint32_t positionDueAt = 0, wxDueAt = 0, statusDueAt = 0;
bool firstPositionDone = false, lastPosWasParked = false;
bool startupStatusPending = false;
float sentCourse = NAN;
uint32_t lastSmartMotionSampleAt = 0;

bool validText(const char *s) {
  size_t n = strnlen(s, 63);
  if (n == 0 || n >= 63) return false;
  for (size_t i=0; i<n; ++i) {
    unsigned char c = (unsigned char)s[i];
    if (c < 32 || c > 126 || c == '|' || c == '~') return false;
  }
  return true;
}
// Direct Flash A/B kaydi: V0.9.14-V0.9.16 ile AYNI adresler, bicim ve CRC.
// LittleFS baslatilmaz; sadece dogrulanmis A/B kayitlari okunur.
// Eski ayarlarin kaybolmamasi icin flash init otomatik cagirilmaz.
// nRF52840/S140: bootloader 0xF4000'de baslar; bu alan kart varyantina ozeldir.
static constexpr uint32_t DF_PAGE_BYTES = 4096UL;
static constexpr uint32_t DF_PAGE_A = 0xF2000UL;
static constexpr uint32_t DF_PAGE_B = 0xF3000UL;
static constexpr uint32_t DF_MAGIC = 0x54414644UL; // TAFD
static constexpr uint32_t DF_SCHEMA = 1UL;
static bool directActive = false;
static uint32_t directSeq = 0;
static uint32_t directSlot = 0;

// Plain data record, zero-initialized before snapshots to stabilize CRC.

static_assert(sizeof(DirectRecord) % 4 == 0, "DirectRecord must be word aligned");
static_assert(sizeof(DirectRecord) <= DF_PAGE_BYTES, "DirectRecord exceeds a flash page");

uint32_t directCrc(const void* data, size_t length) {
  const uint8_t* p = (const uint8_t*)data;
  uint32_t crc = 0xFFFFFFFFUL;
  for (size_t i=0;i<length;i++) {
    crc ^= p[i];
    for (uint8_t j=0;j<8;j++) crc=(crc>>1)^((crc&1U)?0xEDB88320UL:0);
  }
  return crc^0xFFFFFFFFUL;
}

void directSnapshot(DirectRecord& r) {
  memset(&r,0,sizeof(r));
  r.magic=DF_MAGIC; r.schema=DF_SCHEMA; r.seq=directSeq+1;
  strncpy(r.callsign,stationCall,sizeof(r.callsign)-1);
  r.display=oledType; r.utc=utcOffset; r.sleepMode=oledSleepMode;
  r.power=loraTxPowerDbm;
  r.autoMode=autoWxMode; r.wxMin=wxIntervalMin; r.parkMin=parkWaitMin;
  r.symbolTable=(char)positionSymbolTable; r.symbol=positionSymbol;
  r.positionMode=positionMode; r.fixedMin=fixedPositionMin;
  r.statusMin=statusIntervalMin; r.commentMin=commentIntervalMin;
  memcpy(r.intervals,smartIntervalsSec,sizeof(r.intervals));
  r.firstMotion=beaconOnFirstMove?1:0; r.battery=beaconBatteryVoltage?1:0;
  strncpy(r.comment,commentText,sizeof(r.comment)-1);
  strncpy(r.status,statusText,sizeof(r.status)-1);
  r.reserved[0] = loraChip; // 0: eski SX1268 kaydi; 2 veya 8: yeni secim
  r.reserved[1] = parkedSymbolTable;
  r.reserved[2] = (uint8_t)parkedSymbol;
  r.crc=directCrc(&r,offsetof(DirectRecord,crc));
}

bool directValid(uint32_t addr, DirectRecord& r) {
  if (addr != DF_PAGE_A && addr != DF_PAGE_B) return false;
  memcpy(&r,(const void*)addr,sizeof(r));
  if (r.magic!=DF_MAGIC || r.schema!=DF_SCHEMA || r.seq==0 ||
      r.crc!=directCrc(&r,offsetof(DirectRecord,crc))) return false;
  if (r.display<1 || r.display>2 || (r.utc!=0 && r.utc!=3) ||
      r.sleepMode>6 || r.power<1 || r.power>22 || r.autoMode>2 ||
      r.wxMin>60 || r.parkMin>5 || r.positionMode>1 ||
      r.callsign[11]!='\0' || r.comment[62]!='\0' || r.status[62]!='\0' ||
      (r.reserved[0]!=0 && r.reserved[0]!=CHIP_SX1262 && r.reserved[0]!=CHIP_SX1268) ||
      (r.reserved[1] && r.reserved[1]!='/' && r.reserved[1]!='\\') ||
      (r.reserved[2] && (r.reserved[2]<'!' || r.reserved[2]>'~'))) return false;
  return true;
}

bool validateCall(const char *input);
bool directLoad() {
  DirectRecord a={},b={};
  bool va=directValid(DF_PAGE_A,a), vb=directValid(DF_PAGE_B,b);
  if (!va && !vb) return false;
  bool useB = vb && (!va || (int32_t)(b.seq-a.seq)>0);
  const DirectRecord& r = useB?b:a;
  directSeq=r.seq; directSlot=useB?DF_PAGE_B:DF_PAGE_A;
  directActive=true;
  strncpy(stationCall,r.callsign,sizeof(stationCall)-1);
  stationCall[sizeof(stationCall)-1]=0;
  callsignValid=validateCall(stationCall);
  if (!callsignValid) strcpy(stationCall,"NOCALL");
  // Eski kayitlarda reserved[0] sifir: daima SX1268 olarak yorumlanir.
  loraChip = (r.reserved[0] == CHIP_SX1262) ? CHIP_SX1262 : CHIP_SX1268;
  oledType=r.display; utcOffset=r.utc; oledSleepMode=r.sleepMode;
  loraTxPowerDbm=r.power;
  autoWxMode=r.autoMode; wxIntervalMin=r.wxMin; parkWaitMin=r.parkMin;
  positionSymbolTable=(uint8_t)r.symbolTable; positionSymbol=r.symbol;
  parkedSymbolTable = r.reserved[1] ? r.reserved[1] : '\\';
  parkedSymbol = r.reserved[2] ? (char)r.reserved[2] : 'P';
  positionMode=r.positionMode; fixedPositionMin=r.fixedMin;
  statusIntervalMin=r.statusMin; commentIntervalMin=r.commentMin;
  memcpy(smartIntervalsSec,r.intervals,sizeof(smartIntervalsSec));
  beaconOnFirstMove=r.firstMotion!=0; beaconBatteryVoltage=r.battery!=0;
  memcpy(commentText,r.comment,sizeof(commentText));
  memcpy(statusText,r.status,sizeof(statusText));
  Serial.print("[DIRECT] Loaded A/B slot 0x");Serial.print(directSlot,HEX);
  Serial.print(" sequence=");Serial.println(directSeq);
  return true;
}

bool directWrite() {
  if (!directActive) {
    Serial.println("[DIRECT] Storage not enabled. Verify and initialize only if no valid record: flash init");
    return false;
  }
  // Cross-check the expected flash geometry before ANY destructive operation.
  if (NRF_FICR->CODEPAGESIZE!=DF_PAGE_BYTES ||
      NRF_FICR->CODESIZE * NRF_FICR->CODEPAGESIZE != 0x100000UL ||
      (DF_PAGE_A+DF_PAGE_BYTES)!=DF_PAGE_B ||
      DF_PAGE_B+DF_PAGE_BYTES!=0xF4000UL) {
    Serial.println("[DIRECT] Flash geometry mismatch; write blocked");return false;
  }
  DirectRecord next={}; directSnapshot(next);
  uint32_t dest=(directSlot==DF_PAGE_A)?DF_PAGE_B:DF_PAGE_A;
  if (directSlot==0) dest=DF_PAGE_A;
  // Adafruit flash_nrf5x layer handles SoftDevice asynchronous writes,
  // flash page erasure, and cache flush. NEVER use direct NRF_NVMC here.
  int wrote=flash_nrf5x_write(dest,&next,sizeof(next));
  flash_nrf5x_flush();
  DirectRecord readback={};
  bool ok = wrote==(int)sizeof(next) && directValid(dest,readback) &&
            memcmp(&next,&readback,sizeof(next))==0;
  if (!ok) {
    Serial.println("[DIRECT] VERIFY FAILED; old slot retained");return false;
  }
  directSlot=dest; directSeq=next.seq;
  Serial.print("[DIRECT] SAVED slot 0x");Serial.print(dest,HEX);
  Serial.print(" seq=");Serial.println(directSeq);
  return true;
}

void directInfo() {
  DirectRecord a={},b={};
  Serial.print("[DIRECT] mode="); Serial.println(directActive?"ACTIVE":"NOT INITIALIZED");
  Serial.print("[DIRECT] slot A 0xF2000 valid=");Serial.println(directValid(DF_PAGE_A,a));
  Serial.print("[DIRECT] slot B 0xF3000 valid=");Serial.println(directValid(DF_PAGE_B,b));
  Serial.print("[DIRECT] seq=");Serial.println(directSeq);
  Serial.print("[DIRECT] chip=");Serial.println(loraChipName());
  Serial.println("[DIRECT] Direct Flash only; flash init is for EMPTY devices only");
}

// Smart beacon ayarlari Direct Flash A/B kaydindadir.
bool validSmartIntervals(const uint16_t *values) {
  if (values[0] < 15 || values[0] > 3600) return false;
  for (uint8_t i=1; i<6; ++i) {
    if (values[i] < 10 || values[i] > 3600 || values[i] > values[i-1]) return false;
  }
  return true;
}
bool saveSmartSettings() {
  return directWrite();
}

uint32_t smartRateMs(float speed) {
  if (!isfinite(speed) || speed<=SMART_SPEED_KMH[0])
    return (uint32_t)smartIntervalsSec[0]*1000UL;
  for (uint8_t i=1; i<6; ++i) {
    if (speed<=SMART_SPEED_KMH[i]) {
      const float portion=(speed-SMART_SPEED_KMH[i-1]) /
                          (float)(SMART_SPEED_KMH[i]-SMART_SPEED_KMH[i-1]);
      const float seconds=smartIntervalsSec[i-1] +
                      (smartIntervalsSec[i]-smartIntervalsSec[i-1])*portion;
      return (uint32_t)(seconds*1000.0f+0.5f);
    }
  }
  return (uint32_t)smartIntervalsSec[5]*1000UL;
}
// TX gucu Direct Flash A/B kaydindan okunur ve ayni kayda yazilir.
bool validTxPower(int val) {
  return val >= 1 && val <= 22;
}
bool saveTxPower() {
  return directWrite();
}

void changeTxPower(int8_t desired) {
  if (!validTxPower(desired)) { Serial.println("Usage: power 1..22"); return; }
  if (!loraReady) { Serial.println("[TXPOWER] Radio not ready; no change"); return; }
  if (desired == loraTxPowerDbm) {
    Serial.print("[TXPOWER] already "); Serial.print(loraTxPowerDbm); Serial.println(" dBm"); return;
  }
  const int8_t previous = loraTxPowerDbm;
  radio->standby();
  const int16_t result = radio->setOutputPower(desired);
  if (result != RADIOLIB_ERR_NONE) {
    Serial.print("[TXPOWER] Radio rejected level, error "); Serial.println(result);
    radio->startReceive();
    return;
  }
  const int16_t rxResult = radio->startReceive();
  if (rxResult != RADIOLIB_ERR_NONE) {
    Serial.print("[TXPOWER] RX restore failed: "); Serial.println(rxResult);
    radio->standby(); radio->setOutputPower(previous);
    loraReady = radio->startReceive() == RADIOLIB_ERR_NONE;
    return;
  }
  loraRxFlag = false;
  loraTxPowerDbm = desired;
  if (!saveTxPower()) {
    Serial.println("[TXPOWER] SAVE FAILED; restoring previous power");
    radio->standby(); radio->setOutputPower(previous);
    loraReady = radio->startReceive() == RADIOLIB_ERR_NONE;
    loraTxPowerDbm = previous;
    return;
  }
  Serial.print("[TXPOWER] saved: "); Serial.print(loraTxPowerDbm); Serial.println(" dBm");
}

bool saveBeaconSettings() {
  return directWrite();
}

bool saveAutoSettings() {
  return directWrite();
}

void resetAutoTimers() {
  autoEnabledAt = millis();
  lastAutoAttemptAt = 0;
  lastAutoPosAt = 0;
  lastAutoWxAt = 0;
  lastPositionTxAt = 0;
  lastStatusTxAt = 0;
  lastCommentTxAt = 0;
  positionDueAt = 0;
  firstPositionDone = false;
  startupStatusPending = false;
  lastPosWasParked = false;
  sentCourse = NAN;
  wxDueAt = wxIntervalMin ? autoEnabledAt + (uint32_t)wxIntervalMin*60000UL : 0;
  statusDueAt = statusIntervalMin ? autoEnabledAt + (uint32_t)statusIntervalMin*60000UL : 0;
  lastPositionAttemptAt = lastWxAttemptAt = lastStatusAttemptAt = 0;
  lastMotionSeenAt = 0;
  lastMotionSampleAt = 0;
  movingSamples = 0;
  movingConfirmed = false;
  parkedConfirmed = false;
}
const char *modeName(uint8_t m) {
  return m == 0 ? "OFF" : "ON";
}
const char *symbolName(uint8_t i) {
  if (i == 0) return "CAR />";
  if (i == 1) return "WALKER /[";
  return "HOUSE /-";
}
uint8_t symbolIndex() {
  if (positionSymbolTable == '/' && positionSymbol == '[') return 1;
  if (positionSymbolTable == '/' && positionSymbol == '-') return 2;
  return 0;
}
void selectSymbol(uint8_t i) {
  positionSymbolTable = '/';
  positionSymbol = (i == 1) ? '[' : ((i == 2) ? '-' : '>');
}

void loadConfig() {
  // Sadece A/B Direct Flash kaydi kullanilir; dosya sistemi HIC acilmaz.
  configStorageOK = directLoad();
  if (configStorageOK) {
    Serial.println("[DIRECT] A/B READY; LittleFS tamamen devre disi");
  } else {
    // Ilk kurulum veya kayit hatasinda HICBIR flash sayfasina otomatik yazmayiz.
    // Kullanici chip secimini RAM'de belirleyip flash init ile acikca onaylamali.
    callsignValid = false;
    strcpy(stationCall, "NOCALL");
    Serial.println("[DIRECT] NO VALID RECORD! TX disabled (NOCALL).");
    Serial.println("[DIRECT] Once chip secimi, sonra sadece onayli 'flash init'.");
  }
}

bool saveConfig() {
  const bool ok = directWrite();
  Serial.println(ok ? "Config saved" : "Config save FAILED");
  return ok;
}

bool saveSleepMode() {
  const bool ok = directWrite();
  Serial.println(ok ? "[OLED] sleep saved" : "[OLED] sleep save FAILED");
  return ok;
}

void oledPower(bool on) {
  if (!oledReady) return;
  if (oledType == 1) sh.oled_command(on ? SH110X_DISPLAYON : SH110X_DISPLAYOFF);
  else ss.ssd1306_command(on ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
  oledSleeping = !on;
}
void wakeOled() {
  oledLastWakeAt = millis();
  if (oledSleeping) oledPower(true);
}
void pollOledStandby(uint32_t now) {
  if (!oledReady || oledSleeping || page != PAGE_MAIN || oledSleepMode == 0) return;
  const uint32_t waitMs = oledSleepMode == 1 ? 15000UL : OLED_SLEEP_TIMES[oledSleepMode];
  if (now - oledLastWakeAt >= waitMs) oledPower(false);
}
void showMessage(const char *txt, uint32_t duration = 1600);
bool gpsFix();
// Cagri isareti Direct Flash A/B kaydinda saklanir. NOCALL ile RF TX yok.
// Plain APRS/AX.25 callsign portion 1-6 ASCII alphanumeric, optional SSID 0..15.
// Rejects blanks, punctuation and malformed SSID; in particular NOCALL.
bool validateCall(const char *input) {
  if (!input) return false;
  const size_t len = strlen(input);
  if (len < 3 || len > 9 || strcmp(input, "NOCALL") == 0) return false;
  size_t baseLen = 0;
  while (input[baseLen] && input[baseLen] != '-') {
    const char c = input[baseLen];
    if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) return false;
    if (++baseLen > 6) return false;
  }
  if (baseLen < 3) return false;
  if (!input[baseLen]) return true;
  const char *ssid = input + baseLen + 1;
  if (!ssid[0] || strlen(ssid) > 2) return false;
  if (ssid[0] < '0' || ssid[0] > '9') return false;
  if (ssid[1] && (ssid[1] < '0' || ssid[1] > '9')) return false;
  if (ssid[1] && ssid[2]) return false;
  const int val = atoi(ssid);
  if (val > 15 || (ssid[0] == '0' && ssid[1])) return false;
  return true;
}
bool storeCallsign(const char *call) {
  if (!directActive || !validateCall(call)) return false;
  char previousCall[sizeof(stationCall)]={};
  strncpy(previousCall,stationCall,sizeof(previousCall)-1);
  bool previousValid=callsignValid;
  strncpy(stationCall,call,sizeof(stationCall)-1);
  stationCall[sizeof(stationCall)-1]=0;
  callsignValid=true;
  if (!directWrite()) {
    strcpy(stationCall,previousCall);
    callsignValid=previousValid;
    return false;
  }
  return true;
}

// Format direct APRS uncompressed lat/lon, 2 decimal minutes (not decimal degrees).
// No TX unless both coordinates are fresh and in range.
bool makePositionPayload(char *out, size_t capacity) {
  if (!callsignValid || !gps.location.isValid() || gps.location.age() >= 10000) return false;
  const double lat = gps.location.lat();
  const double lon = gps.location.lng();
  if (!isfinite(lat) || !isfinite(lon) || fabs(lat) > 90.0 || fabs(lon) > 180.0) return false;
  // Convert to centiminutes using integer rounding; carry 60.00 correctly.
  long latCenti = lround(fabs(lat) * 6000.0);
  long lonCenti = lround(fabs(lon) * 6000.0);
  const int latDeg = (int)(latCenti / 6000L);
  const int lonDeg = (int)(lonCenti / 6000L);
  const long latMin = latCenti % 6000L;
  const long lonMin = lonCenti % 6000L;
  char latText[10], lonText[11];
  snprintf(latText, sizeof(latText), "%02d%02ld.%02ld%c", latDeg, latMin/100L, latMin%100L, lat < 0 ? 'S' : 'N');
  snprintf(lonText, sizeof(lonText), "%03d%02ld.%02ld%c", lonDeg, lonMin/100L, lonMin%100L, lon < 0 ? 'W' : 'E');
  // APRS-IS destination APZTFM is experimental until TOCALL registration.
  // Park position icon can be shown when stationary, independent of WX.
  const bool includeComment = commentIntervalMin &&
      (!lastCommentTxAt || (uint32_t)(millis()-lastCommentTxAt)>=
                             (uint32_t)commentIntervalMin*60000UL);
  const char table = parkedConfirmed ? parkedSymbolTable : positionSymbolTable;
  const char symbol = parkedConfirmed ? parkedSymbol : positionSymbol;
  char commentAndBattery[105]={};
  if (includeComment) strncat(commentAndBattery,commentText,sizeof(commentAndBattery)-1);
  // Battery is attached independently of comment scheduling; no extra RF packet.
  if (beaconBatteryVoltage && isfinite(batteryV) && batteryV>=2.0f && batteryV<=5.5f) {
    char batt[22];
    snprintf(batt,sizeof(batt),"%sBatt=%.2fV",commentAndBattery[0]?" ":"",batteryV);
    strncat(commentAndBattery,batt,sizeof(commentAndBattery)-strlen(commentAndBattery)-1);
  }
  const int n = snprintf(out, capacity, "%s>APZTFM,WIDE1-1:!%s%c%s%c%s",
                         stationCall, latText, table, lonText, symbol, commentAndBattery);
  return n > 0 && (size_t)n < capacity;
}
// APRS complete weather report. BME280 has no wind/rain sensors:
// unknown wind is represented by .../... (not fabricated 000/000).
// Required weather symbol table '/' and symbol '_' mark station WX.
// Fahrenheit t###; RH h## (00 means 100%); pressure b##### in 0.1 hPa.
// IMPORTANT: pressure from BME280 is station pressure; no sea-level correction yet.
bool makeWeatherPayload(char *out, size_t capacity) {
  if (!callsignValid || !gpsFix() || !bmeReady || !bmeValuesOK ||
      (uint32_t)(millis() - lastBmeGoodAt) > 15000UL) return false;
  if (!isfinite(tempC) || !isfinite(humidity) || !isfinite(pressureHPa)) return false;
  const double lat = gps.location.lat();
  const double lon = gps.location.lng();
  if (!isfinite(lat) || !isfinite(lon) || fabs(lat) > 90.0 || fabs(lon) > 180.0) return false;
  long latCenti = lround(fabs(lat) * 6000.0);
  long lonCenti = lround(fabs(lon) * 6000.0);
  int latDeg = (int)(latCenti / 6000L);
  int lonDeg = (int)(lonCenti / 6000L);
  long latMin = latCenti % 6000L;
  long lonMin = lonCenti % 6000L;
  char latText[10], lonText[11];
  snprintf(latText, sizeof(latText), "%02d%02ld.%02ld%c", latDeg, latMin/100L, latMin%100L, lat < 0 ? 'S' : 'N');
  snprintf(lonText, sizeof(lonText), "%03d%02ld.%02ld%c", lonDeg, lonMin/100L, lonMin%100L, lon < 0 ? 'W' : 'E');

  const int tF = (int)lroundf(tempC * 1.8f + 32.0f);
  if (tF < -99 || tF > 999) return false;
  char tStr[5];
  if (tF < 0) snprintf(tStr, sizeof(tStr), "-%02d", -tF);
  else snprintf(tStr, sizeof(tStr), "%03d", tF);
  int rh = (int)lroundf(humidity);
  if (rh < 0 || rh > 100) return false;
  const int wxRh = (rh == 100) ? 0 : rh;
  const long pressure10 = lroundf(pressureHPa * 10.0f);
  if (pressure10 < 0 || pressure10 > 99999) return false;

  // Complete weather report: !DDMM.mmN/DDDMM.mmE_.../...t075h43b10171
  // The 7 characters .../... represent unavailable wind direction/speed.
  // 3c FF 01 LoRa APRS header is attached only when actually transmitting.
  const int n = snprintf(out, capacity,
      "%s>APZTFM,WIDE1-1:!%s/%s_.../...t%sh%02db%05ld",
      stationCall, latText, lonText, tStr, wxRh, pressure10);
  return n > 0 && (size_t)n < capacity;
}

bool manualWxTx() {
  if (!callsignValid) { showMessage("SET CALL VIA USB"); Serial.println("[WX BLOCK] callsign missing"); return false; }
  if (!gpsFix()) { showMessage("NO GPS FIX"); Serial.println("[WX BLOCK] fresh GPS position required"); return false; }
  if (!bmeReady || !bmeValuesOK || (uint32_t)(millis() - lastBmeGoodAt) > 15000UL) {
    showMessage("BME DATA ERROR"); Serial.println("[WX BLOCK] missing/stale BME280 measurements"); return false;
  }
  if (!loraReady) { showMessage("LORA NOT READY"); Serial.println("[WX BLOCK] LoRa not ready"); return false; }
  const uint32_t now = millis();
  if (everTransmitted && (uint32_t)(now - lastTxAt) < AUX_TX_GAP_MS) {
    showMessage("WX WAIT 15s"); Serial.println("[WX BLOCK] 15s auxiliary TX guard"); return false;
  }
  char packet[155] = {};
  if (!makeWeatherPayload(packet, sizeof(packet))) { showMessage("WX FORMAT ERR"); Serial.println("[WX BLOCK] invalid WX measurements"); return false; }
  const size_t n = strlen(packet);
  uint8_t frame[160] = {};
  memcpy(frame, APRS_HEADER, sizeof(APRS_HEADER));
  memcpy(frame + sizeof(APRS_HEADER), packet, n);
  Serial.print("[WX TX] REQUEST: "); Serial.println(packet);
  everTransmitted = true;
  lastTxAt = now;
  loraRxFlag = false;
  radio->standby();
  txLastCode = radio->transmit(frame, sizeof(APRS_HEADER) + n);
  if (txLastCode == RADIOLIB_ERR_NONE) {
    ++txSuccesses;
    ++wxSuccesses;
    showMessage("WX TX OK (LOCAL)");
    Serial.println("[WX TX] RadioLib OK; verify aprs.fi WX and iGate reception");
  } else {
    showMessage("WX TX ERROR");
    Serial.print("[WX TX] RadioLib error: "); Serial.println(txLastCode);
  }
  loraRxFlag = false;
  loraError = radio->startReceive();
  loraReady = loraError == RADIOLIB_ERR_NONE;
  if (!loraReady) Serial.println("[WX TX] WARN: RX restart failed");
  return txLastCode == RADIOLIB_ERR_NONE;
}

bool manualAprsTx() {
  if (!callsignValid) { showMessage("SET CALL VIA USB"); Serial.println("[TX BLOCK] No valid callsign. USB: c TA1FM-7"); return false; }
  if (!gpsFix()) { showMessage("NO GPS FIX"); Serial.println("[TX BLOCK] GPS location not fresh"); return false; }
  if (!loraReady) { showMessage("LORA NOT READY"); Serial.println("[TX BLOCK] LoRa RX not ready"); return false; }
  const uint32_t now = millis();
  if (everTransmitted && (uint32_t)(now - lastTxAt) < POS_TX_GAP_MS) {
    showMessage("POS WAIT 15s"); Serial.println("[TX BLOCK] 15s position TX guard"); return false;
  }
  char packet[155] = {};
  if (!makePositionPayload(packet, sizeof(packet))) { showMessage("GPS/CALL ERROR"); return false; }
  const size_t n = strlen(packet);
  uint8_t frame[160] = {};
  memcpy(frame, APRS_HEADER, sizeof(APRS_HEADER));
  memcpy(frame + sizeof(APRS_HEADER), packet, n);
  Serial.print("[TX] REQUEST: "); Serial.println(packet);
  // Only this function can turn on LoRa TX; everything else remains RX only.
  everTransmitted = true;
  lastTxAt = now;
  loraRxFlag = false;
  radio->standby();
  txLastCode = radio->transmit(frame, sizeof(APRS_HEADER) + n);
  if (txLastCode == RADIOLIB_ERR_NONE) {
    ++txSuccesses;
    lastPositionTxAt = now;
    if (commentIntervalMin && strstr(packet, commentText)) lastCommentTxAt = now;
    if (!firstPositionDone && statusIntervalMin) { startupStatusPending = true; statusDueAt = now; }
    firstPositionDone = true;
    lastPosWasParked = parkedConfirmed;
    if (gps.course.isValid() && gps.course.age() < 5000)
      sentCourse = gps.course.deg();
    showMessage("TX OK (LOCAL)");
    Serial.println("[TX] RadioLib TX OK; confirm reception on TA1FM-10 iGate");
  } else {
    showMessage("TX ERROR");
    Serial.print("[TX] RadioLib error: "); Serial.println(txLastCode);
  }
  loraRxFlag = false;
  loraError = radio->startReceive();
  loraReady = loraError == RADIOLIB_ERR_NONE;
  if (!loraReady) Serial.println("[TX] WARN: RX restart failed");
  return txLastCode == RADIOLIB_ERR_NONE;
}

// APRS status packet: information field starts with > (no GPS required).
bool manualStatusTx() {
  if (!callsignValid || !loraReady) { Serial.println("[STATUS TX] No callsign/LoRa"); return false; }
  uint32_t now = millis();
  if (everTransmitted && (uint32_t)(now-lastTxAt)<AUX_TX_GAP_MS) {
    Serial.println("[STATUS TX] WAIT 15s"); return false;
  }
  if (!validText(statusText)) return false;
  char packet[155] = {};
  int n = snprintf(packet,sizeof(packet),"%s>APZTFM,WIDE1-1:>%s",stationCall,statusText);
  if (n<=0 || n>=(int)sizeof(packet)) return false;
  uint8_t frame[160] = {};
  memcpy(frame, APRS_HEADER, sizeof(APRS_HEADER));
  memcpy(frame+sizeof(APRS_HEADER),packet,n);
  Serial.print("[STATUS TX] REQUEST: "); Serial.println(packet);
  lastTxAt=now; everTransmitted=true; loraRxFlag=false;
  radio->standby(); txLastCode=radio->transmit(frame,sizeof(APRS_HEADER)+n);
  bool ok = txLastCode==RADIOLIB_ERR_NONE;
  if (ok) { ++txSuccesses; lastStatusTxAt=now; Serial.println("[STATUS TX] RadioLib OK"); showMessage("STATUS TX OK"); }
  else { Serial.print("[STATUS TX] error="); Serial.println(txLastCode); }
  loraRxFlag=false;
  loraError=radio->startReceive();
  loraReady=loraError==RADIOLIB_ERR_NONE;
  return ok;
}

// GPS speed hysteresis: 0-5 stationary candidate, 5-8 keep state,
// 8+ on 3 successive 1-sec samples indicates motion.
void updateMotionState(uint32_t now) {
  if (!gpsFix() || !gps.speed.isValid() || gps.speed.age()>=5000) return;
  float kmh = gps.speed.kmph();
  if (!isfinite(kmh) || kmh<0) return;
  if (lastMotionSampleAt && (uint32_t)(now-lastMotionSampleAt)<1000UL) return;
  lastMotionSampleAt=now;
  if (kmh>=MOVE_SPEED_KMH) {
    if (movingSamples<3) ++movingSamples;
    if (movingSamples>=3) {
      if (!movingConfirmed) {
        Serial.println("[AUTO] MOVING CONFIRMED");
        if (beaconOnFirstMove && positionMode==1 && firstPositionDone) {
          positionDueAt=now; // first movement beacon, subject to shared RF TX guard
          Serial.println("[SMART] First movement -> position due");
        }
      }
      movingConfirmed=true; parkedConfirmed=false; lastMotionSeenAt=now;
    }
  } else if (kmh<=STOP_SPEED_KMH) {
    movingSamples=0; movingConfirmed=false;
    if (!lastMotionSeenAt) lastMotionSeenAt=autoEnabledAt;
    if (!parkedConfirmed && (uint32_t)(now-lastMotionSeenAt) >=
         (uint32_t)parkWaitMin*60000UL) {
      parkedConfirmed=true; Serial.println("[AUTO] PARKED CONFIRMED");
    }
  } else {
    movingSamples=0;
    if (!lastMotionSeenAt) lastMotionSeenAt=autoEnabledAt;
    if (!movingConfirmed && !parkedConfirmed &&
       (uint32_t)(now-lastMotionSeenAt)>=(uint32_t)parkWaitMin*60000UL) {
      parkedConfirmed=true; Serial.println("[AUTO] PARKED CONFIRMED (jitter)");
    }
  }
}

// Priority: first position, transition, timed position; WX; Status.
// All timers independently become DUE. A shared RF guard queues them.
void pollAutoBeacon() {
  if (autoWxMode==0 || !callsignValid || !loraReady) return;
  uint32_t now=millis();
  updateMotionState(now);
  bool fix=gpsFix();
  if (fix) {
    if (!firstPositionDone || parkedConfirmed!=lastPosWasParked) positionDueAt=now;
    else if (!positionDueAt) {
      uint32_t dueMs=(uint32_t)fixedPositionMin*60000UL;
      if (positionMode==1) {
        // V0.9.5: six configurable speed/interval breakpoints.
        const float speed=(gps.speed.isValid() && gps.speed.age()<5000) ? gps.speed.kmph() : 0;
        dueMs=smartRateMs(speed);
        if (dueMs<POS_TX_GAP_MS) dueMs=POS_TX_GAP_MS;
        // Heading-change beacon (corner pegging), GPS course only with movement.
        if (movingConfirmed && speed>=10.0f &&
            gps.course.isValid() && gps.course.age()<5000 && isfinite(sentCourse) &&
            lastPositionTxAt && (uint32_t)(now-lastPositionTxAt)>=POS_TX_GAP_MS) {
          float turn=fabsf(gps.course.deg()-sentCourse);
          if (turn>180.0f) turn=360.0f-turn;
          // Conservative 35deg threshold; avoids repeated noise turn packets.
          if (turn>=35.0f) positionDueAt=now;
        }
      }
      if (!positionDueAt && lastPositionTxAt &&
          (uint32_t)(now-lastPositionTxAt)>=dueMs) positionDueAt=now;
    }
  }
  // Queued independently. Position gets priority at initial FIX, parking/moving
  // transitions and when its track is already over 60s old. Between routine
  // position updates, a pending WX/Status packet gets a fair turn, avoiding
  // permanent starvation if rapid Smart Beaconing continues.
  const bool posWaiting = positionDueAt && fix;
  const bool posCritical = posWaiting &&
      (!firstPositionDone || parkedConfirmed!=lastPosWasParked ||
       (lastPositionTxAt && (uint32_t)(now-lastPositionTxAt)>=60000UL));
  const bool auxiliaryAllowed = !everTransmitted ||
      (uint32_t)(now-lastTxAt)>=AUX_TX_GAP_MS;
  const bool posAllowed = !everTransmitted ||
      (uint32_t)(now-lastTxAt)>=POS_TX_GAP_MS;
  const bool wxWaiting = wxIntervalMin && wxDueAt &&
      (int32_t)(now-wxDueAt)>=0 && fix &&
      bmeReady && bmeValuesOK && (uint32_t)(now-lastBmeGoodAt)<=15000UL;
  const bool statusWaiting = statusIntervalMin && statusDueAt &&
      (int32_t)(now-statusDueAt)>=0;

  if (posCritical && posAllowed) {
    lastPositionAttemptAt=now;
    Serial.println("[AUTO] POSITION due (priority)");
    if (manualAprsTx()) positionDueAt=0;
    return;
  }
  // First boot: GPS position already carries APRS Comment; after 15s send Status.
  // Do not let WX overtake the initial Status package.
  if (startupStatusPending && auxiliaryAllowed && statusWaiting) {
    lastStatusAttemptAt=now;
    Serial.println("[AUTO] STARTUP STATUS (after POS+COMMENT)");
    if (manualStatusTx()) { startupStatusPending=false; statusDueAt=now+(uint32_t)statusIntervalMin*60000UL; }
    return;
  }
  if (auxiliaryAllowed && wxWaiting &&
      (!lastWxAttemptAt || (uint32_t)(now-lastWxAttemptAt)>=AUX_TX_GAP_MS)) {
    lastWxAttemptAt=now;
    Serial.println("[AUTO] WX due (independent)");
    if (manualWxTx()) { lastAutoWxAt=now; wxDueAt=now+(uint32_t)wxIntervalMin*60000UL; }
    return;
  }
  if (!startupStatusPending && firstPositionDone && auxiliaryAllowed && statusWaiting &&
      (!lastStatusAttemptAt || (uint32_t)(now-lastStatusAttemptAt)>=AUX_TX_GAP_MS)) {
    lastStatusAttemptAt=now;
    Serial.println("[AUTO] STATUS due (independent)");
    if (manualStatusTx()) statusDueAt=now+(uint32_t)statusIntervalMin*60000UL;
    return;
  }
  if (posWaiting && posAllowed) {
    lastPositionAttemptAt=now;
    Serial.println("[AUTO] POSITION due");
    if (manualAprsTx()) positionDueAt=0;
  }
}
char usbLine[100] = {};
size_t usbLineLen = 0;
// Web Configurator ile uyumlu OLED seri komut bildirimleri.
void oledUsbReport();
void oledUsbPreview(uint8_t type);
void oledUsbConfirm();
void oledUsbCancel();
void oledUsbSetSleep(uint8_t mode);
void oledUsbWake();

void handleUsbCommand(const char *line) {
  if (strcmp(line, "chip?") == 0) {
    Serial.print("[CHIP] selected="); Serial.print(loraChipName());
    Serial.print(" saved="); Serial.println(directActive ? "DIRECT" : "RAM ONLY");
    Serial.println("[CHIP] Degistirmek icin: chip sx1262 | chip sx1268. Sonra USB'den restart.");
    return;
  }
  if (strcmp(line, "chip sx1262") == 0 || strcmp(line, "chip sx1268") == 0) {
    const uint8_t target = strcmp(line, "chip sx1262") == 0 ? CHIP_SX1262 : CHIP_SX1268;
    if (!directActive) {
      // flash init oncesi RAM secimi desteklenir. Kalici kayit yapilmaz.
      loraChip = target;
      Serial.print("[CHIP] RAM selected: "); Serial.println(loraChipName());
      Serial.println("[CHIP] Kalici kayit icin ayarlari YEDEKLEDIKTEN sonra flash init kullanin.");
      return;
    }
    if (loraChip == target) { Serial.println("[CHIP] Zaten secili"); return; }
    uint8_t previous = loraChip; loraChip = target;
    if (!directWrite()) { loraChip = previous; Serial.println("[CHIP] SAVE FAILED; old selection retained"); return; }
    Serial.print("[CHIP] SAVED: "); Serial.print(loraChipName());
    Serial.println(" - etki icin cihazi yeniden baslatin");
    return;
  }
  if (strcmp(line, "oled?") == 0) { oledUsbReport(); return; }
  if (strcmp(line, "oled wake") == 0) { oledUsbWake(); return; }
  if (strcmp(line, "oled confirm") == 0) { oledUsbConfirm(); return; }
  if (strcmp(line, "oled cancel") == 0) { oledUsbCancel(); return; }
  if (strcmp(line, "oled sh1106") == 0) { oledUsbPreview(1); return; }
  if (strcmp(line, "oled ssd1306") == 0) { oledUsbPreview(2); return; }
  if (strncmp(line, "sleep ", 6) == 0) {
    const char *v=line+6;
    if (strlen(v)==1 && *v>='0' && *v<='6') oledUsbSetSleep((uint8_t)(*v-'0'));
    else Serial.println("[OLED] Kullanim: sleep 0..6");
    return;
  }
  if (strcmp(line,"flash?")==0) {directInfo();return;}
  if (strcmp(line,"flash init")==0) {
    if (directActive) {Serial.println("[DIRECT] Already active");return;}
    Serial.println("[DIRECT] USER REQUEST: initialize current RAM settings (1 page write)");
    directActive=true;
    if (directWrite()) {
      configStorageOK = true;
      Serial.println("[DIRECT] INIT OK; power cycle and verify with p and flash?");
    } else {
      directActive=false;
      configStorageOK=false;
      Serial.println("[DIRECT] INIT FAILED");
    }
    return;
  }
  if (strncmp(line, "c ", 2) == 0) {
    char call[12] = {};
    const char *p = line + 2;
    const size_t len = strlen(p);
    if (len >= sizeof(call)) { Serial.println("[CALL] Too long"); return; }
    for (size_t i = 0; i < len; ++i) call[i] = (char)toupper((unsigned char)p[i]);
    if (!validateCall(call)) { Serial.println("[CALL] Invalid callsign; example: TA1FM-7"); return; }
    const bool firstValidCall = !callsignValid;
    if (!storeCallsign(call)) {
      Serial.println("[CALL] SAVE FAILED; previous stored call unchanged");
      return;
    }
    Serial.println("[CALL] Saved");
    if (firstValidCall) {
      autoWxMode = 1;
      resetAutoTimers();
      Serial.println(saveAutoSettings() ?
          "[AUTO] ON (first callsign saved)" : "[AUTO] ON, SAVE FAILED");
    }
  } else if (strncmp(line, "power ", 6) == 0) {
    const char *v=line+6;
    if (strlen(v)>0 && strlen(v)<=2 && strspn(v,"0123456789")==strlen(v) && validTxPower(atoi(v))) changeTxPower((int8_t)atoi(v));
    else Serial.println("Usage: power 1..22");
  } else if (strncmp(line,"smart ",6)==0) {
    // smart 600,90,45,30,20,15   (seconds; fixed 7/10/20/49/60/90 km/h)
    uint16_t proposed[6]={}; char extra=0;
    int a[6]={};
    if (sscanf(line+6,"%d,%d,%d,%d,%d,%d%c",&a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&extra)!=6) {
      Serial.println("Usage: smart 600,90,45,30,20,15"); return;
    }
    for (uint8_t i=0;i<6;++i) {
      if (a[i]<10 || a[i]>3600) { Serial.println("[SMART] 10..3600 seconds required");return; }
      proposed[i]=(uint16_t)a[i];
    }
    if (!validSmartIntervals(proposed)) { Serial.println("[SMART] Invalid: intervals must decrease with speed"); return; }
    uint16_t old[6]; memcpy(old,smartIntervalsSec,sizeof(old));
    memcpy(smartIntervalsSec,proposed,sizeof(smartIntervalsSec));
    if (!saveSmartSettings()) { memcpy(smartIntervalsSec,old,sizeof(old));Serial.println("[SMART] SAVE FAILED"); }
    else Serial.println("[SMART] intervals saved");
  } else if (strncmp(line,"motion ",7)==0) {
    if (strcmp(line+7,"on") && strcmp(line+7,"off")) {Serial.println("Usage: motion on|off");return;}
    bool old=beaconOnFirstMove; beaconOnFirstMove=strcmp(line+7,"on")==0;
    if (!saveSmartSettings()) {beaconOnFirstMove=old;Serial.println("[SMART] SAVE FAILED");}
    else Serial.println("[SMART] first movement saved");
  } else if (strncmp(line,"batt ",5)==0) {
    if (strcmp(line+5,"on") && strcmp(line+5,"off")) {Serial.println("Usage: batt on|off");return;}
    bool old=beaconBatteryVoltage; beaconBatteryVoltage=strcmp(line+5,"on")==0;
    if (!saveSmartSettings()) {beaconBatteryVoltage=old;Serial.println("[BATT] SAVE FAILED");}
    else Serial.println("[BATT] beacon option saved");
  } else if (strcmp(line,"smart?")==0) {
    Serial.print("[SMART] rates=");
    for(uint8_t i=0;i<6;++i){if(i)Serial.print(',');Serial.print(smartIntervalsSec[i]);}
    Serial.print(" motion="); Serial.print(beaconOnFirstMove?"on":"off");
    Serial.print(" batt="); Serial.println(beaconBatteryVoltage?"on":"off");
  } else if (strcmp(line, "power?") == 0) {
    Serial.print("[TXPOWER] "); Serial.print(loraTxPowerDbm); Serial.println(" dBm");
  } else if (strcmp(line, "t") == 0) {
    manualAprsTx();
  } else if (strcmp(line, "w") == 0) {
    manualWxTx();
  } else if (strncmp(line, "a ", 2) == 0) {
    const char *v = line + 2;
    if (strcmp(v,"off") == 0) autoWxMode = 0;
    else if (strcmp(v,"on") == 0 || strcmp(v,"parked") == 0 ||
             strcmp(v,"always") == 0) autoWxMode = 1;
    else { Serial.println("Usage: a on|off"); return; }
    resetAutoTimers();
    Serial.print("[AUTO] mode="); Serial.println(modeName(autoWxMode));
    Serial.println(saveAutoSettings() ? "[AUTO] saved" : "[AUTO] SAVE FAILED");
  } else if (strncmp(line, "x ", 2) == 0) {
    if (!(strcmp(line+2,"off") == 0 || strcmp(line+2,"15") == 0 ||
          strcmp(line+2,"30") == 0 || strcmp(line+2,"60") == 0)) {
      Serial.println("Usage: x off|15|30|60"); return;
    }
    wxIntervalMin = strcmp(line+2,"off") == 0 ? 0 : (uint8_t)atoi(line+2);
    wxDueAt = wxIntervalMin ? millis() + (uint32_t)wxIntervalMin*60000UL : 0;
    Serial.println(saveAutoSettings() ? "[WX] interval saved" : "[WX] SAVE FAILED");
  } else if (strncmp(line, "k ", 2) == 0) {
    if (!(strcmp(line+2,"1") == 0 || strcmp(line+2,"3") == 0 || strcmp(line+2,"5") == 0)) {
      Serial.println("Usage: k 1|3|5"); return;
    }
    parkWaitMin = (uint8_t)atoi(line + 2);
    resetAutoTimers();
    Serial.println(saveAutoSettings() ? "[AUTO] Park minutes saved" : "[AUTO] SAVE FAILED");
  } else if (strncmp(line,"parksymbol ",11)==0) {
    const char* v=line+11;
    if (strlen(v)!=2 || (v[0]!='/' && v[0]!='\\') || v[1]<'!' || v[1]>'~') {
      Serial.println("[PARK SYMBOL] Usage: parksymbol \\P");return;
    }
    uint8_t prevTable=parkedSymbolTable;char prevIcon=parkedSymbol;
    parkedSymbolTable=(uint8_t)v[0];parkedSymbol=v[1];
    if (!saveAutoSettings()) {parkedSymbolTable=prevTable;parkedSymbol=prevIcon;Serial.println("[PARK SYMBOL] SAVE FAILED");}
    else {Serial.print("[PARK SYMBOL] SAVED ");Serial.print((char)parkedSymbolTable);Serial.println(parkedSymbol);}
  } else if (strncmp(line,"symbol ",7)==0) {
    const char *v = line+7;
    if (strlen(v)!=2 || (v[0]!='/' && v[0]!='\\') || v[1]<'!' || v[1]>'~') {
      Serial.println("[SYMBOL] Usage: symbol />  (2 karakter)"); return;
    }
    uint8_t oldTable=positionSymbolTable; char oldSymbol=positionSymbol;
    positionSymbolTable=(uint8_t)v[0]; positionSymbol=v[1];
    if (!saveAutoSettings()) { positionSymbolTable=oldTable;positionSymbol=oldSymbol;Serial.println("[SYMBOL] SAVE FAILED");}
    else {Serial.print("[SYMBOL] SAVED ");Serial.print(positionSymbolTable);Serial.println(positionSymbol);}
  } else if (strncmp(line, "s ", 2) == 0) {
    const char *v = line + 2;
    if (strcmp(v,"car") == 0) selectSymbol(0);
    else if (strcmp(v,"walker") == 0) selectSymbol(1);
    else if (strcmp(v,"house") == 0) selectSymbol(2);
    else { Serial.println("Usage: s car|walker|house"); return; }
    Serial.println(saveAutoSettings() ? "[SYMBOL] saved" : "[SYMBOL] SAVE FAILED");
  } else if (strncmp(line, "comment ", 8) == 0) {
    const char *v=line+8;
    if (!validText(v)) { Serial.println("[COMMENT] ASCII 1..62 chars; no | or ~"); return; }
    strncpy(commentText,v,sizeof(commentText)-1); commentText[sizeof(commentText)-1]=0;
    lastCommentTxAt=0;
    Serial.println(saveBeaconSettings() ? "[COMMENT] saved" : "[COMMENT] SAVE FAILED");
  } else if (strncmp(line, "status ", 7) == 0) {
    const char *v=line+7;
    if (!validText(v)) { Serial.println("[STATUS TEXT] ASCII 1..62 chars; no | or ~"); return; }
    strncpy(statusText,v,sizeof(statusText)-1); statusText[sizeof(statusText)-1]=0;
    Serial.println(saveBeaconSettings() ? "[STATUS TEXT] saved" : "[STATUS TEXT] SAVE FAILED");
  } else if (strcmp(line, "comment?") == 0) {
    Serial.print("[COMMENT] "); Serial.println(commentText);
  } else if (strcmp(line, "status?") == 0) {
    Serial.print("[STATUS TEXT] "); Serial.println(statusText);
  } else if (strcmp(line, "v") == 0) {
    manualStatusTx();
  } else if (strncmp(line, "z ", 2) == 0) {
    if (strcmp(line+2,"off")==0) statusIntervalMin=0;
    else if (strcmp(line+2,"15")==0) statusIntervalMin=15;
    else if (strcmp(line+2,"30")==0) statusIntervalMin=30;
    else if (strcmp(line+2,"60")==0) statusIntervalMin=60;
    else { Serial.println("Usage: z off|15|30|60"); return; }
    statusDueAt=statusIntervalMin ? millis()+(uint32_t)statusIntervalMin*60000UL : 0;
    Serial.println(saveBeaconSettings()?"[STATUS] interval saved":"[STATUS] SAVE FAILED");
  } else if (strncmp(line,"j ",2)==0) {
    if (strcmp(line+2,"off")==0) commentIntervalMin=0;
    else if (strcmp(line+2,"15")==0) commentIntervalMin=15;
    else if (strcmp(line+2,"30")==0) commentIntervalMin=30;
    else if (strcmp(line+2,"60")==0) commentIntervalMin=60;
    else { Serial.println("Usage: j off|15|30|60"); return; }
    lastCommentTxAt=0;
    Serial.println(saveBeaconSettings()?"[COMMENT] interval saved":"[COMMENT] SAVE FAILED");
  } else if (strncmp(line,"f ",2)==0) {
    if (strcmp(line+2,"smart")==0) positionMode=1;
    else if (strcmp(line+2,"fixed")==0) positionMode=0;
    else { Serial.println("Usage: f smart|fixed"); return; }
    positionDueAt=0;
    Serial.println(saveBeaconSettings()?"[POS] mode saved":"[POS] SAVE FAILED");
  } else if (strncmp(line,"i ",2)==0) {
    uint8_t val=0;
    if (strcmp(line+2,"1")==0) val=1;
    else if (strcmp(line+2,"3")==0) val=3;
    else if (strcmp(line+2,"5")==0) val=5;
    else if (strcmp(line+2,"10")==0) val=10;
    else if (strcmp(line+2,"15")==0) val=15;
    else { Serial.println("Usage: i 1|3|5|10|15"); return; }
    fixedPositionMin=val;
    Serial.println(saveBeaconSettings()?"[POS] fixed interval saved":"[POS] SAVE FAILED");
  } else if (strcmp(line, "p") == 0 || strcmp(line, "?") == 0) {
    oledUsbReport();
    Serial.print("[STATUS] Call: "); Serial.println(stationCall);
    Serial.print("[STATUS] GPS fix: "); Serial.println(gpsFix() ? "YES" : "NO");
    Serial.print("[STATUS] LoRa CHIP: "); Serial.println(loraChipName());
    Serial.print("[STATUS] LoRa RX: "); Serial.println(loraReady ? "OK" : "ERROR");
    Serial.print("[TXPOWER] "); Serial.print(loraTxPowerDbm); Serial.println(" dBm");
    Serial.print("[STATUS] TX count: "); Serial.println(txSuccesses);
    Serial.print("[STATUS] WX TX count: "); Serial.println(wxSuccesses);
    Serial.print("[AUTO] mode="); Serial.print(modeName(autoWxMode));
    Serial.print(" POS="); Serial.print(positionMode?"SMART":"FIXED");
    Serial.print(" FIXEDmin="); Serial.print(fixedPositionMin);
    Serial.print(" WXmin="); Serial.print(wxIntervalMin);
    Serial.print(" STATUSmin="); Serial.print(statusIntervalMin);
    Serial.print(" COMMENTmin="); Serial.print(commentIntervalMin);
    Serial.print(" ParkMin="); Serial.print(parkWaitMin);
    Serial.print(" Symbol="); Serial.print(positionSymbolTable); Serial.print(positionSymbol);
    Serial.print(" ParkSymbol="); Serial.print((char)parkedSymbolTable); Serial.println(parkedSymbol);
    Serial.print("[SMART] rates=");
    for(uint8_t i=0;i<6;++i){if(i)Serial.print(',');Serial.print(smartIntervalsSec[i]);}
    Serial.print(" motion="); Serial.print(beaconOnFirstMove?"on":"off");
    Serial.print(" batt="); Serial.println(beaconBatteryVoltage?"on":"off");
    Serial.print("[TEXT] Comment: "); Serial.println(commentText);
    Serial.print("[TEXT] Status: "); Serial.println(statusText);
    Serial.print("[RX] Last heard: "); Serial.println(lastHeardCall);
    Serial.print("[AUTO] MOVING="); Serial.print(movingConfirmed);
    Serial.print(" PARKED="); Serial.println(parkedConfirmed);
    Serial.print("[GPS SPEED] ");
    if (gps.speed.isValid()) {
      Serial.print(gps.speed.kmph(), 1);
      Serial.print(" km/h, age="); Serial.print(gps.speed.age());
      Serial.println(" ms");
    } else Serial.println("INVALID (park counter paused)");
    Serial.print("[PARK TIMER] ");
    if (autoWxMode == 0 || !gpsFix() || !gps.speed.isValid() || gps.speed.age() >= 5000)
      Serial.println("WAIT GPS/SPEED");
    else {
      const uint32_t elapsed = (uint32_t)(millis() - (lastMotionSeenAt ? lastMotionSeenAt : autoEnabledAt)) / 1000UL;
      Serial.print(elapsed); Serial.print(" / ");
      Serial.print((uint32_t)parkWaitMin * 60UL); Serial.println(" s");
    }
    Serial.println("[CMDS] a on|off | f smart|fixed | i 1|3|5|10|15 | x off|15|30|60 | z off|15|30|60 | j off|15|30|60");
    Serial.println("[CMDS] smart 600,90,45,30,20,15 | motion on|off | batt on|off | smart?");
    Serial.println("[CMDS] chip? | chip sx1262 | chip sx1268 | flash? | flash init");
    Serial.println("[CMDS] oled? | oled sh1106|ssd1306 | oled confirm|cancel|wake | sleep 0..6");
    Serial.println("[CMDS] comment TEXT | status TEXT | comment? | status? | t POS | w WX | v STATUS | symbol /> | s car|walker|house | power 1..22 | power? | p");
  } else if (line[0]) {
    Serial.print("[USB] Unknown command: "); Serial.println(line);
  }
}


bool buttonRaw = HIGH;
bool buttonStable = HIGH;
bool longHandled = false;
uint32_t buttonChangedAt = 0;
uint32_t pressBeganAt = 0;
uint32_t lastUserAction = 0;
uint32_t lastDraw = 0;
uint32_t lastBattery = 0;
uint32_t lastBme = 0;
uint32_t lastBmeProbe = 0;
uint32_t lastLog = 0;
uint32_t messageUntil = 0;
const char *message = nullptr;

bool i2cPresent(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

void probeBme() {
  if (bmeReady) return;
  const uint8_t addrs[] = { 0x76, 0x77 };
  for (uint8_t i = 0; i < 2; ++i) {
    uint8_t addr = addrs[i];
    if (!i2cPresent(addr)) continue;
    if (bme.begin(addr, &Wire)) {
      bmeReady = true;
      bmeAddress = addr;
      bmeValuesOK = false;
      Serial.print("BME280 hazir, I2C adresi: 0x");
      Serial.println(addr, HEX);
      return;
    }
  }
  Serial.println("BME280 bulunamadi; BME menusu gizli.");
}

void readBme() {
  if (!bmeReady) return;
  float t = bme.readTemperature();
  float h = bme.readHumidity();
  float p = bme.readPressure() / 100.0f;
  if (isfinite(t) && isfinite(h) && isfinite(p) &&
      t > -45.0f && t < 90.0f && h >= 0.0f && h <= 100.0f &&
      p > 300.0f && p < 1200.0f) {
    tempC = t;
    humidity = h;
    pressureHPa = p;
    bmeValuesOK = true;
    lastBmeGoodAt = millis();
  } else {
    bmeValuesOK = false;
    bmeReady = false;
    bmeAddress = 0;
    Serial.println("BME280 okuma hatasi; tekrar aranacak.");
    if (page == PAGE_BME) page = PAGE_MENU;
  }
}

void readBattery() {
  // V0.2'de fiziksel cihazda calisan yaklasik kalibrasyon.
  // Gercek dogruluk icin multimetreyle ayrica kalibre edilecek.
  uint32_t total = 0;
  for (uint8_t i = 0; i < 8; ++i) total += analogRead(VBAT_PIN);
  batteryV = (total / 8.0f) * (3.0f / 4095.0f) * 1.73f;
}

void initOled(uint8_t type) {
  oledSleeping = false;
  oledType = type;
  oledReady = false;
  gfx = nullptr;
  if (!i2cPresent(OLED_ADDR)) {
    Serial.println("OLED I2C 0x3C yok.");
    return;
  }
  if (type == 1) {
    oledReady = sh.begin(OLED_ADDR, true);
    if (oledReady) gfx = &sh;
  } else {
    oledReady = ss.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    if (oledReady) gfx = &ss;
  }
  Serial.println(oledReady ? "OLED hazir" : "OLED baslatma basarisiz");
}

// V0.9.14: OLED web komutlari. Yanlis secimde kayit degismez.
// Bu fonksiyonlar dogrudan flash A/B kayit sistemini kullanir.
void oledUsbReport() {
  Serial.print("[OLED] driver=");
  Serial.print(oledType == 1 ? "sh1106" : "ssd1306");
  Serial.print(" sleep="); Serial.print(oledSleepMode);
  Serial.print(" ready="); Serial.print(oledReady ? 1 : 0);
  Serial.print(" preview="); Serial.println(page == PAGE_OLED_CONFIRM ? 1 : 0);
}
void oledUsbCancel() {
  if (page == PAGE_OLED_CONFIRM && oledPreviewFromUsb) {
    initOled(previousOledType);
    pendingOledType = previousOledType;
    page = PAGE_MAIN;
    oledPreviewFromUsb = false;
    wakeOled();
    Serial.println("[OLED] ONIZLEME IPTAL; onceki surucuye donuldu");
  } else {
    Serial.println("[OLED] IPTAL EDILECEK WEB ONIZLEMESI YOK");
  }
  oledUsbReport();
}
void oledUsbPreview(uint8_t type) {
  if (page == PAGE_OLED_CONFIRM && !oledPreviewFromUsb) {
    Serial.println("[OLED] Once fiziksel ekran onay menusunu bitirin"); return;
  }
  if (page == PAGE_OLED_CONFIRM) oledUsbCancel();
  if (type == oledType) {
    Serial.println("[OLED] SURUCU DEGISMEDI"); oledUsbReport(); return;
  }
  previousOledType = oledType;
  pendingOledType = type;
  initOled(type);
  if (!oledReady) {
    initOled(previousOledType);
    pendingOledType = previousOledType;
    Serial.println("[OLED] ONIZLEME BASLATILAMADI; eski surucuye donuldu");
    oledUsbReport(); return;
  }
  oledConfirmStarted = millis();
  oledConfirmWindowMs = 20000UL;
  oledPreviewFromUsb = true;
  page = PAGE_OLED_CONFIRM;
  wakeOled();
  Serial.println("[OLED] ONIZLEME 20 saniye; ekrani kontrol edin ve oled confirm gonderin");
  oledUsbReport();
}
void oledUsbConfirm() {
  if (page != PAGE_OLED_CONFIRM || !oledPreviewFromUsb) {
    Serial.println("[OLED] ONAYLANACAK WEB ONIZLEMESI YOK"); return;
  }
  if ((uint32_t)(millis() - oledConfirmStarted) >= oledConfirmWindowMs || !oledReady) {
    oledUsbCancel(); return;
  }
  if (!directActive) {
    Serial.println("[OLED] DIRECT FLASH AKTIF DEGIL; kayit yapilmadi");
    oledUsbCancel(); return;
  }
  if (saveConfig()) {
    Serial.println("[OLED] DRIVER SAVED");
  } else {
    initOled(previousOledType);
    Serial.println("[OLED] DRIVER SAVE FAILED; eski surucuye donuldu");
  }
  page = PAGE_MAIN;
  oledPreviewFromUsb = false;
  wakeOled();
  oledUsbReport();
}
void oledUsbSetSleep(uint8_t mode) {
  if (page == PAGE_OLED_CONFIRM) {
    Serial.println("[OLED] Onizlemeyi once onaylayin veya iptal edin");return;
  }
  if (mode>6) return;
  if (!directActive) {
    Serial.println("[OLED] DIRECT FLASH AKTIF DEGIL; kayit engellendi"); return;
  }
  if (mode == oledSleepMode) {
    Serial.println("[OLED] SURE DEGISMEDI");oledUsbReport();return;
  }
  const uint8_t oldMode = oledSleepMode;
  oledSleepMode = mode;
  if (!saveSleepMode()) {
    oledSleepMode = oldMode;
    Serial.println("[OLED] SLEEP SAVE FAILED; eski sure korundu");
    oledUsbReport();return;
  }
  page=PAGE_MAIN;
  wakeOled();
  Serial.println("[OLED] SLEEP SAVED");
  oledUsbReport();
}
void oledUsbWake() {
  wakeOled();
  Serial.println("[OLED] EKRAN UYANDIRILDI");
  oledUsbReport();
}

void oledFlush() {
  if (oledSleeping) return;
  if (oledType == 1) sh.display();
  else ss.display();
}

void showMessage(const char *txt, uint32_t duration) {
  message = txt;
  messageUntil = millis() + duration;
}

bool gpsFix() {
  return gps.location.isValid() && gps.location.age() < 10000;
}

bool gpsTimeFresh() {
  return gps.time.isValid() && gps.time.age() < 10000;
}

void drawTime() {
  gfx->setCursor(78, 0);
  if (!gpsTimeFresh()) {
    gfx->print("--:--:--");
    return;
  }
  int hh = ((int)gps.time.hour() + utcOffset + 24) % 24;
  char s[10];
  snprintf(s, sizeof(s), "%02d:%02d:%02d", hh,
           (int)gps.time.minute(), (int)gps.time.second());
  gfx->print(s);
}

uint8_t menuCount() { return bmeReady ? 11 : 10; }

const char *menuItem(uint8_t item) {
  if (bmeReady) {
    switch (item) {
      case 0: return "BME280 WX";
      case 1: return "AUTO BEACON";
      case 2: return "POS SYMBOL";
      case 3: return "UTC OFFSET";
      case 4: return "DISPLAY";
      case 5: return "OLED TIMER";
      case 6: return "TX POWER";
      case 7: return "GPS INFO";
      case 8: return "LORA INFO";
      case 9: return "DEVICE INFO";
      default: return "EXIT";
    }
  }
  switch (item) {
    case 0: return "AUTO BEACON";
    case 1: return "POS SYMBOL";
    case 2: return "UTC OFFSET";
    case 3: return "DISPLAY";
    case 4: return "OLED TIMER";
    case 5: return "TX POWER";
    case 6: return "GPS INFO";
    case 7: return "LORA INFO";
    case 8: return "DEVICE INFO";
    default: return "EXIT";
  }
}

void showIntro() {
  if (!oledReady || !gfx) return;
  gfx->fillScreen(0);
  gfx->setTextSize(2);
  gfx->setTextColor(SSD1306_WHITE);
  gfx->setCursor(29, 4); gfx->print("TA1FM");
  gfx->setTextSize(1);
  gfx->setCursor(24, 29); gfx->print("LoRa Tracker");
  gfx->setCursor(19, 43); gfx->print("nRF52840 APRS");
  gfx->setCursor(48, 55); gfx->print("V0.9.22");
  oledFlush();
}

void drawOled() {
  if (!oledReady || !gfx || oledSleeping) return;
  gfx->fillScreen(0);
  gfx->setTextColor(SSD1306_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(0, 0);
  gfx->print(stationCall);  // NOCALL until user saves a valid callsign
  if (page == PAGE_MAIN) drawTime();
  else {
    // 7 karakter * 6 px = 42 px; 84 + 42 = 126 <= 128.
    // 91 px'de son karakter otomatik alt satira tasiyordu.
    gfx->setCursor(84, 0);
    gfx->print("V0.9.22");
  }
  gfx->drawLine(0, 10, 127, 10, SSD1306_WHITE);

  if (message && (int32_t)(messageUntil - millis()) > 0) {
    gfx->setCursor(0, 25);
    gfx->print(message);
    gfx->setCursor(0, 48);
    gfx->print(callsignValid ? "MANUAL TX" : "TX BLOCKED");
    oledFlush();
    return;
  }
  message = nullptr;

  if (page == PAGE_MAIN) {
    const bool fix = gpsFix();
    gfx->setCursor(0, 14); gfx->print(fix ? "GPS:FIX" : "GPS:SEARCH");
    gfx->setCursor(76, 14); gfx->print("SAT:");
    if (gps.satellites.isValid() && gps.satellites.age() < 10000)
      gfx->print(gps.satellites.value());
    else gfx->print("--");
    gfx->setCursor(0, 25); gfx->print("SPD: ");
    if (fix && gps.speed.isValid() && gps.speed.age() < 10000)
      gfx->print(gps.speed.kmph(), 1);
    else gfx->print("--");
    gfx->print(" km/h");
    gfx->setCursor(0, 36); gfx->print("LAT: ");
    if (fix) gfx->print(gps.location.lat(), 5);
    else gfx->print("--");
    gfx->setCursor(0, 47); gfx->print("LON: ");
    if (fix) gfx->print(gps.location.lng(), 5);
    else gfx->print("--");
    gfx->setCursor(0, 57); gfx->print("BAT:");
    gfx->print(batteryV, 2); gfx->print('V');
    gfx->setCursor(60, 57);
    gfx->print("RX:"); gfx->print(lastHeardCall);
  } else if (page == PAGE_MENU) {
    if (menuIndex >= menuCount()) menuIndex = 0;
    gfx->setCursor(0, 14); gfx->print("SET ");
    gfx->print(menuIndex + 1); gfx->print('/'); gfx->print(menuCount());
    gfx->setCursor(0, 29); gfx->print('>'); gfx->print(menuItem(menuIndex));
    gfx->setCursor(0, 47); gfx->print("TAP: NEXT");
    gfx->setCursor(0, 57); gfx->print("HOLD: SELECT");
  } else if (page == PAGE_BME) {
    gfx->setCursor(0, 14); gfx->println("BME280 WX (USB:w)");
    if (bmeReady && bmeValuesOK) {
      gfx->setCursor(0, 27); gfx->print("TEMP: "); gfx->print(tempC, 1); gfx->print(" C");
      gfx->setCursor(0, 38); gfx->print("HUM:  %"); gfx->print(humidity, 1);
      gfx->setCursor(0, 49); gfx->print("PRES: "); gfx->print(pressureHPa, 1);
      gfx->setCursor(106, 57); gfx->print("hPa");
    } else {
      gfx->setCursor(0, 31); gfx->print("SENSOR WAIT");
    }
    gfx->setCursor(0, 57); gfx->print("HOLD: BACK");
  } else if (page == PAGE_CLOCK) {
    gfx->setCursor(0, 15); gfx->print("UTC OFFSET");
    gfx->setCursor(0, 28); gfx->print("> ");
    gfx->print(pendingUtc == 0 ? "UTC" : "UTC+3");
    gfx->setCursor(0, 46); gfx->print("TAP: CHANGE");
    gfx->setCursor(0, 57); gfx->print("HOLD: SAVE");
  } else if (page == PAGE_GPS) {
    gfx->setCursor(0, 15); gfx->print("GPS INFO");
    gfx->setCursor(0, 27); gfx->print("SAT: ");
    if (gps.satellites.isValid()) gfx->print(gps.satellites.value());
    else gfx->print("--");
    gfx->setCursor(0, 39); gfx->print("FIX: "); gfx->print(gpsFix() ? "YES" : "NO");
    gfx->setCursor(0, 51); gfx->print("RX CHARS: "); gfx->print(gps.charsProcessed());
  } else if (page == PAGE_LORA) {
    gfx->setCursor(0, 15); gfx->print("LORA RX 433.775");
    gfx->setCursor(0, 27); gfx->print("RF: ");
    if (loraReady) gfx->print("OK");
    else { gfx->print("ERR "); gfx->print(loraError); }
    gfx->setCursor(0, 38); gfx->print("RX: "); gfx->print(loraPackets);
    gfx->setCursor(65, 38); gfx->print("APRS:"); gfx->print(loraAprsPackets);
    gfx->setCursor(0, 49); gfx->print("RSSI:");
    if (loraPackets) { gfx->print(lastRssi, 0); gfx->print(" SNR:"); gfx->print(lastSnr, 0); }
    else gfx->print("--");
    gfx->setCursor(0, 57); gfx->print("AUTO: "); gfx->print(modeName(autoWxMode));
  } else if (page == PAGE_INFO) {
    gfx->setCursor(0, 15); gfx->print("DEVICE INFO");
    gfx->setCursor(0, 27); gfx->print("TA1FM nrf52");
    gfx->setCursor(0, 38); gfx->print("LoRa APRS Tracker");
    gfx->setCursor(0, 49); gfx->print("FW: V0.9.22");
    gfx->setCursor(0, 57); gfx->print("HOLD: BACK");
  } else if (page == PAGE_AUTO) {
    gfx->setCursor(0, 15); gfx->print("AUTO BEACON");
    gfx->setCursor(0, 29); gfx->print("> "); gfx->print(modeName(pendingAutoWxMode));
    gfx->setCursor(0, 41); gfx->print("POS:"); gfx->print(positionMode?"SMART":"FIXED"); gfx->print(" WX:"); gfx->print(wxIntervalMin); gfx->print("m");
    gfx->setCursor(0, 57); gfx->print("HOLD: SAVE");
  } else if (page == PAGE_SYMBOL) {
    gfx->setCursor(0, 15); gfx->print("POSITION SYMBOL");
    gfx->setCursor(0, 29); gfx->print("> "); gfx->print(symbolName(pendingSymbolIndex));
    gfx->setCursor(0, 46); gfx->print("TAP: NEXT");
    gfx->setCursor(0, 57); gfx->print("HOLD: SAVE");
  } else if (page == PAGE_TX_POWER) {
    gfx->setCursor(0, 15); gfx->print("TX POWER");
    gfx->setCursor(0, 28); gfx->print("> "); gfx->print(pendingTxPowerDbm); gfx->print(" dBm");
    gfx->setCursor(0, 46); gfx->print("TAP: +1 dBm");
    gfx->setCursor(0, 57); gfx->print("HOLD: SAVE");
  } else if (page == PAGE_SLEEP) {
    gfx->setCursor(0, 15); gfx->print("OLED AUTO OFF");
    gfx->setCursor(0, 28); gfx->print("> "); gfx->print(sleepModeName(pendingSleepMode));
    gfx->setCursor(0, 46); gfx->print("TAP: NEXT");
    gfx->setCursor(0, 57); gfx->print("HOLD: SAVE");
  } else if (page == PAGE_OLED) {
    gfx->setCursor(0, 15); gfx->print("DISPLAY DRIVER");
    gfx->setCursor(0, 28); gfx->print("> ");
    gfx->print(pendingOledType == 1 ? "SH1106" : "SSD1306");
    gfx->setCursor(0, 46); gfx->print("TAP: CHANGE");
    gfx->setCursor(0, 57); gfx->print("HOLD: TEST");
  } else if (page == PAGE_OLED_CONFIRM) {
    gfx->setCursor(0, 15); gfx->print("DISPLAY TEST");
    gfx->setCursor(0, 28); gfx->print(oledType == 1 ? "SH1106" : "SSD1306");
    gfx->setCursor(0, 40); gfx->print("HOLD: CONFIRM");
    gfx->setCursor(0, 53); gfx->print("AUTO BACK: ");
    uint32_t passed = millis() - oledConfirmStarted;
    gfx->print(passed >= oledConfirmWindowMs ? 0 : (oledConfirmWindowMs - passed + 999) / 1000);
    gfx->print('s');
  }
  oledFlush();
}

void returnToMenu() {
  page = PAGE_MENU;
  lastUserAction = millis();
  if (menuIndex >= menuCount()) menuIndex = 0;
}

void shortClick() {
  lastUserAction = millis();
  wakeOled();
  switch (page) {
    case PAGE_MAIN:
      manualAprsTx();
      break;
    case PAGE_MENU:
      menuIndex = (menuIndex + 1) % menuCount();
      break;
    case PAGE_CLOCK:
      pendingUtc = (pendingUtc == 0) ? 3 : 0;
      break;
    case PAGE_AUTO:
      pendingAutoWxMode = pendingAutoWxMode ? 0 : 1;
      break;
    case PAGE_SYMBOL:
      pendingSymbolIndex = (pendingSymbolIndex + 1) % 3;
      break;
    case PAGE_SLEEP:
      pendingSleepMode = (pendingSleepMode + 1) % 7;
      break;
    case PAGE_TX_POWER:
      pendingTxPowerDbm = (pendingTxPowerDbm >= 22) ? 1 : pendingTxPowerDbm + 1;
      break;
    case PAGE_OLED:
      pendingOledType = (pendingOledType == 1) ? 2 : 1;
      break;
    case PAGE_BME:
    case PAGE_GPS:
    case PAGE_LORA:
    case PAGE_INFO:
    case PAGE_OLED_CONFIRM:
      // Read-only pages; confirmation only on long press.
      break;
  }
}

void longClick() {
  lastUserAction = millis();
  wakeOled();
  switch (page) {
    case PAGE_MAIN:
      page = PAGE_MENU;
      menuIndex = 0;
      break;
    case PAGE_MENU: {
      const char *selected = menuItem(menuIndex);
      if (strcmp(selected, "BME280 WX") == 0) page = PAGE_BME;
      else if (strcmp(selected, "AUTO BEACON") == 0) {
        pendingAutoWxMode = autoWxMode;
        page = PAGE_AUTO;
      } else if (strcmp(selected, "POS SYMBOL") == 0) {
        pendingSymbolIndex = symbolIndex();
        page = PAGE_SYMBOL;
      } else if (strcmp(selected, "UTC OFFSET") == 0) {
        pendingUtc = utcOffset;
        page = PAGE_CLOCK;
      } else if (strcmp(selected, "DISPLAY") == 0) {
        pendingOledType = oledType;
        page = PAGE_OLED;
      } else if (strcmp(selected, "OLED TIMER") == 0) {
        pendingSleepMode = oledSleepMode;
        page = PAGE_SLEEP;
      } else if (strcmp(selected, "TX POWER") == 0) {
        pendingTxPowerDbm = loraTxPowerDbm;
        page = PAGE_TX_POWER;
      } else if (strcmp(selected, "GPS INFO") == 0) page = PAGE_GPS;
      else if (strcmp(selected, "LORA INFO") == 0) page = PAGE_LORA;
      else if (strcmp(selected, "DEVICE INFO") == 0) page = PAGE_INFO;
      else page = PAGE_MAIN;
      break;
    }
    case PAGE_BME:
    case PAGE_GPS:
    case PAGE_LORA:
    case PAGE_INFO:
      returnToMenu();
      break;
    case PAGE_CLOCK:
      utcOffset = pendingUtc;
      saveConfig();
      Serial.print("Saat dilimi: UTC"); Serial.println(utcOffset == 0 ? "" : "+3");
      returnToMenu();
      break;
    case PAGE_AUTO:
      autoWxMode = pendingAutoWxMode;
      resetAutoTimers();
      Serial.print("[AUTO] mode="); Serial.println(modeName(autoWxMode));
      Serial.println(saveAutoSettings() ? "[AUTO] saved" : "[AUTO] SAVE FAILED");
      returnToMenu();
      break;
    case PAGE_SYMBOL:
      selectSymbol(pendingSymbolIndex);
      Serial.println(saveAutoSettings() ? "[SYMBOL] saved" : "[SYMBOL] SAVE FAILED");
      returnToMenu();
      break;
    case PAGE_TX_POWER:
      changeTxPower(pendingTxPowerDbm);
      returnToMenu();
      break;
    case PAGE_SLEEP:
      oledSleepMode = pendingSleepMode;
      saveSleepMode();
      Serial.print("[OLED] standby: "); Serial.println(sleepModeName(oledSleepMode));
      returnToMenu();
      break;
    case PAGE_OLED:
      if (oledType == pendingOledType) { returnToMenu(); break; }
      previousOledType = oledType;
      initOled(pendingOledType);
      oledConfirmStarted = millis();
      oledConfirmWindowMs = OLED_CONFIRM_MS;
      oledPreviewFromUsb = false;
      page = PAGE_OLED_CONFIRM;
      // Do not store until user physically confirms image is visible.
      break;
    case PAGE_OLED_CONFIRM:
      // Only confirm deliberately if screen is legible.
      if (oledReady) {
        const bool saved = saveConfig();
        if (!saved) initOled(previousOledType);
        oledPreviewFromUsb = false;
        returnToMenu();
      }
      break;
  }
}

// BLE write authorization: explicit ON/OFF toggle on panel, OFF by default.
// CAUTION: NUS is unpaired; ON means any client with access may write settings.
static bool bleSavingEnabled = false;

void pollButton() {
  uint32_t now = millis();
  bool raw = digitalRead(BTN_PIN);
  if (raw != buttonRaw) {
    buttonRaw = raw;
    buttonChangedAt = now;
  }
  if (now - buttonChangedAt >= DEBOUNCE_MS && raw != buttonStable) {
    buttonStable = raw;
    if (buttonStable == LOW) {
      pressBeganAt = now;
      consumeWakeClick = oledSleeping;
      longHandled = false;
      if (consumeWakeClick) wakeOled();
    } else if (!longHandled) {
      if (!consumeWakeClick) shortClick();
      consumeWakeClick = false;
    }
  }
  if (buttonStable == LOW && !longHandled &&
      now - pressBeganAt >= LONG_PRESS_MS) {
    longHandled = true;
    consumeWakeClick = false;
    longClick();
  }
}

// BLE UART V0.9.22 SAVE SWITCH - USB/GPS/LoRa ana akisi korunur.
// NUS MTU=23 iken 18 baytlik parcali bildirim ve yazma.
// Ayarlar once DirectRecord RAM taslagina alinir, TEK flash yazimi yapilir.
// Flash init BLE'den yasaktir; OLED driver BLE kaydi dogrudan yapilir.
// USB ve fiziksel OLED onizleme/kurtarma mekanizmalari korunur.
static char bleCommand[160] = {};
static uint8_t bleCommandLen = 0;
static uint32_t bleCommandLastByteMs = 0;
static uint32_t bleLastReportMs = 0;
static bool bleWasConnected = false;
static char bleTxQueue[256] = {};
static uint16_t bleTxLen = 0;
static uint16_t bleTxOffset = 0;
static uint32_t bleLastChunkMs = 0;
static const uint8_t BLE_TX_CHUNK_BYTES = 18;
static const uint32_t BLE_TX_PACE_MS = 65UL;
static const uint32_t BLE_COMMAND_IDLE_MS = 1200UL;
static int8_t bleCfgReadIndex = -1;
static DirectRecord bleDraft = {};
static bool bleDraftValid = false;
static uint32_t bleDraftSeq = 0;

bool bleWriteUnlocked() { return bleSavingEnabled; }
uint32_t bleWriteSecondsLeft() { return bleSavingEnabled ? 1U : 0U; }
void bleCloseWriteWindow() {
  bleSavingEnabled=false;
  bleDraftValid=false;
}

void bleReply(const char* msg); // Bu fonksiyon asagida tanimlidir.

void bleReplyLockState() { bleReply(bleSavingEnabled ? "LOCK|OPEN|1" : "LOCK|CLOSED"); }

bool bleQueueLine(const char* msg) {
  if (!Bluefruit.connected() || !msg) return false;
  const size_t n = strlen(msg);
  if (n == 0 || n > sizeof(bleTxQueue) - 3) return false;
  if (bleTxOffset == bleTxLen) bleTxLen = bleTxOffset = 0;
  if ((size_t)bleTxLen + n + 2 > sizeof(bleTxQueue)) return false;
  memcpy(bleTxQueue + bleTxLen, msg, n);
  bleTxLen += (uint16_t)n;
  bleTxQueue[bleTxLen++] = '\r';
  bleTxQueue[bleTxLen++] = '\n';
  return true;
}

void bleReply(const char* msg) {
  if (bleQueueLine(msg)) { Serial.print("[BLE REPLY] "); Serial.println(msg); }
  else Serial.println("[BLE] Could not queue reply");
}

void bleSendStatus() {
  if (!Bluefruit.connected()) return;
  char response[120];
  const unsigned int sats = gps.satellites.isValid() ? gps.satellites.value() : 0;
  snprintf(response, sizeof(response),
           "INFO|%s|%s|GPS=%u|SAT=%u|RF=%s|BAT=%.2f",
           stationCall, loraChipName(), gpsFix() ? 1U : 0U,
           sats, loraReady ? "OK" : "ERR", batteryV);
  if (bleQueueLine(response)) { Serial.print("[BLE] "); Serial.println(response); }
}

// Each read response is a separate line; a slow consumer cannot overflow the
// TX FIFO, as the next line is generated only after the previous was sent.
void bleCfgSendNext() {
  if (bleCfgReadIndex < 0 || bleTxOffset < bleTxLen) return;
  char line[144] = {};
  switch (bleCfgReadIndex) {
    case 0: snprintf(line,sizeof(line),"CFG|BEGIN|V0.9.22"); break;
    case 1: snprintf(line,sizeof(line),"CFG|callsign=%s",stationCall); break;
    case 2: snprintf(line,sizeof(line),"CFG|chip=%s",loraChipName()); break;
    case 3: snprintf(line,sizeof(line),"CFG|seq=%lu",(unsigned long)directSeq); break;
    case 4: snprintf(line,sizeof(line),"CFG|direct=%s",directActive?"ACTIVE":"NONE"); break;
    case 5: snprintf(line,sizeof(line),"CFG|auto=%s",autoWxMode?"on":"off"); break;
    case 6: snprintf(line,sizeof(line),"CFG|positionMode=%s",positionMode?"smart":"fixed"); break;
    case 7: snprintf(line,sizeof(line),"CFG|fixedMin=%u",fixedPositionMin); break;
    case 8: snprintf(line,sizeof(line),"CFG|symbol=%c%c",positionSymbolTable,positionSymbol); break;
    case 9: snprintf(line,sizeof(line),"CFG|wx=%u",wxIntervalMin); break;
    case 10: snprintf(line,sizeof(line),"CFG|statusMin=%u",statusIntervalMin); break;
    case 11: snprintf(line,sizeof(line),"CFG|commentMin=%u",commentIntervalMin); break;
    case 12: snprintf(line,sizeof(line),"CFG|parkMin=%u",parkWaitMin); break;
    case 13: snprintf(line,sizeof(line),"CFG|comment=%s",commentText); break;
    case 14: snprintf(line,sizeof(line),"CFG|status=%s",statusText); break;
    case 15: snprintf(line,sizeof(line),"CFG|txPower=%d",loraTxPowerDbm); break;
    case 16: snprintf(line,sizeof(line),"CFG|smart=%u,%u,%u,%u,%u,%u",
       smartIntervalsSec[0],smartIntervalsSec[1],smartIntervalsSec[2],
       smartIntervalsSec[3],smartIntervalsSec[4],smartIntervalsSec[5]); break;
    case 17: snprintf(line,sizeof(line),"CFG|smartMotion=%s",beaconOnFirstMove?"on":"off"); break;
    case 18: snprintf(line,sizeof(line),"CFG|beaconBattery=%s",beaconBatteryVoltage?"on":"off"); break;
    case 19: snprintf(line,sizeof(line),"CFG|oledDriver=%s",oledType==1?"sh1106":"ssd1306"); break;
    case 20: snprintf(line,sizeof(line),"CFG|oledSleep=%u",oledSleepMode); break;
    case 21: snprintf(line,sizeof(line),"CFG|parkSymbol=%c%c",(char)parkedSymbolTable,parkedSymbol); break;
    case 22: snprintf(line,sizeof(line),"CFG|END"); break;
    default: bleCfgReadIndex=-1; return;
  }
  if (bleQueueLine(line)) { bleCfgReadIndex++; }
}

bool bleParseNumber(const char* value, uint16_t &out, uint16_t minVal, uint16_t maxVal) {
  if (!value || !*value || strlen(value)>5) return false;
  for (const char* p=value;*p;p++) if (*p<'0'||*p>'9') return false;
  unsigned long n=strtoul(value,nullptr,10);
  if (n<minVal || n>maxVal) return false;
  out=(uint16_t)n; return true;
}

bool bleParseInterval(const char* value, uint8_t &out) {
  if (!strcmp(value,"off") || !strcmp(value,"0")) { out=0;return true; }
  if (!strcmp(value,"15") || !strcmp(value,"30") || !strcmp(value,"60")) {
    out=(uint8_t)atoi(value); return true;
  }
  return false;
}

bool bleStageValue(const char* key, const char* v) {
  uint16_t n=0;
  if (!strcmp(key,"callsign")) {
    char call[12]={}; if (strlen(v)>=sizeof(call)) return false;
    for (size_t i=0;i<strlen(v);i++) call[i]=(char)toupper((unsigned char)v[i]);
    if (!validateCall(call)) return false;
    strcpy(bleDraft.callsign,call); return true;
  }
  if (!strcmp(key,"auto")) {
    if (strcmp(v,"on") && strcmp(v,"off")) return false;
    bleDraft.autoMode=!strcmp(v,"on")?1:0; return true;
  }
  if (!strcmp(key,"positionMode")) {
    if (strcmp(v,"smart") && strcmp(v,"fixed")) return false;
    bleDraft.positionMode=!strcmp(v,"smart")?1:0; return true;
  }
  if (!strcmp(key,"fixedMin")) {
    if (!bleParseNumber(v,n,1,15) || (n!=1&&n!=3&&n!=5&&n!=10&&n!=15)) return false;
    bleDraft.fixedMin=(uint8_t)n; return true;
  }
  if (!strcmp(key,"parkSymbol")) {
    if (strlen(v)!=2 || (v[0]!='/' && v[0]!='\\') || v[1]<'!' || v[1]>'~') return false;
    bleDraft.reserved[1]=(uint8_t)v[0];bleDraft.reserved[2]=(uint8_t)v[1];return true;
  }
  if (!strcmp(key,"chip")) {
    if (!strcmp(v,"SX1262"))bleDraft.reserved[0]=CHIP_SX1262;
    else if (!strcmp(v,"SX1268"))bleDraft.reserved[0]=CHIP_SX1268;
    else return false;
    return true;
  }
  if (!strcmp(key,"symbol")) {
    if (!strcmp(v,"car")) {bleDraft.symbolTable='/';bleDraft.symbol='>';return true;}
    if (!strcmp(v,"walker")) {bleDraft.symbolTable='/';bleDraft.symbol='[';return true;}
    if (!strcmp(v,"house")) {bleDraft.symbolTable='/';bleDraft.symbol='-';return true;}
    // Other valid APRS 2-character symbols are supported by text command.
    if (strlen(v)==2 && (v[0]=='/' || v[0]=='\\') && v[1]>='!' && v[1]<='~') {
      bleDraft.symbolTable=v[0]; bleDraft.symbol=v[1]; return true;
    }
    return false;
  }
  if (!strcmp(key,"wx")) return bleParseInterval(v,bleDraft.wxMin);
  if (!strcmp(key,"statusMin")) return bleParseInterval(v,bleDraft.statusMin);
  if (!strcmp(key,"commentMin")) return bleParseInterval(v,bleDraft.commentMin);
  if (!strcmp(key,"parkMin")) {
    if (!bleParseNumber(v,n,1,5) || (n!=1&&n!=3&&n!=5)) return false;
    bleDraft.parkMin=(uint8_t)n; return true;
  }
  if (!strcmp(key,"comment")) {
    if (!validText(v)) return false;
    strcpy(bleDraft.comment,v);return true;
  }
  if (!strcmp(key,"status")) {
    if (!validText(v)) return false;
    strcpy(bleDraft.status,v);return true;
  }
  if (!strcmp(key,"txPower")) {
    if (!bleParseNumber(v,n,1,22)) return false;
    bleDraft.power=(int8_t)n;return true;
  }
  if (!strcmp(key,"smart")) {
    int a[6]={};char extra=0;uint16_t rates[6]={};
    if (sscanf(v,"%d,%d,%d,%d,%d,%d%c",&a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&extra)!=6) return false;
    for(int i=0;i<6;i++) { if(a[i]<10||a[i]>3600)return false; rates[i]=(uint16_t)a[i]; }
    if (!validSmartIntervals(rates))return false;
    memcpy(bleDraft.intervals,rates,sizeof(rates));return true;
  }
  if (!strcmp(key,"smartMotion")) {
    if (strcmp(v,"on") && strcmp(v,"off"))return false;
    bleDraft.firstMotion=!strcmp(v,"on")?1:0;return true;
  }
  if (!strcmp(key,"beaconBattery")) {
    if (strcmp(v,"on") && strcmp(v,"off"))return false;
    bleDraft.battery=!strcmp(v,"on")?1:0;return true;
  }
  if (!strcmp(key,"oledSleep")) {
    if (!bleParseNumber(v,n,0,6))return false;
    bleDraft.sleepMode=(uint8_t)n;return true;
  }
  if (!strcmp(key,"oledDriver")) {
    if (!strcmp(v,"sh1106")) bleDraft.display=1;
    else if (!strcmp(v,"ssd1306")) bleDraft.display=2;
    else return false;
    return true;
  }
  // Dangerous flash initialization is intentionally unavailable over BLE.
  return false;
}

void bleApplyRecord(const DirectRecord& r) {
  strncpy(stationCall,r.callsign,sizeof(stationCall)-1);stationCall[sizeof(stationCall)-1]=0;
  callsignValid=validateCall(stationCall);
  autoWxMode=r.autoMode;wxIntervalMin=r.wxMin;parkWaitMin=r.parkMin;
  positionSymbolTable=(uint8_t)r.symbolTable;positionSymbol=r.symbol;
  parkedSymbolTable=r.reserved[1]?r.reserved[1]:'\\';parkedSymbol=r.reserved[2]?(char)r.reserved[2]:'P';
  positionMode=r.positionMode;fixedPositionMin=r.fixedMin;
  statusIntervalMin=r.statusMin;commentIntervalMin=r.commentMin;
  memcpy(smartIntervalsSec,r.intervals,sizeof(smartIntervalsSec));
  beaconOnFirstMove=r.firstMotion!=0;beaconBatteryVoltage=r.battery!=0;
  memcpy(commentText,r.comment,sizeof(commentText));
  memcpy(statusText,r.status,sizeof(statusText));
  oledSleepMode=r.sleepMode;loraTxPowerDbm=r.power;
}

bool bleSwitchHardwarePower(int8_t target) {
  if (!loraReady || !radio) return false;
  const int8_t previous=loraTxPowerDbm;
  radio->standby();
  if (radio->setOutputPower(target)!=RADIOLIB_ERR_NONE) {
    loraReady=radio->startReceive()==RADIOLIB_ERR_NONE;return false;
  }
  if (radio->startReceive()!=RADIOLIB_ERR_NONE) {
    radio->standby();radio->setOutputPower(previous);
    loraReady=radio->startReceive()==RADIOLIB_ERR_NONE;
    return false;
  }
  loraRxFlag=false;
  return true;
}

void bleCommitDraft() {
  if (!bleWriteUnlocked()) { bleReply("ERR|LOCKED|OPEN ON WEB PANEL"); return; }
  if (!bleDraftValid) { bleReply("ERR|NO TRANSACTION");return; }
  if (!directActive) { bleReply("ERR|DIRECT INACTIVE");return; }
  if (bleDraftSeq!=directSeq) {bleDraftValid=false;bleReply("ERR|STALE SETTINGS");return;}
  if (!validateCall(bleDraft.callsign)||!validText(bleDraft.comment)||
      !validText(bleDraft.status)||!validSmartIntervals(bleDraft.intervals)||
      !validTxPower(bleDraft.power)||bleDraft.parkMin==0||bleDraft.parkMin>5||
      (bleDraft.display!=1 && bleDraft.display!=2) ||
      (bleDraft.reserved[0]!=CHIP_SX1262 && bleDraft.reserved[0]!=CHIP_SX1268) ||
      (bleDraft.reserved[1]!='/' && bleDraft.reserved[1]!='\\') ||
      bleDraft.reserved[2]<'!' || bleDraft.reserved[2]>'~') {
    bleReply("ERR|INVALID SETTINGS");return;
  }
  DirectRecord before={}; directSnapshot(before);
  const bool oledChanged=before.display!=bleDraft.display;
  // BLE does not enter the USB/physical preview state; do not override a running preview.
  if (oledChanged && page==PAGE_OLED_CONFIRM) {
    bleReply("ERR|OLED PREVIEW ACTIVE"); return;
  }
  const bool chipChanged=before.reserved[0]!=bleDraft.reserved[0];
  const bool powerChanged=before.power!=bleDraft.power;
  if (chipChanged && powerChanged) { bleReply("ERR|CHANGE CHIP SEPARATELY"); return; }
  if (powerChanged && !bleSwitchHardwarePower(bleDraft.power)) {
    bleReply("ERR|RF POWER REJECTED");return;
  }
  const bool timersChanged = before.autoMode!=bleDraft.autoMode ||
      before.positionMode!=bleDraft.positionMode || before.fixedMin!=bleDraft.fixedMin ||
      before.parkMin!=bleDraft.parkMin ||
      memcmp(before.intervals,bleDraft.intervals,sizeof(before.intervals))!=0 ||
      strcmp(before.callsign,bleDraft.callsign)!=0;
  bleApplyRecord(bleDraft);
  const uint8_t oldChip=loraChip;
  loraChip=bleDraft.reserved[0];
  // Save the requested driver in the SAME A/B transaction as all other settings.
  // Initialize the new OLED only after flash verification succeeds.
  oledType=bleDraft.display;
  if (!directWrite()) {
    loraChip=oldChip;
    oledType=before.display;
    bleApplyRecord(before);
    if (powerChanged)bleSwitchHardwarePower(before.power);
    bleReply("ERR|FLASH VERIFY FAILED");return;
  }
  if (oledChanged) {
    // BLE direct-save: no 20s preview, no confirmation countdown.
    // If the driver is wrong, BLE/USB or BOOT 3s recovery can restore the screen.
    initOled(oledType);
    wakeOled();
    Serial.print("[OLED] BLE DRIVER SAVED: ");
    Serial.println(oledType==1 ? "SH1106" : "SSD1306");
  }
  if (chipChanged) {
    // Driver remains configured for previous chip until power cycle.
    radio->standby();loraReady=false;
    Serial.println("[CHIP] SAVED. RESTART REQUIRED. RF TX disabled until restart.");
  }
  bleCloseWriteWindow(); // Kayit sonrasi kaydetme tekrar KAPALI.
  if (timersChanged)resetAutoTimers();
  if (before.wxMin!=wxIntervalMin)wxDueAt=wxIntervalMin?millis()+(uint32_t)wxIntervalMin*60000UL:0;
  if (before.statusMin!=statusIntervalMin)statusDueAt=statusIntervalMin?millis()+(uint32_t)statusIntervalMin*60000UL:0;
  if (before.commentMin!=commentIntervalMin)lastCommentTxAt=0;
  if (before.sleepMode!=oledSleepMode)wakeOled();
  char ack[48];snprintf(ack,sizeof(ack),"ACK|SAVE|%lu",(unsigned long)directSeq);
  bleReply(ack);
  bleReply("LOCK|CLOSED");
}

void bleStart() {
  Bluefruit.begin();
  Bluefruit.setName("nRF52_Lora_Tracker");
  Bluefruit.setTxPower(4);
  ta1fmBleUart.begin();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(ta1fmBleUart);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(160,320);
  Bluefruit.Advertising.start(0);
  Serial.println("[BLE] Advertising: nRF52_Lora_Tracker (NUS, SAVE SWITCH V0.9.22)");
}

void bleProcessCommand() {
  if (!bleCommandLen) return;
  bleCommand[bleCommandLen]=0;
  // No PIN: explicit enable switch required for flash writes.
  Serial.print("[BLE RX] ");Serial.println(bleCommand);
  if (!strcmp(bleCommand,"info")||!strcmp(bleCommand,"p"))bleSendStatus();
  else if (!strcmp(bleCommand,"ping"))bleReply("PONG|TA1FM|V0.9.22");
  else if (!strcmp(bleCommand,"cfg?")) { bleCfgReadIndex=0;bleReply("ACK|READ|START"); }
  else if (!strcmp(bleCommand,"cfg lock?")) bleReplyLockState();
  else if (!strcmp(bleCommand,"cfg lock")) {
    bleCloseWriteWindow(); bleReply("LOCK|CLOSED");
    Serial.println("[BLE] EDIT LOCKED (web)");
  }
  else if (!strcmp(bleCommand,"cfg enable on")) {
    bleSavingEnabled=true;
    bleReplyLockState();
    Serial.println("[BLE] SAVE ENABLED (no PIN)");
  }
  else if (!strcmp(bleCommand,"cfg enable off")) {
    bleCloseWriteWindow();bleReplyLockState();
  }
  else if (!strcmp(bleCommand,"cfg begin")) {
    if (!bleWriteUnlocked())bleReply("ERR|LOCKED|OPEN ON WEB PANEL");
    else if (!directActive)bleReply("ERR|DIRECT INACTIVE");
    else {directSnapshot(bleDraft);bleDraftSeq=directSeq;bleDraftValid=true;bleReply("ACK|BEGIN|READY");}
  }
  else if (!strcmp(bleCommand,"cfg cancel")) {bleDraftValid=false;bleReply("ACK|CANCEL|OK");}
  else if (!strcmp(bleCommand,"cfg save"))bleCommitDraft();
  else if (!strncmp(bleCommand,"set ",4)) {
    if (!bleWriteUnlocked())bleReply("ERR|LOCKED|OPEN ON WEB PANEL");
    else if (!bleDraftValid)bleReply("ERR|NO TRANSACTION");
    else {
      char *key=bleCommand+4; char *space=strchr(key,' ');
      if (!space||!space[1])bleReply("ERR|BAD SET COMMAND");
      else {
        *space=0;
        if (!bleStageValue(key,space+1))bleReply("ERR|INVALID KEY OR VALUE");
        else {char ack[64];snprintf(ack,sizeof(ack),"ACK|%s|STAGED",key);bleReply(ack);}
      }
    }
  }
  else bleReply("ERR|UNKNOWN COMMAND");
  bleCommandLen=0;
}

void blePoll() {
  const uint32_t now=millis();
  const bool connected=Bluefruit.connected();
  if (connected!=bleWasConnected) {
    bleWasConnected=connected;
    bleTxLen=bleTxOffset=0;bleCommandLen=0;
    bleCfgReadIndex=-1;bleCloseWriteWindow();
    bleLastChunkMs=now;bleLastReportMs=now;
    Serial.println(connected?"[BLE] CONNECTED":"[BLE] DISCONNECTED");
  }
  // No PIN or countdown; write switch resets OFF at disconnect and after save.
  while (ta1fmBleUart.available()) {
    char ch=(char)ta1fmBleUart.read();
    if (ch=='\r'||ch=='\n')bleProcessCommand();
    else if (ch>=32&&ch<=126) {
      if (bleCommandLen<sizeof(bleCommand)-1) {
        bleCommand[bleCommandLen++]=ch;bleCommandLastByteMs=millis();
      } else {bleCommandLen=0;bleReply("ERR|COMMAND TOO LONG");}
    }
  }
  if (bleCommandLen && (uint32_t)(millis()-bleCommandLastByteMs)>=BLE_COMMAND_IDLE_MS)
    bleProcessCommand();
  if (connected&&ta1fmBleUart.notifyEnabled()) {
    if (bleCfgReadIndex>=0 && bleTxOffset==bleTxLen)bleCfgSendNext();
    if (bleCfgReadIndex<0 && !bleDraftValid &&
        (uint32_t)(now-bleLastReportMs)>=10000UL) {
      bleLastReportMs=now;bleSendStatus();
    }
    if (bleTxOffset<bleTxLen && (uint32_t)(now-bleLastChunkMs)>=BLE_TX_PACE_MS) {
      const size_t remain=bleTxLen-bleTxOffset;
      const size_t n=remain<BLE_TX_CHUNK_BYTES?remain:BLE_TX_CHUNK_BYTES;
      const size_t sent=ta1fmBleUart.write((const uint8_t*)bleTxQueue+bleTxOffset,n);
      if (sent==n) {
        bleTxOffset+=(uint16_t)sent;bleLastChunkMs=now;
        if (bleTxOffset>=bleTxLen)bleTxLen=bleTxOffset=0;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(700);
  Serial.println("\nTA1FM nrf52 Tracker UNIVERSAL BLE SAVE SWITCH V0.9.22");
  Serial.println("AUTO: first saved callsign enables TX; a off preserves user choice.");

  pinMode(BTN_PIN, INPUT_PULLUP);
  // Boot-only recovery: if button is already held as device powers up,
  // hold it continuously for 3 seconds to toggle the display type.
  // A normal long press after boot remains 1.2 seconds.
  const bool bootHeld = digitalRead(BTN_PIN) == LOW;
  bool recoveryRequested = false;
  if (bootHeld) {
    uint32_t started = millis();
    while (digitalRead(BTN_PIN) == LOW && millis() - started < BOOT_RECOVERY_MS) {
      delay(5);
    }
    recoveryRequested = (digitalRead(BTN_PIN) == LOW && millis() - started >= BOOT_RECOVERY_MS);
  }
  // Mark boot-time press as handled: it must not open menus on release.
  buttonRaw = buttonStable = digitalRead(BTN_PIN);
  buttonChangedAt = millis();
  longHandled = bootHeld;
  pressBeganAt = millis();

  pinMode(GPS_PWR_PIN, OUTPUT);
  digitalWrite(GPS_PWR_PIN, GPS_PWR_ACTIVE_LEVEL);
  Serial1.begin(GPS_BAUD);

  Wire.begin(); // Kart varyanti: SDA=P1.04 / SCL=P0.11
  Wire.setClock(100000);
  analogReadResolution(12);
  analogReference(AR_INTERNAL_3_0);
  pinMode(VBAT_PIN, INPUT);
  delay(120);

  loadConfig();
  resetAutoTimers();
  if (recoveryRequested) {
    oledType = (oledType == 1) ? 2 : 1;
    saveConfig();
    Serial.println("Boot rescue: display driver toggled");
  }
  readBattery();
  initOled(oledType);
  // Intro is short; GPS keeps buffering in UART during LoRa startup.
  showIntro();
  const uint32_t introStarted = millis();
  probeBme();
  readBme();
  initLora();
  while (millis() - introStarted < 3000UL) {
    while (Serial1.available()) gps.encode(Serial1.read());
    pollLora();
    delay(2);
  }
  bleStart();
  oledLastWakeAt = millis();
  drawOled();
  Serial.println("USB: UNIVERSAL V0.9.22 | chip? | oled? | oled sh1106|ssd1306 | oled confirm|cancel|wake | sleep 0..6 | flash? | p");
}

void loop() {
  // Her dongude GPS verilerini bosalt; menu GPS'i durdurmaz.
  while (Serial1.available()) gps.encode(Serial1.read());
  pollButton();
  pollLora();
  pollAutoBeacon();
  blePoll();
  const uint32_t now = millis();

  while (Serial.available()) {
    char ch = (char)Serial.read();
    if (ch == '\r' || ch == '\n') {
      if (usbLineLen) {
        usbLine[usbLineLen] = 0;
        if (strcmp(usbLine, "b") == 0) {
          if (!bmeReady) probeBme();
          readBme();
          Serial.print("BME: "); Serial.print(bmeReady ? "OK" : "NONE");
          if (bmeValuesOK) { Serial.print(" C="); Serial.print(tempC,1); Serial.print(" RH="); Serial.print(humidity,1); Serial.print(" hPa="); Serial.print(pressureHPa,1); }
          Serial.println();
        } else if (strcmp(usbLine, "g") == 0) {
          Serial.print("GPS chars="); Serial.print(gps.charsProcessed());
          Serial.print(" fix="); Serial.print(gpsFix());
          Serial.print(" sat="); Serial.println(gps.satellites.isValid()?gps.satellites.value():0);
        } else if (strcmp(usbLine, "r") == 0) {
          Serial.print("[LORA] "); Serial.print(loraReady ? "RX OK" : "ERR");
          Serial.print(" code="); Serial.print(loraError);
          Serial.print(" received="); Serial.println(loraPackets);
        } else if (strcmp(usbLine, "u") == 0) {
          utcOffset = (utcOffset == 0) ? 3 : 0; saveConfig();
          Serial.println(utcOffset == 0 ? "UTC" : "UTC+3");
        } else handleUsbCommand(usbLine);
        usbLineLen = 0;
      }
    } else if (usbLineLen < sizeof(usbLine) - 1) {
      usbLine[usbLineLen++] = ch;
    } else {
      usbLineLen = 0; // Ignore oversized line, never process partial command.
      Serial.println("[USB] Command too long");
    }
  }

  if (page == PAGE_OLED_CONFIRM && now - oledConfirmStarted >= oledConfirmWindowMs) {
    initOled(previousOledType);
    pendingOledType = previousOledType;
    returnToMenu();
    oledPreviewFromUsb = false;
    Serial.println("[OLED] ONIZLEME SURESI DOLDU; eski surucuye donuldu");
    oledUsbReport();
  }
  if (page != PAGE_MAIN && page != PAGE_OLED_CONFIRM && now - lastUserAction >= MENU_TIMEOUT_MS) {
    page = PAGE_MAIN;
    oledLastWakeAt = now;
  }
  if (now - lastBattery >= 5000) { lastBattery = now; readBattery(); }
  if (now - lastBme >= 3000) { lastBme = now; readBme(); }
  if (now - lastBmeProbe >= 30000) { lastBmeProbe = now; probeBme(); }
  pollOledStandby(now);
  if (now - lastDraw >= 500) { lastDraw = now; drawOled(); }
  if (now - lastLog >= 15000) {
    lastLog = now;
    Serial.print("GPS fix="); Serial.print(gpsFix());
    Serial.print("  uydu="); Serial.print(gps.satellites.isValid() ? gps.satellites.value() : 0);
    Serial.print("  PIL~"); Serial.print(batteryV, 2);
    Serial.print("V  BME="); Serial.print(bmeReady ? "VAR" : "YOK");
    if (bmeValuesOK) {
      Serial.print(" C="); Serial.print(tempC, 1);
      Serial.print(" RH="); Serial.print(humidity, 1);
      Serial.print(" hPa="); Serial.print(pressureHPa, 1);
    }
    Serial.println();
  }
}
