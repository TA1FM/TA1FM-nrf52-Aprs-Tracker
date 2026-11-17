/*
  TA1FM nrf52 Tracker - UNIVERSAL BLE V0.9.25 BETA-4 (APRS USB MSG TX)
  Temel: V0.9.14 SX1268; Direct Flash A/B ve CRC32 sistemi korunur.
  TEST: SX1262 / SX1268 secimi; mevcut GPS/OLED/APRS islevleri korunur.
  BLE: 18 bayt NUS, ayar okuma, sifresiz kaydetme anahtari, tek Direct Flash commit.
  APRS mesaj RX + OLED MENU + USB TEK MESAJ RF TX (ACK/RETRY YOK).
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
  Park sembolu korunur; WX bagimsizdir. LoRa RF TX korumasi 10 saniye.
  Callsign, OLED, smart ve RF ayarlari Direct Flash A/B kaydinda korunur.
  Tum ayarlar Direct Flash A/B tek kaydinda saklanir.
  RF TEST 2: SX1262 + SX1268 OCP yuksek guc icin 140 mA; diger ozellikler aynen korunur.
  AYRI TEST SURUMU: PA GIRISINE BAGLAMADAN ONCE 50 OHM YUKTE KONTROL EDIN.
  V0.9.24 BETA: DIGI tekrar filtresi 30 sn, USB seri cikti duzeni,
  DIGI TX dogrulama/sayaclari, DIGI RX kontrollu toparlanma.
  RF TEST 2 OCP 140 mA (SX1262/SX1268), Direct Flash A/B schema 2,
  BLE UART V0.9.23 ayar protokolu ve web panel uyumlulugu korunur.
  Derleme ve donanim testleri kullanici tarafinda dogrulanmalidir.

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
  uint32_t crc;  // v1 CRC at unchanged offset; required for backward compatible reads
  // V0.9.23 schema 2 extension; old schema 1 A/B records remain readable.
  uint8_t bleEnabled;  // 0=off, 1=on, default 1
  uint8_t stationMode; // 0=TRACKER, 1=FIXED DIGI
  uint8_t digiBeaconMin; // 10/15/30/60, default 15
  uint8_t digiPath;    // 1=WIDE1-1 only, 2=also WIDE2-1/2
  int32_t digiLatE5;   // fixed latitude x100000
  int32_t digiLonE5;   // fixed longitude x100000
  uint32_t extCrc;     // CRC over full v2 record (excluding extCrc)
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
// Ortak TX korumasi: Tum RF paketleri arasinda en az 10 saniye.
static const uint32_t POS_TX_GAP_MS = 10000UL;
static const uint32_t AUX_TX_GAP_MS = 10000UL;

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
bool loraInitialized = false; // Basarili begin/CRC/OCP sonrasinda RX yeniden deneme izni.
bool loraRecoverySuppressed = false; // BLE chip degisikligi: restart edilene kadar RF kapali.
int16_t loraError = 0;
uint32_t loraPackets = 0;
uint32_t loraAprsPackets = 0;
float lastRssi = 0;
float lastSnr = 0;
uint32_t lastLoraRxAt = 0;
char lastHeardCall[12] = "---";
void onLoraRx() { loraRxFlag = true; }

// RF TEST 2: Set 140 mA OCP for SX1262 AND SX1268 after begin().
// Semtech SX1262 allows 140 mA OCP; the earlier SX1268-only RF TEST 1
// restored RF output on a tested MiniMesh SX1268 board.
// RF output can increase substantially: measure into rated 50-ohm load first.
// Keep existing DIO2 and RXEN RF switch routing unchanged.
static const float TA1FM_RF_TEST_OCP_MA = 140.0f;
static const int16_t TA1FM_RF_TEST_OCP_ERROR = -32000;

bool ta1fmRfTestApplyOcp(const char* phase) {
  if (!radio) return false;
  if (loraChip != CHIP_SX1262 && loraChip != CHIP_SX1268) {
    Serial.println("[RF TEST 2] Unsupported LoRa chip: OCP not changed");
    return false;
  }
  const float before = radio->getCurrentLimit();
  const int16_t rc = radio->setCurrentLimit(TA1FM_RF_TEST_OCP_MA);
  const float after = radio->getCurrentLimit();
  Serial.print("[RF TEST 2] "); Serial.print(loraChipName());
  Serial.print(" OCP "); Serial.print(phase);
  Serial.print(" before="); Serial.print(before, 1);
  Serial.print(" mA requested="); Serial.print(TA1FM_RF_TEST_OCP_MA, 1);
  Serial.print(" mA readback="); Serial.print(after, 1);
  Serial.print(" mA rc="); Serial.println(rc);
  return rc == RADIOLIB_ERR_NONE &&
         after >= TA1FM_RF_TEST_OCP_MA - 1.25f &&
         after <= TA1FM_RF_TEST_OCP_MA + 1.25f;
}

// RadioLib setOutputPower() may update PA configuration. Always reapply
// OCP afterwards, including changes from the OLED, USB and BLE panel.
int16_t ta1fmRfTestSetOutputPower(int8_t level) {
  const int16_t rc = radio->setOutputPower(level);
  if (rc != RADIOLIB_ERR_NONE) return rc;
  if (!ta1fmRfTestApplyOcp("after-power")) return TA1FM_RF_TEST_OCP_ERROR;
  return RADIOLIB_ERR_NONE;
}
// V0.9.23: DIGI and BLE runtime configuration must be declared before
// digiOnReceived/pollDigi/pollFixedDigiBeacon. Arduino auto-prototypes
// cover functions, but not global variables declared later in the sketch.
// Runtime settings: never clear previously saved tracker configuration.
bool bleEnabled=true;
uint8_t stationMode=0; // 0=TRACKER, 1=SABIT DIGI
uint8_t digiBeaconMin=15;
uint8_t digiPath=1;
int32_t digiLatE5=0, digiLonE5=0;
static bool directActive = false;

// DIGI declarations; implementation below RX handler.
bool validateCall(const char *input);
bool manualAprsTx();
// APRS message beta RX only (no message TX or ACK)
void ta1fmMessageRX(const uint8_t* frame, size_t length, bool simulated);
bool ta1fmMessageUsb(const char* command);
void digiOnReceived(const uint8_t* data, size_t len);
void pollDigi();
void pollFixedDigiBeacon();
void bleApplyRadioState();

void initLora() {
  loraReady = false;
  loraInitialized = false;
  loraRecoverySuppressed = false;
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
  const bool ocpOk = ta1fmRfTestApplyOcp("init");
  Serial.print("[LORA] DIO2 switch="); Serial.print(sw);
  Serial.print(" CRC="); Serial.println(crc);
  if (sw != RADIOLIB_ERR_NONE || crc != RADIOLIB_ERR_NONE || !ocpOk) {
    loraError = (sw != RADIOLIB_ERR_NONE) ? sw :
                (crc != RADIOLIB_ERR_NONE ? crc : TA1FM_RF_TEST_OCP_ERROR);
    Serial.println("[RF TEST 2] Radio TX disabled: init check failed");
    return;
  }
  radio->setDio1Action(onLoraRx);
  loraInitialized = true;
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
      digiOnReceived(data, packetLen);
      ta1fmMessageRX(data, packetLen, false);
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


// APRS digipeater V0.9.24: bounded queue and 30-sec source+payload
// duplicate suppression. Same WIDE parser, RF gap and saved settings.
static const uint8_t DIGI_QUEUE_MAX=4;
static const uint32_t DIGI_DUPLICATE_WINDOW_MS=30000UL; // Once TX OK sonrasi; kuyruktaki ayni icerik zaten engellenir.
struct DigiPending { uint8_t frame[256]; uint16_t size; uint32_t readyAt, addedAt; uint32_t hash; bool used; };
DigiPending digiQueue[DIGI_QUEUE_MAX]={};
struct DigiSeen { uint32_t hash, at; };
DigiSeen digiSeen[16]={};
uint8_t digiSeenNext=0;
uint32_t digiRxEligible=0, digiTxOk=0, digiTxErrors=0;
uint32_t digiDupes=0, digiRejected=0, digiQueueFull=0, digiExpired=0;
uint32_t digiRxRecoverOk=0, digiRxRecoverErrors=0;
uint32_t digiRxRetryAt=0;
uint32_t digiNextBeacon=0;

static uint32_t digiHash(const char* src, size_t slen, const char* info, size_t ilen) {
  uint32_t hash=2166136261UL;
  for(size_t i=0;i<slen;i++){hash^=(uint8_t)toupper((unsigned char)src[i]);hash*=16777619UL;}
  hash^=0x3A; hash*=16777619UL;
  for(size_t i=0;i<ilen;i++){hash^=(uint8_t)info[i];hash*=16777619UL;}
  return hash;
}
void digiOnReceived(const uint8_t* frame, size_t len) {
  if (stationMode!=1 || !callsignValid || !directActive || !loraReady ||
      len<16 || len>252 || memcmp(frame,APRS_HEADER,3)!=0) return;
  char packet[253]={};
  size_t n=len-3;
  for(size_t i=0;i<n;i++){
    if(frame[i+3]<32 || frame[i+3]>126) { digiRejected++; return; }
    packet[i]=(char)frame[i+3];
  }
  const char* colon=strchr(packet,':');
  const char* greater=strchr(packet,'>');
  if (!colon || !greater || greater==packet || greater>colon ||
      (greater-packet)>11 || colon==packet+n-1) { digiRejected++; return; }
  // Do not relay own packets or malformed frame paths.
  char source[12]={}; memcpy(source,packet,(size_t)(greater-packet));
  if (!validateCall(source) || strcmp(source,stationCall)==0) {digiRejected++;return;}
  const size_t ilen=strlen(colon+1);
  uint32_t hash=digiHash(packet,(size_t)(greater-packet),colon+1,ilen);
  const uint32_t now=millis();
  for(const DigiSeen &e:digiSeen) if(e.hash==hash && (uint32_t)(now-e.at)<DIGI_DUPLICATE_WINDOW_MS) {digiDupes++;return;}
  for(const DigiPending &e:digiQueue) if(e.used && e.hash==hash) {digiDupes++;return;}
  // Parse routing path. Reject hops already marked via * and non-matching WIDE aliases.
  // Source>Destination[,path...] : data ; match first available WIDEn-N only.
  const char* destStart=greater+1;
  const char* comma=strchr(destStart,',');
  size_t destLen=(size_t)((comma && comma<colon ? comma:colon)-destStart);
  if(destLen<3 || destLen>9) {digiRejected++;return;}
  char pathOut[130]={}; size_t used=0; bool repeated=false;
  const char* cursor=destStart+destLen;
  while(cursor<colon && *cursor==',') {
    cursor++;
    const char* end=cursor;
    while(end<colon && *end!=',')end++;
    size_t tlen=(size_t)(end-cursor);
    if(!tlen || tlen>20) {digiRejected++;return;}
    char token[22]={}; memcpy(token,cursor,tlen);
    if(strncmp(token,stationCall,strlen(stationCall))==0 && (token[strlen(stationCall)]=='*' || token[strlen(stationCall)]=='\0')) {digiRejected++;return;}
    if(!strcmp(token,"TCPIP*")||!strcmp(token,"TCPXX*")||!strcmp(token,"NOGATE")||!strcmp(token,"RFONLY")){digiRejected++;return;}
    // Only WIDE1-1, WIDE2-1, WIDE2-2 are repeated, with at most one hop consumed.
    bool acceptable=(!strcmp(token,"WIDE1-1") ||
       (digiPath==2 && (!strcmp(token,"WIDE2-1") || !strcmp(token,"WIDE2-2"))));
    char replacement[40]={};
    if(!repeated && acceptable) {
      if(!strcmp(token,"WIDE2-2"))snprintf(replacement,sizeof(replacement),"%s*,WIDE2-1",stationCall);
      else snprintf(replacement,sizeof(replacement),"%s*",stationCall);
      repeated=true;
    } else {
      strncpy(replacement,token,sizeof(replacement)-1);
    }
    size_t rl=strlen(replacement);
    if(used+rl+1>=sizeof(pathOut)){digiRejected++;return;}
    pathOut[used++]=',';memcpy(pathOut+used,replacement,rl);used+=rl;
    cursor=end;
  }
  if(!repeated){digiRejected++;return;}
  char newPacket[253]={};
  int pktLen=snprintf(newPacket,sizeof(newPacket),"%.*s>%.*s%s:%s",
       (int)(greater-packet),packet,(int)destLen,destStart,pathOut,colon+1);
  if(pktLen<=0 || pktLen>249){digiRejected++;return;}
  for(DigiPending &q:digiQueue)if(!q.used){
    memset(&q,0,sizeof(q));
    memcpy(q.frame,APRS_HEADER,3);memcpy(q.frame+3,newPacket,(size_t)pktLen);
    q.size=(uint16_t)pktLen+3;q.readyAt=now+400UL+(hash%500UL);
    q.addedAt=now;q.hash=hash;q.used=true;digiRxEligible++;
    Serial.print("[DIGI QUEUE] ");Serial.println(newPacket);
    return;
  }
  digiQueueFull++; Serial.println("[DIGI] QUEUE FULL - dropped");
}
void pollDigi(){
  if(stationMode!=1 || !loraReady || !radio || !callsignValid)return;
  const uint32_t now=millis();
  // Oldest eligible queued packet has priority over our fixed beacon.
  for(DigiPending &q:digiQueue){
    if(!q.used)continue;
    if((uint32_t)(now-q.addedAt)>45000UL){
      q.used=false; ++digiExpired;
      Serial.println("[DIGI] QUEUE EXPIRED (TX not attempted)");
      continue;
    }
    if((int32_t)(now-q.readyAt)<0)continue;
    if(everTransmitted && (uint32_t)(now-lastTxAt)<POS_TX_GAP_MS) return;
    const uint32_t hash=q.hash;
    // Basarili paket kaydi ancak radio->transmit() olumlu dondukten sonra yazilir.
    // RadioLib OK, yalnizca yerel RF TX sonucudur; iGate alimini garanti etmez.
    loraRxFlag=false;
    radio->standby();lastTxAt=now;everTransmitted=true;
    int16_t rc=radio->transmit(q.frame,q.size);
    if(rc==RADIOLIB_ERR_NONE){
      ++digiTxOk;
      digiSeen[digiSeenNext].hash=hash;digiSeen[digiSeenNext].at=millis();
      digiSeenNext=(digiSeenNext+1)%16;
      Serial.print("[DIGI TX OK] "); Serial.write(q.frame+3,q.size-3);Serial.println();
    }else {
      ++digiTxErrors;
      Serial.print("[DIGI TX ERROR] code=");Serial.println(rc);
    }
    q.used=false;
    loraRxFlag=false;loraError=radio->startReceive();loraReady=loraError==RADIOLIB_ERR_NONE;
    if(!loraReady){Serial.print("[DIGI RX] RX restart error=");Serial.println(loraError);}
    return;
  }
}

// DIGI only: if RX restart failed after a transmit/read, try again in the
// regular non-blocking loop. Does not reinitialize radio or alter TX power.
// A BLE chip change deliberately disables RF until reboot: never bypass it.
void pollDigiRxRecovery(){
  if(stationMode!=1 || loraReady || !loraInitialized || !radio || loraRecoverySuppressed) return;
  const uint32_t now=millis();
  if((int32_t)(now-digiRxRetryAt)<0) return;
  digiRxRetryAt=now+3000UL;
  loraRxFlag=false;
  const int16_t rc=radio->startReceive();
  loraError=rc;
  if(rc==RADIOLIB_ERR_NONE){
    loraReady=true;
    ++digiRxRecoverOk;
    Serial.println("[DIGI RX] RX restored");
  }else{
    ++digiRxRecoverErrors;
    if(digiRxRecoverErrors<=3 || digiRxRecoverErrors%10==0){
      Serial.print("[DIGI RX] retry failed code=");Serial.println(rc);
    }
  }
}

void printDigiCounters(){
  Serial.print("[DIGI] Eligible=");Serial.print(digiRxEligible);
  Serial.print(" TX=");Serial.print(digiTxOk);
  Serial.print(" TX_ERR=");Serial.print(digiTxErrors);
  Serial.print(" DUP=");Serial.print(digiDupes);
  Serial.print(" REJECT=");Serial.println(digiRejected);
  Serial.print("[DIGI] QUEUE_FULL=");Serial.print(digiQueueFull);
  Serial.print(" EXPIRED=");Serial.print(digiExpired);
  Serial.print(" RX_RECOVER=");Serial.print(digiRxRecoverOk);
  Serial.print(" RX_RECOVER_ERR=");Serial.println(digiRxRecoverErrors);
}

// Fixed DIGI always uses a stored position and a separate 10/15/30/60 min
// beacon timer. Previous tracker SMART/FIXED preferences remain untouched.
void pollFixedDigiBeacon(){
  if(stationMode!=1 || !callsignValid || !loraReady || !directActive ||
      digiLatE5==0 || digiLonE5==0)return;
  uint32_t now=millis();
  if(!digiNextBeacon) digiNextBeacon=now+5000UL;
  if((int32_t)(now-digiNextBeacon)<0)return;
  for(const DigiPending& q:digiQueue)if(q.used)return;
  if(everTransmitted && (uint32_t)(now-lastTxAt)<POS_TX_GAP_MS)return;
  if(manualAprsTx()){
    digiNextBeacon=now+(uint32_t)digiBeaconMin*60000UL;
    Serial.println("[DIGI] Own fixed beacon sent");
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

// PAGE_MAIN / PAGE_MENU / PAGE_BME / PAGE_CLOCK / PAGE_BLE / PAGE_OLED
// Saat ve ekran secimi alt sayfalarda kisa basla degisir,
// uzun basla onaylanarak ana menuye donulur.
enum Page { PAGE_MAIN, PAGE_MENU, PAGE_BME, PAGE_CLOCK, PAGE_BLE, PAGE_LORA, PAGE_INFO, PAGE_OLED, PAGE_OLED_CONFIRM, PAGE_AUTO, PAGE_SYMBOL, PAGE_SLEEP, PAGE_TX_POWER, PAGE_MESSAGES, PAGE_MSG_DELETE_CONFIRM };
Page page = PAGE_MAIN;
uint8_t menuIndex = 0;
int8_t pendingUtc = 3;
uint8_t pendingOledType = 1;
uint8_t pendingAutoWxMode = 0;
bool pendingBleEnabled=true;
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
uint8_t commentIntervalMin = 15; // 0=OFF, 1=EACH POS, 15/30/60=minutes
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
static constexpr uint32_t DF_SCHEMA = 2UL;
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
  r.bleEnabled=bleEnabled ? 1:0;
  r.stationMode=stationMode;
  r.digiBeaconMin=digiBeaconMin;
  r.digiPath=digiPath;
  r.digiLatE5=digiLatE5;
  r.digiLonE5=digiLonE5;
  r.crc=directCrc(&r,offsetof(DirectRecord,crc));
  r.extCrc=directCrc(&r,offsetof(DirectRecord,extCrc));
}

bool directValid(uint32_t addr, DirectRecord& r) {
  if (addr != DF_PAGE_A && addr != DF_PAGE_B) return false;
  memcpy(&r,(const void*)addr,sizeof(r));
  if (r.magic!=DF_MAGIC || (r.schema!=1UL && r.schema!=DF_SCHEMA) || r.seq==0 ||
      r.crc!=directCrc(&r,offsetof(DirectRecord,crc))) return false;
  // Schema 1 is verified exclusively against the original CRC layout.
  if (r.schema==DF_SCHEMA && (r.extCrc!=directCrc(&r,offsetof(DirectRecord,extCrc)) ||
      r.bleEnabled>1 || r.stationMode>1 ||
      (r.digiBeaconMin!=10 && r.digiBeaconMin!=15 && r.digiBeaconMin!=30 && r.digiBeaconMin!=60) ||
      (r.digiPath!=1 && r.digiPath!=2) ||
      r.digiLatE5 < -9000000L || r.digiLatE5 > 9000000L ||
      r.digiLonE5 < -18000000L || r.digiLonE5 > 18000000L)) return false;
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
  // Schema 1 migration: don't flash-write automatically on boot.
  bleEnabled = (r.schema==1UL) ? true : (r.bleEnabled!=0);
  stationMode = (r.schema==1UL) ? 0 : r.stationMode;
  digiBeaconMin = (r.schema==1UL) ? 15 : r.digiBeaconMin;
  digiPath = (r.schema==1UL) ? 1 : r.digiPath;
  digiLatE5 = (r.schema==1UL) ? 0 : r.digiLatE5;
  digiLonE5 = (r.schema==1UL) ? 0 : r.digiLonE5;
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
  Serial.print("[V23] mode=");Serial.print(stationMode?"DIGI":"TRACKER");
  Serial.print(" BLE=");Serial.print(bleEnabled?"ON":"OFF");
  Serial.print(" DIGImin=");Serial.print(digiBeaconMin);
  Serial.print(" PATH=");Serial.print(digiPath);
  Serial.print(" LAT=");Serial.print(digiLatE5/100000.0,5);
  Serial.print(" LON=");Serial.println(digiLonE5/100000.0,5);
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
  const int16_t result = ta1fmRfTestSetOutputPower(desired);
  if (result != RADIOLIB_ERR_NONE) {
    Serial.print("[TXPOWER] Radio rejected level, error "); Serial.println(result);
    radio->startReceive();
    return;
  }
  const int16_t rxResult = radio->startReceive();
  if (rxResult != RADIOLIB_ERR_NONE) {
    Serial.print("[TXPOWER] RX restore failed: "); Serial.println(rxResult);
    radio->standby(); ta1fmRfTestSetOutputPower(previous);
    loraReady = radio->startReceive() == RADIOLIB_ERR_NONE;
    return;
  }
  loraRxFlag = false;
  loraTxPowerDbm = desired;
  if (!saveTxPower()) {
    Serial.println("[TXPOWER] SAVE FAILED; restoring previous power");
    radio->standby(); ta1fmRfTestSetOutputPower(previous);
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

// ---------------------------------------------------------------------------
// TA1FM V0.9.25 BETA-2: APRS message RX, OLED inbox, USB injection.
// No message RF TX, no auto ACK, no Direct Flash layout changes.
// APRS format: :ADDRESSEE:TEXT{ID (ADDRESSEE is exactly 9 padded characters).
// ---------------------------------------------------------------------------
static const uint8_t TA1FM_MSG_CAPACITY = 10;
static const uint32_t TA1FM_MSG_DUP_WINDOW = 1800000UL;
struct Ta1fmMessage {
  char from[12], to[12], text[68], id[6];
  uint32_t at;
  bool simulated;
};
static Ta1fmMessage ta1fmInbox[TA1FM_MSG_CAPACITY] = {};
static uint8_t ta1fmInboxCount = 0, ta1fmInboxNext = 0;
static uint32_t ta1fmMsgRxCount = 0, ta1fmMsgDupes = 0;
static uint32_t ta1fmMsgBad = 0, ta1fmMsgOther = 0;
// OLED RAM-only inbox UI; no Direct Flash schema changes.
static uint8_t ta1fmInboxView = 0;
static uint8_t ta1fmUnread = 0;
static char ta1fmLastMsgFrom[12] = "---";

void ta1fmMessageRX(const uint8_t* frame, size_t length, bool simulated) {
  if (stationMode != 0 || !callsignValid || !frame || length < 3 || length > 255 ||
      memcmp(frame, APRS_HEADER, sizeof(APRS_HEADER)) != 0) return;
  size_t n = length - sizeof(APRS_HEADER);
  char pkt[256] = {};
  for (size_t i=0; i<n; ++i) {
    uint8_t c = frame[sizeof(APRS_HEADER)+i];
    if (c < 32 || c > 126) return;
    pkt[i] = (char)c;
  }
  const char* gt = strchr(pkt, '>');
  const char* colon = strchr(pkt, ':');
  if (!gt || !colon || gt == pkt || gt > colon || colon[1] != ':') return;
  const char* info = colon+1;
  if (strlen(info)<11 || info[10]!=':') { ++ta1fmMsgBad; return; }

  size_t fromLen = (size_t)(gt-pkt);
  if (fromLen<3 || fromLen>9) { ++ta1fmMsgBad; return; }
  char from[12]={};
  memcpy(from,pkt,fromLen);
  for(size_t i=0;i<fromLen;i++)from[i]=(char)toupper((unsigned char)from[i]);
  if (!validateCall(from)) { ++ta1fmMsgBad; return; }

  char to[12]={};
  memcpy(to,info+1,9);
  size_t toLen=9;
  while (toLen && to[toLen-1]==' ')to[--toLen]=0;
  for(size_t i=0;i<toLen;i++)to[i]=(char)toupper((unsigned char)to[i]);
  if (!validateCall(to)) { ++ta1fmMsgBad; return; }
  if (strcmp(to,stationCall)) { ++ta1fmMsgOther; return; }

  const char* raw=info+11;
  const char* brace=strchr(raw,'{');
  size_t textLen=brace?(size_t)(brace-raw):strlen(raw);
  if (!textLen || textLen>67) { ++ta1fmMsgBad; return; }
  char id[6]={};
  if (brace) {
    size_t idLen=strlen(brace+1);
    if (idLen<1 || idLen>5) { ++ta1fmMsgBad; return; }
    for(size_t i=0;i<idLen;i++)
      if (!isalnum((unsigned char)brace[1+i])) { ++ta1fmMsgBad; return; }
    memcpy(id,brace+1,idLen);
  }
  for(size_t i=0;i<textLen;i++)
    if (raw[i]=='{' || raw[i]=='|' || raw[i]=='~') { ++ta1fmMsgBad; return; }
  char text[68]={};
  memcpy(text,raw,textLen);

  // Protocol ACK/REJ is not a user message; later BETA will handle matching.
  if (!brace && textLen>=4 && textLen<=8 &&
      (!strncmp(text,"ack",3) || !strncmp(text,"rej",3))) {
    bool control=true;
    for(size_t i=3;i<textLen;i++)
      if(!isalnum((unsigned char)text[i]))control=false;
    if (control) {
      Serial.print(simulated?"[APRS MSG TEST CTRL] ":"[APRS MSG CTRL RX] ");
      Serial.print(from);Serial.print(" ");Serial.println(text);
      return;
    }
  }
  uint32_t now=millis();
  if (id[0])for(uint8_t i=0;i<ta1fmInboxCount;i++){
    const Ta1fmMessage& old=ta1fmInbox[i];
    if (!strcmp(old.from,from) && !strcmp(old.id,id) &&
        !strcmp(old.text,text) && (uint32_t)(now-old.at)<TA1FM_MSG_DUP_WINDOW) {
      ++ta1fmMsgDupes;
      Serial.print("[APRS MSG DUP] ");Serial.print(from);
      Serial.print(" ID=");Serial.println(id);
      return;
    }
  }
  Ta1fmMessage& m=ta1fmInbox[ta1fmInboxNext];
  memset(&m,0,sizeof(m));
  strncpy(m.from,from,sizeof(m.from)-1);
  strncpy(m.to,to,sizeof(m.to)-1);
  strncpy(m.text,text,sizeof(m.text)-1);
  strncpy(m.id,id,sizeof(m.id)-1);
  m.at=now;m.simulated=simulated;
  ta1fmInboxNext=(uint8_t)((ta1fmInboxNext+1)%TA1FM_MSG_CAPACITY);
  if(ta1fmInboxCount<TA1FM_MSG_CAPACITY)++ta1fmInboxCount;
  ++ta1fmMsgRxCount;
  if (ta1fmUnread < TA1FM_MSG_CAPACITY) ++ta1fmUnread;
  strncpy(ta1fmLastMsgFrom, from, sizeof(ta1fmLastMsgFrom)-1);
  ta1fmLastMsgFrom[sizeof(ta1fmLastMsgFrom)-1] = 0;
  if (page == PAGE_MAIN && oledSleepMode != 1) wakeOled();
  Serial.println(simulated?"[APRS MSG TEST] ACCEPTED - NO RF TX":
                           "[APRS MSG RX] NEW MESSAGE");
  Serial.print("[APRS MSG] FROM : ");Serial.println(from);
  Serial.print("[APRS MSG] TO   : ");Serial.println(to);
  Serial.print("[APRS MSG] TEXT : ");Serial.println(text);
  Serial.print("[APRS MSG] ID   : ");Serial.println(id[0]?id:"(none)");
  if(id[0])Serial.println("[APRS MSG] AUTO ACK DISABLED IN BETA-3");
}
void ta1fmMessageClearInbox() {
  memset(ta1fmInbox, 0, sizeof(ta1fmInbox));
  ta1fmInboxCount=0;
  ta1fmInboxNext=0;
  ta1fmInboxView=0;
  ta1fmUnread=0;
  ta1fmLastMsgFrom[0]='-';
  ta1fmLastMsgFrom[1]='-';
  ta1fmLastMsgFrom[2]='-';
  ta1fmLastMsgFrom[3]=0;
  Serial.println("[APRS MSG] RAM INBOX CLEARED - FLASH UNCHANGED");
}
void ta1fmMessageList(){
  Serial.print("[APRS MSG] INBOX ");Serial.print(ta1fmInboxCount);
  Serial.print("/");Serial.println(TA1FM_MSG_CAPACITY);
  for(uint8_t i=0;i<ta1fmInboxCount;i++){
    uint8_t j=(uint8_t)((ta1fmInboxNext+TA1FM_MSG_CAPACITY-1-i)%TA1FM_MSG_CAPACITY);
    const Ta1fmMessage& m=ta1fmInbox[j];
    Serial.print("[MSG ");Serial.print(i+1);Serial.print("] ");
    if(m.simulated)Serial.print("(SIM) ");
    Serial.print(m.from);Serial.print(" -> ");Serial.print(m.to);
    Serial.print(" ID=");Serial.print(m.id[0]?m.id:"-");
    Serial.print(" TIME+");Serial.print(m.at/1000UL);
    Serial.print("s TEXT=");Serial.println(m.text);
  }
  if(!ta1fmInboxCount)Serial.println("[APRS MSG] EMPTY");
}

// ---------------------------------------------------------------------------
// TA1FM V0.9.25 BETA-4: one-shot addressed APRS RF message TX from USB.
// This is UNACKNOWLEDGED APRS (no {ID), so no retries and no receipt claims.
// Exactly one pending message; existing 10 s shared TX guard is respected.
// No outgoing Flash storage, no BLE remote-transmit, no GPS fix requirement.
// ----------------------------------------------------------------------------
enum Ta1fmTxState {
  TA1FM_TX_NONE, TA1FM_TX_QUEUED, TA1FM_TX_LOCAL_OK,
  TA1FM_TX_RADIO_ERROR, TA1FM_TX_TIMEOUT, TA1FM_TX_CANCELLED
};
static Ta1fmTxState ta1fmMsgTxState=TA1FM_TX_NONE;
static char ta1fmMsgTxPacket[155]={};
static char ta1fmMsgTxTo[12]={};
static uint16_t ta1fmMsgTxLen=0;
static uint32_t ta1fmMsgTxQueuedAt=0;
static uint32_t ta1fmMsgTxCount=0;
static int16_t ta1fmMsgTxLastError=0;
static const uint32_t TA1FM_MSG_TX_TTL_MS=30000UL;

const char* ta1fmMsgTxStateName(){
  switch(ta1fmMsgTxState){
    case TA1FM_TX_QUEUED: return "QUEUED";
    case TA1FM_TX_LOCAL_OK: return "TX_OK_LOCAL_ONLY";
    case TA1FM_TX_RADIO_ERROR: return "TX_RADIO_ERROR";
    case TA1FM_TX_TIMEOUT: return "QUEUE_TIMEOUT";
    case TA1FM_TX_CANCELLED: return "CANCELLED";
    default: return "NONE";
  }
}
void ta1fmMsgTxStatus(){
  Serial.print("[APRS MSG TX] STATE=");Serial.print(ta1fmMsgTxStateName());
  Serial.print(" TO=");Serial.print(ta1fmMsgTxTo[0]?ta1fmMsgTxTo:"-");
  Serial.print(" SENT_COUNT=");Serial.print(ta1fmMsgTxCount);
  Serial.print(" LAST_RADIO_CODE=");Serial.println(ta1fmMsgTxLastError);
  if(ta1fmMsgTxState==TA1FM_TX_LOCAL_OK)
    Serial.println("[APRS MSG TX] TX OK is only a LOCAL RadioLib result, not delivery confirmation");
}
bool ta1fmMsgTxBuild(const char* input, bool queueIt){
  if(!callsignValid || !validateCall(stationCall)){
    Serial.println("[APRS MSG TX] BLOCKED - SET VALID LOCAL CALLSIGN");return false;
  }
  if(stationMode!=0){
    Serial.println("[APRS MSG TX] BLOCKED - TRACKER MODE ONLY");return false;
  }
  if(!input || !*input){
    Serial.println("[APRS MSG TX] Usage: msg preview TA1FM-8 SELAM KANKA");
    Serial.println("[APRS MSG TX] Usage: msg send TA1FM-8 SELAM KANKA");
    return false;
  }
  const char* space=strchr(input,' ');
  if(!space || space==input || !space[1]){
    Serial.println("[APRS MSG TX] Expected CALL-SSID + space + TEXT");return false;
  }
  const size_t toLen=(size_t)(space-input);
  if(toLen>9){Serial.println("[APRS MSG TX] Target too long");return false;}
  char destination[12]={};
  memcpy(destination,input,toLen);
  if(!validateCall(destination)){
    Serial.println("[APRS MSG TX] INVALID TARGET (UPPERCASE CALL-SSID)");return false;
  }
  if(!strcmp(destination,stationCall)){
    Serial.println("[APRS MSG TX] Target is our own call - use another station");return false;
  }
  const char* text=space+1;
  const size_t length=strlen(text);
  if(length==0 || length>67){
    Serial.println("[APRS MSG TX] Text length 1..67 ASCII characters required");return false;
  }
  for(size_t i=0;i<length;i++){
    const unsigned char ch=(unsigned char)text[i];
    if(ch<32 || ch>126 || ch=='|' || ch=='~' || ch=='{'){
      Serial.println("[APRS MSG TX] Printable ASCII only, no | ~ {");return false;
    }
  }
  char packet[155]={};
  const int n=snprintf(packet,sizeof(packet),
        "%s>APZTFM,WIDE1-1::%-9s:%s",stationCall,destination,text);
  if(n<1 || n>=(int)sizeof(packet) || n>249){
    Serial.println("[APRS MSG TX] Encoded frame too long");return false;
  }
  Serial.print("[APRS MSG TX] FRAME: ");Serial.println(packet);
  if(!queueIt){
    Serial.println("[APRS MSG TX] PREVIEW ONLY - NO RF TX");
    return true;
  }
  if(ta1fmMsgTxState==TA1FM_TX_QUEUED){
    Serial.println("[APRS MSG TX] BUSY - use msg status or msg cancel");return false;
  }
  if(!loraReady || !radio){
    Serial.println("[APRS MSG TX] BLOCKED - RADIO NOT READY");return false;
  }
  memset(ta1fmMsgTxPacket,0,sizeof(ta1fmMsgTxPacket));
  memcpy(ta1fmMsgTxPacket,packet,(size_t)n);
  ta1fmMsgTxLen=(uint16_t)n;
  strncpy(ta1fmMsgTxTo,destination,sizeof(ta1fmMsgTxTo)-1);
  ta1fmMsgTxTo[sizeof(ta1fmMsgTxTo)-1]=0;
  ta1fmMsgTxQueuedAt=millis();
  ta1fmMsgTxState=TA1FM_TX_QUEUED;
  ta1fmMsgTxLastError=0;
  Serial.println("[APRS MSG TX] QUEUED FOR ONE RF TRANSMISSION - NO RETRY / NO ACK");
  return true;
}
bool ta1fmMessageTxUsb(const char* cmd){
  if(!strcmp(cmd,"msg status")){
    ta1fmMsgTxStatus();return true;
  }
  if(!strcmp(cmd,"msg cancel")){
    if(ta1fmMsgTxState==TA1FM_TX_QUEUED){
      ta1fmMsgTxState=TA1FM_TX_CANCELLED;
      ta1fmMsgTxLen=0;
      Serial.println("[APRS MSG TX] QUEUED MESSAGE CANCELLED - NO RF TX");
    } else Serial.println("[APRS MSG TX] NOTHING PENDING TO CANCEL");
    return true;
  }
  if(!strncmp(cmd,"msg preview ",12)){
    ta1fmMsgTxBuild(cmd+12,false);return true;
  }
  if(!strncmp(cmd,"msg send ",9)){
    ta1fmMsgTxBuild(cmd+9,true);return true;
  }
  if(!strcmp(cmd,"msg send")||!strcmp(cmd,"msg preview")){
    ta1fmMsgTxBuild(nullptr,false);return true;
  }
  return false;
}
void ta1fmPollMessageTx(){
  if(ta1fmMsgTxState!=TA1FM_TX_QUEUED)return;
  uint32_t now=millis();
  if((uint32_t)(now-ta1fmMsgTxQueuedAt)>TA1FM_MSG_TX_TTL_MS){
    ta1fmMsgTxState=TA1FM_TX_TIMEOUT;
    ta1fmMsgTxLen=0;
    Serial.println("[APRS MSG TX] QUEUE TIMEOUT; NO RF TX");
    return;
  }
  if(!callsignValid || stationMode!=0){
    ta1fmMsgTxState=TA1FM_TX_CANCELLED;
    ta1fmMsgTxLen=0;
    Serial.println("[APRS MSG TX] CANCELLED - CALLSIGN/MODE CHANGED");
    return;
  }
  if(!loraReady || !radio)return;
  if(everTransmitted && (uint32_t)(now-lastTxAt)<AUX_TX_GAP_MS)return;
  // Send one LoRa APRS frame. Existing TX uses 3C FF 01 prefix and RX restart.
  if(!ta1fmMsgTxLen || ta1fmMsgTxLen>sizeof(ta1fmMsgTxPacket)){
    ta1fmMsgTxState=TA1FM_TX_RADIO_ERROR;
    ta1fmMsgTxLastError=-1;
    return;
  }
  uint8_t frame[160]={};
  memcpy(frame,APRS_HEADER,sizeof(APRS_HEADER));
  memcpy(frame+sizeof(APRS_HEADER),ta1fmMsgTxPacket,ta1fmMsgTxLen);
  const size_t bytes=sizeof(APRS_HEADER)+ta1fmMsgTxLen;
  ta1fmMsgTxState=TA1FM_TX_RADIO_ERROR; // never transmit twice after failed TX.
  lastTxAt=now;
  everTransmitted=true;
  loraRxFlag=false;
  radio->standby();
  txLastCode=radio->transmit(frame,bytes);
  ta1fmMsgTxLastError=txLastCode;
  if(txLastCode==RADIOLIB_ERR_NONE){
    ++txSuccesses;
    ++ta1fmMsgTxCount;
    ta1fmMsgTxState=TA1FM_TX_LOCAL_OK;
    Serial.print("[APRS MSG TX] TX OK (LOCAL) TO=");Serial.println(ta1fmMsgTxTo);
    Serial.println("[APRS MSG TX] Recipient delivery NOT CONFIRMED; no ID/ACK");
    showMessage("MSG TX OK LOCAL");
  } else {
    Serial.print("[APRS MSG TX] RADIO ERROR=");Serial.println(txLastCode);
    showMessage("MSG TX ERROR");
  }
  ta1fmMsgTxLen=0;
  loraRxFlag=false;
  loraError=radio->startReceive();
  loraReady=loraError==RADIOLIB_ERR_NONE;
  if(!loraReady){
    Serial.print("[APRS MSG TX] RX RESTART FAILED=");Serial.println(loraError);
  }
}

bool ta1fmMessageUsb(const char* cmd){
  if(ta1fmMessageTxUsb(cmd))return true;
  if(!strcmp(cmd,"msg?") || !strcmp(cmd,"testmsg")){
    Serial.println("[APRS MSG V0.9.25 BETA-4] RX/OLED + SINGLE-SHOT USB TX; NO AUTO ACK");
    Serial.println("[CMDS] testmsg CALL TEXT | msg list | msg clear | msg?");
    Serial.println("[CMDS] msg preview CALL TEXT | msg send CALL TEXT | msg status | msg cancel");
    Serial.print("[MSG] received=");Serial.print(ta1fmMsgRxCount);
    Serial.print(" duplicates=");Serial.print(ta1fmMsgDupes);
    Serial.print(" malformed=");Serial.print(ta1fmMsgBad);
    Serial.print(" other=");Serial.println(ta1fmMsgOther);
    return true;
  }
  if(!strcmp(cmd,"msg list")) {ta1fmMessageList();return true;}
  if(!strcmp(cmd,"msg clear")){
    ta1fmMessageClearInbox();
    return true;
  }
  if(!strncmp(cmd,"testmsg ",8)){
    if(stationMode!=0 || !callsignValid){
      Serial.println("[APRS MSG TEST] Set valid station callsign / TRACKER mode first");
      return true;
    }
    const char* start=cmd+8;
    const char* split=strchr(start,' ');
    if(!split || split==start){
      Serial.println("[APRS MSG TEST] Usage: testmsg TA1FM-9 SELAM KANKA");
      return true;
    }
    size_t fromLen=(size_t)(split-start);
    if(fromLen>9){Serial.println("[APRS MSG TEST] BAD SOURCE");return true;}
    char from[12]={};
    memcpy(from,start,fromLen);
    if(!validateCall(from)){
      Serial.println("[APRS MSG TEST] BAD SOURCE CALL-SSID (uppercase required)");
      return true;
    }
    const char* message=split+1;
    size_t len=strlen(message);
    if(!len || len>67 || strchr(message,'{') ||
       strchr(message,'|') || strchr(message,'~')){
      Serial.println("[APRS MSG TEST] Text must be 1..67 ASCII, no { | ~");
      return true;
    }
    for(size_t i=0;i<len;i++)if((unsigned char)message[i]<32 ||
                              (unsigned char)message[i]>126){
      Serial.println("[APRS MSG TEST] Printable ASCII only");return true;
    }
    char asciiFrame[160]={};
    int written=snprintf(asciiFrame,sizeof(asciiFrame),
          "%s>APZTFM,WIDE1-1::%-9s:%s{01",from,stationCall,message);
    if(written<=0 || written>152){
      Serial.println("[APRS MSG TEST] OVERLONG FRAME");return true;
    }
    uint8_t frame[160]={};
    memcpy(frame,APRS_HEADER,sizeof(APRS_HEADER));
    memcpy(frame+sizeof(APRS_HEADER),asciiFrame,(size_t)written);
    Serial.println("[APRS MSG TEST] Inject local frame WITHOUT RF TX...");
    ta1fmMessageRX(frame,sizeof(APRS_HEADER)+(size_t)written,true);
    return true;
  }
  return false;
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
  if (!callsignValid) return false;
  const bool fixedDigi=stationMode==1;
  if(fixedDigi && (digiLatE5==0 || digiLonE5==0))return false;
  if(!fixedDigi && (!gps.location.isValid() || gps.location.age() >= 10000))return false;
  const double lat = fixedDigi ? (double)digiLatE5/100000.0 : gps.location.lat();
  const double lon = fixedDigi ? (double)digiLonE5/100000.0 : gps.location.lng();
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
      (commentIntervalMin==1 || !lastCommentTxAt ||
       (uint32_t)(millis()-lastCommentTxAt)>=(uint32_t)commentIntervalMin*60000UL);
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
    showMessage("WX WAIT 10s"); Serial.println("[WX BLOCK] 10s auxiliary TX guard"); return false;
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
    Serial.println("[WX TX] RadioLib TX OK");
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
  if (stationMode==0 && !gpsFix()) { showMessage("NO GPS FIX"); Serial.println("[TX BLOCK] GPS location not fresh"); return false; }
  if (!loraReady) { showMessage("LORA NOT READY"); Serial.println("[TX BLOCK] LoRa RX not ready"); return false; }
  const uint32_t now = millis();
  if (everTransmitted && (uint32_t)(now - lastTxAt) < POS_TX_GAP_MS) {
    showMessage("POS WAIT 10s"); Serial.println("[TX BLOCK] 10s position TX guard"); return false;
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
    if (stationMode==0 && gps.course.isValid() && gps.course.age() < 5000)
      sentCourse = gps.course.deg();
    showMessage("TX OK (LOCAL)");
    Serial.println("[TX] RadioLib TX OK");
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
    Serial.println("[STATUS TX] WAIT 10s"); return false;
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
  if (stationMode==1 || autoWxMode==0 || !callsignValid || !loraReady) return;
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
  // First boot: GPS position already carries APRS Comment; after 10s send Status.
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
  if (ta1fmMessageUsb(line)) return;
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

  if(strcmp(line,"mode?")==0 || strcmp(line,"digi?")==0 || strcmp(line,"ble?")==0){
    Serial.print("[V23] mode=");Serial.print(stationMode?"DIGI":"TRACKER");
    Serial.print(" BLE=");Serial.print(bleEnabled?"ON":"OFF");
    Serial.print(" DIGImin=");Serial.print(digiBeaconMin);
    Serial.print(" PATH=");Serial.print(digiPath);
    Serial.print(" LAT=");Serial.print(digiLatE5/100000.0,5);
    Serial.print(" LON=");Serial.println(digiLonE5/100000.0,5);
    printDigiCounters();
    return;
  }
  if(!strncmp(line,"ble ",4)){
    if(strcmp(line+4,"on") && strcmp(line+4,"off")){Serial.println("Usage: ble on|off");return;}
    bool prev=bleEnabled;bleEnabled=!strcmp(line+4,"on");
    if(!directWrite()){bleEnabled=prev;Serial.println("[BLE] SAVE FAILED");return;}
    bleApplyRadioState();Serial.println(bleEnabled?"[BLE] ON SAVED":"[BLE] OFF SAVED");return;
  }
  if(!strncmp(line,"mode ",5)){
    if(strcmp(line+5,"tracker") && strcmp(line+5,"digi")){Serial.println("Usage: mode tracker|digi");return;}
    uint8_t v=(!strcmp(line+5,"digi"))?1:0;
    if(v==1 && (!callsignValid || !digiLatE5 || !digiLonE5)){
      Serial.println("[DIGI] Set valid callsign and BOTH coordinates first");return;
    }
    uint8_t prev=stationMode;stationMode=v;
    if(!directWrite()){stationMode=prev;Serial.println("[MODE] SAVE FAILED");return;}
    digiNextBeacon=0;resetAutoTimers();
    Serial.println(stationMode?"[MODE] FIXED DIGI":"[MODE] TRACKER");return;
  }
  if(!strncmp(line,"digi beacon ",12)){
    int v=atoi(line+12);
    if(v!=10 && v!=15 && v!=30 && v!=60){Serial.println("Usage: digi beacon 10|15|30|60");return;}
    uint8_t prev=digiBeaconMin;digiBeaconMin=(uint8_t)v;
    if(!directWrite()){digiBeaconMin=prev;Serial.println("[DIGI] SAVE FAILED");return;}
    digiNextBeacon=0;Serial.println("[DIGI] beacon interval saved");return;
  }
  if(!strncmp(line,"digi path ",10)){
    int v=atoi(line+10);
    if(v!=1 && v!=2){Serial.println("Usage: digi path 1|2");return;}
    uint8_t prev=digiPath;digiPath=(uint8_t)v;
    if(!directWrite()){digiPath=prev;Serial.println("[DIGI] SAVE FAILED");return;}
    Serial.println("[DIGI] path saved");return;
  }
  if(!strncmp(line,"digi lat ",9) || !strncmp(line,"digi lon ",9)){
    bool isLat=line[5]=='l' && line[6]=='a';
    const char* p=line+9;char* end=nullptr;
    double v=strtod(p,&end);
    if(!end||end==p||*end!='\0'||!isfinite(v)||fabs(v)>(isLat?90.0:180.0)){
       Serial.println("Usage: digi lat 40.97500 / digi lon 27.50400");return;
    }
    int32_t scaled=(int32_t)lround(v*100000.0);
    int32_t prev=isLat?digiLatE5:digiLonE5;
    if(isLat)digiLatE5=scaled;else digiLonE5=scaled;
    if(!directWrite()){
      if(isLat)digiLatE5=prev;else digiLonE5=prev;
      Serial.println("[DIGI] SAVE FAILED");return;
    }
    digiNextBeacon=0;Serial.println("[DIGI] coordinate saved");return;
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
    else if (strcmp(line+2,"each")==0) commentIntervalMin=1;
    else if (strcmp(line+2,"15")==0) commentIntervalMin=15;
    else if (strcmp(line+2,"30")==0) commentIntervalMin=30;
    else if (strcmp(line+2,"60")==0) commentIntervalMin=60;
    else { Serial.println("Usage: j off|each|15|30|60"); return; }
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
    // [V23] is retained intentionally: existing USB web panel parses this protocol marker.
    Serial.print("[V23] mode=");Serial.print(stationMode?"DIGI":"TRACKER");
    Serial.print(" BLE=");Serial.print(bleEnabled?"ON":"OFF");
    Serial.print(" DIGImin=");Serial.print(digiBeaconMin);
    Serial.print(" PATH=");Serial.print(digiPath);
    Serial.print(" LAT=");Serial.print(digiLatE5/100000.0,5);
    Serial.print(" LON=");Serial.println(digiLonE5/100000.0,5);
    if(stationMode==1) printDigiCounters();
    Serial.println("[CMDS] a on|off | f smart|fixed | i 1|3|5|10|15 | x off|15|30|60 | z off|15|30|60 | j off|each|15|30|60");
    Serial.println("[CMDS] smart 600,90,45,30,20,15 | motion on|off | batt on|off | smart?");
    Serial.println("[CMDS] chip? | chip sx1262 | chip sx1268 | flash? | flash init");
    Serial.println("[CMDS] ble on|off | mode tracker|digi | mode? | digi lat NUM | digi lon NUM | digi beacon 10|15|30|60 | digi path 1|2");
    Serial.println("[CMDS] oled? | oled sh1106|ssd1306 | oled confirm|cancel|wake | sleep 0..6");
    Serial.println("[CMDS] comment TEXT | status TEXT | comment? | status? | t POS | w WX | v STATUS | symbol /> | s car|walker|house | power 1..22 | power? | p");
    Serial.println("[CMDS] testmsg CALL TEXT | msg list | msg clear | msg? | msg preview CALL TEXT | msg send CALL TEXT | msg status | msg cancel");
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
bool bmeAbsentLogged = false;  // BME absent: log only on missing->present->missing transitions.
bool gpsLogInitialized = false;
bool lastGpsFixLogged = false;
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
      bmeAbsentLogged = false;
      Serial.print("BME280 hazir, I2C adresi: 0x");
      Serial.println(addr, HEX);
      return;
    }
  }
  if(!bmeAbsentLogged){
    bmeAbsentLogged=true;
    Serial.println("BME280 bulunamadi; BME menusu gizli.");
  }
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

uint8_t menuCount() { return bmeReady ? 12 : 11; }

const char *menuItem(uint8_t item) {
  if (bmeReady) {
    switch (item) {
      case 0: return "MESSAGES";
      case 1: return "BME280 WX";
      case 2: return "AUTO BEACON";
      case 3: return "POS SYMBOL";
      case 4: return "UTC OFFSET";
      case 5: return "DISPLAY";
      case 6: return "OLED TIMER";
      case 7: return "TX POWER";
      case 8: return "BLE ON/OFF";
      case 9: return "LORA INFO";
      case 10: return "DEVICE INFO";
      default: return "EXIT";
    }
  }
  switch (item) {
    case 0: return "MESSAGES";
    case 1: return "AUTO BEACON";
    case 2: return "POS SYMBOL";
    case 3: return "UTC OFFSET";
    case 4: return "DISPLAY";
    case 5: return "OLED TIMER";
    case 6: return "TX POWER";
    case 7: return "BLE ON/OFF";
    case 8: return "LORA INFO";
    case 9: return "DEVICE INFO";
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
  gfx->setCursor(48, 55); gfx->print("V0.9.25");
  oledFlush();
}

void drawOled() {
  if (!oledReady || !gfx || oledSleeping) return;
  gfx->fillScreen(0);
  gfx->setTextColor(SSD1306_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(0, 0);
  gfx->print(stationCall);  // NOCALL until user saves a valid callsign
  // 2-character unread marker fits between callsign and clock.
  if (page == PAGE_MAIN && ta1fmUnread) {
    gfx->setCursor(60, 0); gfx->print('M');
    if (ta1fmUnread < 10) gfx->print(ta1fmUnread);
    else gfx->print('+');
  }
  if (page == PAGE_MAIN) drawTime();
  else {
    // 7 karakter * 6 px = 42 px; 84 + 42 = 126 <= 128.
    // 91 px'de son karakter otomatik alt satira tasiyordu.
    gfx->setCursor(84, 0);
    gfx->print("V0.9.25");
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

  // Persistent notification: stays until the inbox is opened or cleared.
  // OLED standby still works; after wake this same notice is redrawn.
  if (page == PAGE_MAIN && ta1fmUnread && ta1fmInboxCount) {
    gfx->setCursor(0, 17); gfx->print("NEW APRS MESSAGE");
    gfx->setCursor(0, 30); gfx->print("FROM: "); gfx->print(ta1fmLastMsgFrom);
    gfx->setCursor(0, 45); gfx->print("TAP: READ NOW");
    gfx->setCursor(0, 56); gfx->print("HOLD: MENU");
    oledFlush();
    return;
  }

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
  } else if (page == PAGE_MESSAGES) {
    if (!ta1fmInboxCount) {
      gfx->setCursor(0, 17); gfx->print("APRS MESSAGES");
      gfx->setCursor(0, 33); gfx->print("INBOX EMPTY");
      gfx->setCursor(0, 53); gfx->print("HOLD: BACK");
    } else if (ta1fmInboxView >= ta1fmInboxCount) {
      // Last item after the newest-to-oldest messages.
      gfx->setCursor(0, 16); gfx->print("APRS MESSAGES");
      gfx->setCursor(0, 30); gfx->print("> DELETE ALL");
      gfx->setCursor(0, 43); gfx->print(ta1fmInboxCount); gfx->print(" MESSAGES IN RAM");
      gfx->setCursor(0, 56); gfx->print("HOLD: SELECT");
    } else {
      const uint8_t pos = (uint8_t)(
        (ta1fmInboxNext + TA1FM_MSG_CAPACITY - 1 - ta1fmInboxView) %
        TA1FM_MSG_CAPACITY);
      const Ta1fmMessage &msg = ta1fmInbox[pos];
      gfx->setCursor(0, 14);
      gfx->print(ta1fmInboxView + 1); gfx->print('/');
      gfx->print(ta1fmInboxCount); gfx->print(" FROM:");
      gfx->print(msg.from);
      if (msg.simulated) gfx->print('*');
      const size_t msgLen = strlen(msg.text);
      for (uint8_t row = 0; row < 4; ++row) {
        gfx->setCursor(0, (int16_t)(25 + row * 10));
        for (uint8_t col = 0; col < 21; ++col) {
          const size_t at = (size_t)row * 21U + col;
          if (at >= msgLen) break;
          gfx->write((uint8_t)msg.text[at]);
        }
      }
    }
  } else if (page == PAGE_MSG_DELETE_CONFIRM) {
    gfx->setCursor(0, 15); gfx->print("DELETE ALL MESSAGES?");
    gfx->setCursor(0, 31); gfx->print(ta1fmInboxCount);
    gfx->print(" MSG WILL BE LOST");
    gfx->setCursor(0, 46); gfx->print("TAP: CANCEL");
    gfx->setCursor(0, 57); gfx->print("HOLD: DELETE");
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
  } else if (page == PAGE_BLE) {
    gfx->setCursor(0, 15); gfx->print("BLE ON/OFF");
    gfx->setCursor(0, 29); gfx->print("> ");gfx->print(pendingBleEnabled?"ON":"OFF");
    gfx->setCursor(0, 46); gfx->print("TAP: CHANGE");
    gfx->setCursor(0, 57); gfx->print("HOLD: SAVE");
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
    gfx->setCursor(0, 49); gfx->print("FW: V0.9.25");
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
      if (ta1fmUnread && ta1fmInboxCount) {
        // A single tap opens the newest unread message instead of TX.
        ta1fmInboxView = 0;
        ta1fmUnread = 0;
        page = PAGE_MESSAGES;
        Serial.println("[APRS MSG OLED] Opened unread inbox with one tap");
      } else {
        manualAprsTx(); // Existing beacon shortcut, unchanged with no unread mail.
      }
      break;
    case PAGE_MENU:
      menuIndex = (menuIndex + 1) % menuCount();
      break;
    case PAGE_MESSAGES:
      if (ta1fmInboxCount)
        // Cycle through all messages, then one extra DELETE ALL item.
        ta1fmInboxView = (uint8_t)((ta1fmInboxView + 1) % (ta1fmInboxCount + 1));
      break;
    case PAGE_MSG_DELETE_CONFIRM:
      page = PAGE_MESSAGES; // Cancellation never deletes.
      Serial.println("[APRS MSG OLED] Deletion cancelled");
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
    case PAGE_BLE:
      pendingBleEnabled=!pendingBleEnabled;
      break;
    case PAGE_OLED:
      pendingOledType = (pendingOledType == 1) ? 2 : 1;
      break;
    case PAGE_BME:
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
      if (strcmp(selected, "MESSAGES") == 0) {
        ta1fmInboxView = 0;
        ta1fmUnread = 0;
        page = PAGE_MESSAGES;
      } else if (strcmp(selected, "BME280 WX") == 0) page = PAGE_BME;
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
      } else if (strcmp(selected, "BLE ON/OFF") == 0) { pendingBleEnabled=bleEnabled;page=PAGE_BLE; }
      else if (strcmp(selected, "LORA INFO") == 0) page = PAGE_LORA;
      else if (strcmp(selected, "DEVICE INFO") == 0) page = PAGE_INFO;
      else page = PAGE_MAIN;
      break;
    }
    case PAGE_BME:
    case PAGE_LORA:
    case PAGE_INFO:
      returnToMenu();
      break;
    case PAGE_MESSAGES:
      if (ta1fmInboxCount && ta1fmInboxView >= ta1fmInboxCount) {
        page = PAGE_MSG_DELETE_CONFIRM; // First hold only opens confirmation.
        Serial.println("[APRS MSG OLED] Delete all requested; HOLD again to confirm");
      } else {
        returnToMenu();
      }
      break;
    case PAGE_MSG_DELETE_CONFIRM:
      // Second deliberate long hold deletes all RAM messages.
      ta1fmMessageClearInbox();
      page = PAGE_MESSAGES;
      Serial.println("[APRS MSG OLED] Delete all confirmed");
      break;
    case PAGE_BLE: {
      bool previous=bleEnabled;
      bleEnabled=pendingBleEnabled;
      if (!directWrite()) {bleEnabled=previous;showMessage("BLE SAVE FAILED");}
      else {bleApplyRadioState();showMessage(bleEnabled?"BLE ON":"BLE OFF");}
      returnToMenu();
      break;
    }
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
void bleCloseWriteWindow();
static bool bleStackReady=false;
static uint32_t bleOffPendingAt=0; // allow final ACK to leave NUS before disconnect
void bleApplyRadioState(){
  if(!bleStackReady)return;
  Bluefruit.Advertising.restartOnDisconnect(bleEnabled);
  if(bleEnabled){
    if(!Bluefruit.connected() && !Bluefruit.Advertising.isRunning())Bluefruit.Advertising.start(0);
    Serial.println("[BLE] RADIO ON / advertising");
  }else{
    bleCloseWriteWindow();
    if(Bluefruit.connected())bleOffPendingAt=millis()+1200UL;
    else {Bluefruit.Advertising.stop();Serial.println("[BLE] RADIO OFF / advertising stopped");}
  }
}

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
      // With unread mail, the FIRST short click must open messages even if
      // the OLED slept. With no unread mail preserve the old wake-only click.
      consumeWakeClick = oledSleeping &&
          !(page == PAGE_MAIN && ta1fmUnread && ta1fmInboxCount);
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

// BLE UART V0.9.23 SAVE SWITCH - USB/GPS/LoRa ana akisi korunur.
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
    case 0: snprintf(line,sizeof(line),"CFG|BEGIN|V0.9.23"); break;
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
    case 22: snprintf(line,sizeof(line),"CFG|bleEnabled=%s",bleEnabled?"on":"off"); break;
    case 23: snprintf(line,sizeof(line),"CFG|stationMode=%s",stationMode?"digi":"tracker");break;
    case 24: snprintf(line,sizeof(line),"CFG|digiBeaconMin=%u",digiBeaconMin);break;
    case 25: snprintf(line,sizeof(line),"CFG|digiPath=%u",digiPath);break;
    case 26: snprintf(line,sizeof(line),"CFG|digiLat=%.5f",digiLatE5/100000.0);break;
    case 27: snprintf(line,sizeof(line),"CFG|digiLon=%.5f",digiLonE5/100000.0);break;
    case 28: snprintf(line,sizeof(line),"CFG|END"); break;
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

  if (!strcmp(key,"bleEnabled")) {
    if(strcmp(v,"on") && strcmp(v,"off"))return false;
    bleDraft.bleEnabled=!strcmp(v,"on");return true;
  }
  if (!strcmp(key,"stationMode")) {
    if(strcmp(v,"tracker") && strcmp(v,"digi"))return false;
    bleDraft.stationMode=!strcmp(v,"digi")?1:0;return true;
  }
  if (!strcmp(key,"digiBeaconMin")) {
    if(!bleParseNumber(v,n,10,60)||(n!=10&&n!=15&&n!=30&&n!=60))return false;
    bleDraft.digiBeaconMin=(uint8_t)n;return true;
  }
  if (!strcmp(key,"digiPath")) {
    if(!bleParseNumber(v,n,1,2))return false;
    bleDraft.digiPath=(uint8_t)n;return true;
  }
  if (!strcmp(key,"digiLat") || !strcmp(key,"digiLon")) {
    char* end=nullptr;double d=strtod(v,&end);
    bool isLat=!strcmp(key,"digiLat");
    if(!end||end==v||*end!='\0'||!isfinite(d)||fabs(d)>(isLat?90.0:180.0))return false;
    if(isLat)bleDraft.digiLatE5=(int32_t)lround(d*100000.0);
    else bleDraft.digiLonE5=(int32_t)lround(d*100000.0);
    return true;
  }
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
  if (!strcmp(key,"commentMin")) {
    if (!strcmp(v,"each") || !strcmp(v,"1")) { bleDraft.commentMin=1; return true; }
    return bleParseInterval(v,bleDraft.commentMin);
  }
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
  bleEnabled=r.bleEnabled!=0;
  stationMode=r.stationMode;
  digiBeaconMin=r.digiBeaconMin;
  digiPath=r.digiPath;
  digiLatE5=r.digiLatE5;digiLonE5=r.digiLonE5;
}

bool bleSwitchHardwarePower(int8_t target) {
  if (!loraReady || !radio) return false;
  const int8_t previous=loraTxPowerDbm;
  radio->standby();
  if (ta1fmRfTestSetOutputPower(target)!=RADIOLIB_ERR_NONE) {
    loraReady=radio->startReceive()==RADIOLIB_ERR_NONE;return false;
  }
  if (radio->startReceive()!=RADIOLIB_ERR_NONE) {
    radio->standby();ta1fmRfTestSetOutputPower(previous);
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
      bleDraft.reserved[2]<'!' || bleDraft.reserved[2]>'~' ||
      bleDraft.bleEnabled>1 || bleDraft.stationMode>1 ||
      (bleDraft.digiBeaconMin!=10 && bleDraft.digiBeaconMin!=15 &&
       bleDraft.digiBeaconMin!=30 && bleDraft.digiBeaconMin!=60) ||
      (bleDraft.digiPath!=1 && bleDraft.digiPath!=2) ||
      bleDraft.digiLatE5 < -9000000L || bleDraft.digiLatE5 > 9000000L ||
      bleDraft.digiLonE5 < -18000000L || bleDraft.digiLonE5 > 18000000L ||
      (bleDraft.stationMode && (!bleDraft.digiLatE5 || !bleDraft.digiLonE5))) {
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
  const bool bleChanged=before.bleEnabled!=bleDraft.bleEnabled;
  const bool digiChanged=before.stationMode!=bleDraft.stationMode ||
      before.digiBeaconMin!=bleDraft.digiBeaconMin ||
      before.digiLatE5!=bleDraft.digiLatE5 || before.digiLonE5!=bleDraft.digiLonE5;
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
    loraRecoverySuppressed=true;
    Serial.println("[CHIP] SAVED. RESTART REQUIRED. RF TX disabled until restart.");
  }
  bleCloseWriteWindow(); // Kayit sonrasi kaydetme tekrar KAPALI.
  if (timersChanged)resetAutoTimers();
  if (before.wxMin!=wxIntervalMin)wxDueAt=wxIntervalMin?millis()+(uint32_t)wxIntervalMin*60000UL:0;
  if (before.statusMin!=statusIntervalMin)statusDueAt=statusIntervalMin?millis()+(uint32_t)statusIntervalMin*60000UL:0;
  if (before.commentMin!=commentIntervalMin)lastCommentTxAt=0;
  if (before.sleepMode!=oledSleepMode)wakeOled();
  if (digiChanged) {digiNextBeacon=0;resetAutoTimers();}
  // The ACK is queued before BLE is disconnected by blePoll().
  if (bleChanged && bleEnabled)bleApplyRadioState();
  char ack[48];snprintf(ack,sizeof(ack),"ACK|SAVE|%lu",(unsigned long)directSeq);
  bleReply(ack);
  bleReply("LOCK|CLOSED");
  if (bleChanged && !bleEnabled)bleOffPendingAt=millis()+1200UL;
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
  Bluefruit.Advertising.restartOnDisconnect(bleEnabled);
  Bluefruit.Advertising.setInterval(160,320);
  bleStackReady=true;
  if(bleEnabled) Bluefruit.Advertising.start(0);
  Serial.println(bleEnabled?"[BLE] Advertising: nRF52_Lora_Tracker":"[BLE] OFF from saved config");
}

void bleProcessCommand() {
  if (!bleCommandLen) return;
  bleCommand[bleCommandLen]=0;
  // No PIN: explicit enable switch required for flash writes.
  Serial.print("[BLE RX] ");Serial.println(bleCommand);
  if (!strcmp(bleCommand,"info")||!strcmp(bleCommand,"p"))bleSendStatus();
  else if (!strcmp(bleCommand,"ping"))bleReply("PONG|TA1FM|V0.9.25-B4");
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
  if(!bleEnabled && bleOffPendingAt && (int32_t)(millis()-bleOffPendingAt)>=0){
    bleOffPendingAt=0;
    Bluefruit.Advertising.restartOnDisconnect(false);
    if(Bluefruit.connected())Bluefruit.disconnect(Bluefruit.connHandle());
    Bluefruit.Advertising.stop();
    Serial.println("[BLE] OFF, connection closed");
  }
}

void setup() {
  Serial.begin(115200);
  delay(700);
  Serial.println("\nTA1FM nrf52 Tracker UNIVERSAL BLE V0.9.25 BETA-4 USB MSG TX");
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
  Serial.println("USB: UNIVERSAL V0.9.25 BETA-4 | chip? | oled? | oled sh1106|ssd1306 | oled confirm|cancel|wake | sleep 0..6 | flash? | p");
}

void loop() {
  // Her dongude GPS verilerini bosalt; menu GPS'i durdurmaz.
  while (Serial1.available()) gps.encode(Serial1.read());
  pollButton();
  pollLora();
  pollDigiRxRecovery();
  pollDigi();
  pollFixedDigiBeacon();
  ta1fmPollMessageTx(); // single pending RF message; priority over AUTO beacon
  if(ta1fmMsgTxState!=TA1FM_TX_QUEUED) pollAutoBeacon();
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
  const bool currentGpsFix=gpsFix();
  if(!gpsLogInitialized || currentGpsFix!=lastGpsFixLogged){
    gpsLogInitialized=true;
    lastGpsFixLogged=currentGpsFix;
    Serial.println(currentGpsFix ? "[GPS] FIX OK" : "[GPS] FIX WAIT/LOST");
  }
  if (now - lastLog >= 60000UL) {
    lastLog = now;
    Serial.print("GPS fix="); Serial.print(currentGpsFix);
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
