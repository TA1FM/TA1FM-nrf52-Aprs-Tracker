/*
  TA1FM nrf52 Tracker - V0.9.2
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
  Parking state for /P symbol, but never gates WX. Hard 60 s RF guard.
  Callsign, old OLED settings and old v08 auto file retained.
  New beacon text/config is stored in separate LittleFS file.
  Not yet compiled on user's Arduino or tested on physical tracker.

*/

#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <RadioLib.h>

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
// Shared RF cooldown: Position requests may use a 10s minimum, auxiliary
// WX/Status requests need 30s after ANY TX. Actual beacon intervals are longer.
static const uint32_t POS_TX_GAP_MS = 10000UL;
static const uint32_t AUX_TX_GAP_MS = 30000UL;

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
static const int8_t LORA_TX_POWER_DBM = 10; // Ilk test: dusuk guc
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


SX1262 radio = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);
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
  Serial.println("[LORA] SX1262 / manual POS+WX TX test, 10 dBm");
  SPI.begin();
  // begin(frequency,bw,sf,cr,syncWord, power, preambleLength, tcxoVoltage)
  loraError = radio.begin(LORA_FREQ_MHZ, LORA_BW_KHZ, LORA_SF, LORA_CR,
                          LORA_SYNC, LORA_TX_POWER_DBM, 8, 1.8f);
  if (loraError != RADIOLIB_ERR_NONE) {
    Serial.print("[LORA] begin ERR="); Serial.println(loraError);
    return;
  }
  // These settings were confirmed in the running Meshtastic startup log.
  int16_t sw = radio.setDio2AsRfSwitch();
  radio.setRfSwitchPins(LORA_RXEN_PIN, RADIOLIB_NC);
  int16_t crc = radio.setCRC(2);
  Serial.print("[LORA] DIO2 switch="); Serial.print(sw);
  Serial.print(" CRC="); Serial.println(crc);
  if (sw != RADIOLIB_ERR_NONE || crc != RADIOLIB_ERR_NONE) {
    loraError = (sw != RADIOLIB_ERR_NONE) ? sw : crc;
    return;
  }
  radio.setDio1Action(onLoraRx);
  loraError = radio.startReceive();
  loraReady = (loraError == RADIOLIB_ERR_NONE);
  Serial.print("[LORA] RX ");
  Serial.println(loraReady ? "OK 433.775 SF12 BW125 CR4/5" : "START FAILED");
  if (!loraReady) { Serial.print("[LORA] code="); Serial.println(loraError); }
}

void pollLora() {
  if (!loraReady || !loraRxFlag) return;
  loraRxFlag = false;
  size_t packetLen = radio.getPacketLength();
  if (packetLen > 255) packetLen = 255;
  uint8_t data[256] = {};
  int16_t state = radio.readData(data, packetLen);
  if (state == RADIOLIB_ERR_NONE) {
    loraPackets++;
    lastLoraRxAt = millis();
    lastRssi = radio.getRSSI();
    lastSnr = radio.getSNR();
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
  loraError = radio.startReceive();
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

using namespace Adafruit_LittleFS_Namespace;

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
enum Page { PAGE_MAIN, PAGE_MENU, PAGE_BME, PAGE_CLOCK, PAGE_GPS, PAGE_LORA, PAGE_INFO, PAGE_OLED, PAGE_OLED_CONFIRM, PAGE_AUTO, PAGE_SYMBOL };
Page page = PAGE_MAIN;
uint8_t menuIndex = 0;
int8_t pendingUtc = 3;
uint8_t pendingOledType = 1;
uint8_t pendingAutoWxMode = 0;
uint8_t pendingSymbolIndex = 0;
uint8_t previousOledType = 1;
uint32_t oledConfirmStarted = 0;

bool configStorageOK = false;
static const char *CONFIG_FILENAME = "/ta1fm_v04.cfg"; // V0.4/V0.6 OLED+UTC settings kept intact
static const char *CALL_FILENAME = "/ta1fm_call_v06.cfg";
static const char *AUTO_FILENAME = "/ta1fm_auto_v08.cfg";
static const uint32_t AUTO_MAGIC = 0x54463038UL;
// V0.9 beacon configuration uses its own file; no alteration of v08 layout.
static const char *BEACON_FILENAME = "/ta1fm_beacon_v09.cfg";
static const uint32_t BEACON_MAGIC = 0x54463039UL;
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
float sentCourse = NAN;
uint32_t lastSmartMotionSampleAt = 0;

struct BeaconRecord {
  uint32_t magic;
  uint8_t positionMode, fixedPositionMin, statusMin, commentMin;
  char comment[63];
  char status[63];
  uint32_t crc;
};
uint32_t beaconChecksum(const uint8_t *data, size_t length) {
  uint32_t sum = 2166136261UL;
  for (size_t i=0; i<length; ++i) {
    sum ^= data[i]; sum *= 16777619UL;
  }
  return sum;
}
bool validText(const char *s) {
  size_t n = strnlen(s, 63);
  if (n == 0 || n >= 63) return false;
  for (size_t i=0; i<n; ++i) {
    unsigned char c = (unsigned char)s[i];
    if (c < 32 || c > 126 || c == '|' || c == '~') return false;
  }
  return true;
}
// V0.9.2: Safe record replacement for Adafruit nRF52 LittleFS.
// FILE_O_WRITE seeks to EOF; do not truncate an already opened record.
// Instead create a fresh temporary file, close and verify it, then rename.
// An unsuccessful write leaves the previously stored record untouched.
bool saveRecordSafely(const char *destination, const void *record, size_t recordSize) {
  if (!configStorageOK || !destination || !record || recordSize == 0) return false;
  static const char *TEMP_RECORD = "/ta1fm_save_tmp.cfg";
  if (InternalFS.exists(TEMP_RECORD) && !InternalFS.remove(TEMP_RECORD)) {
    Serial.println("[FS] Cannot remove stale temp file");
    return false;
  }
  {
    File out(InternalFS);
    if (!out.open(TEMP_RECORD, FILE_O_WRITE)) {
      Serial.println("[FS] Cannot create temp file");
      return false;
    }
    const bool wrote = out.write((const uint8_t *)record, recordSize) == recordSize;
    out.close();
    if (!wrote) {
      Serial.println("[FS] Write failed; old settings untouched");
      InternalFS.remove(TEMP_RECORD);
      return false;
    }
  }
  // Read back to catch truncated or incorrect writes before replacing old file.
  {
    File in(InternalFS);
    if (!in.open(TEMP_RECORD, FILE_O_READ)) return false;
    bool valid = in.size() == recordSize;
    const uint8_t *expected = (const uint8_t *)record;
    uint8_t buffer[32];
    for (size_t offset = 0; valid && offset < recordSize; offset += sizeof(buffer)) {
      const size_t count = (recordSize - offset < sizeof(buffer)) ? recordSize - offset : sizeof(buffer);
      if (in.read(buffer, count) != (int)count || memcmp(buffer, expected + offset, count) != 0) valid = false;
    }
    in.close();
    if (!valid) {
      Serial.println("[FS] Read-back verification failed; old settings untouched");
      InternalFS.remove(TEMP_RECORD);
      return false;
    }
  }
  if (!InternalFS.rename(TEMP_RECORD, destination)) {
    Serial.println("[FS] Rename failed; old settings untouched");
    return false;
  }
  return true;
}

bool saveBeaconSettings() {
  if (!configStorageOK) return false;
  BeaconRecord r = {};
  r.magic = BEACON_MAGIC;
  r.positionMode = positionMode; r.fixedPositionMin = fixedPositionMin;
  r.statusMin = statusIntervalMin; r.commentMin = commentIntervalMin;
  strncpy(r.comment, commentText, sizeof(r.comment)-1);
  strncpy(r.status, statusText, sizeof(r.status)-1);
  r.crc = beaconChecksum((const uint8_t*)&r, sizeof(r)-sizeof(r.crc));
  return saveRecordSafely(BEACON_FILENAME, &r, sizeof(r));
}
void loadBeaconSettings() {
  if (!configStorageOK) return;
  File f(InternalFS);
  if (!f.open(BEACON_FILENAME, FILE_O_READ)) return;
  BeaconRecord r = {};
  int len = f.read(&r, sizeof(r)); f.close();
  if (len != (int)sizeof(r) || r.magic != BEACON_MAGIC ||
      r.crc != beaconChecksum((const uint8_t*)&r, sizeof(r)-sizeof(r.crc)) ||
      r.positionMode > 1 || (r.fixedPositionMin!=1 && r.fixedPositionMin!=3 &&
      r.fixedPositionMin!=5 && r.fixedPositionMin!=10 && r.fixedPositionMin!=15) ||
      (r.statusMin!=0 && r.statusMin!=15 && r.statusMin!=30 && r.statusMin!=60) ||
      (r.commentMin!=0 && r.commentMin!=15 && r.commentMin!=30 && r.commentMin!=60) ||
      !validText(r.comment) || !validText(r.status)) return;
  positionMode = r.positionMode;
  fixedPositionMin = r.fixedPositionMin;
  statusIntervalMin = r.statusMin;
  commentIntervalMin = r.commentMin;
  strcpy(commentText, r.comment); strcpy(statusText, r.status);
  Serial.println("[BEACON] Saved settings loaded");
}

struct AutoRecord {
  uint32_t magic;
  uint8_t mode;
  uint8_t wxMin;
  uint8_t parkMin;
  char symbolTable;
  char symbol;
  uint16_t check;
};
uint16_t autoCheck(uint32_t magic, uint8_t mode, uint8_t wx, uint8_t park, char tbl, char sym) {
  return (uint16_t)(0x6BD2U ^ (uint16_t)magic ^ (uint16_t)(magic >> 16)
       ^ ((uint16_t)mode << 12) ^ ((uint16_t)wx << 4) ^ park
       ^ ((uint16_t)(uint8_t)tbl << 8) ^ (uint8_t)sym);
}
bool validAutoConfig(uint8_t mode, uint8_t wx, uint8_t park, char tbl, char sym) {
  return mode <= 2 && (wx == 0 || wx == 15 || wx == 30 || wx == 60) &&
    (park == 1 || park == 3 || park == 5) &&
    (tbl == '/' || tbl == '\\') && sym >= '!' && sym <= '~';
}
void loadAutoSettings() {
  if (!configStorageOK) return;
  File f(InternalFS);
  if (!f.open(AUTO_FILENAME, FILE_O_READ)) return;
  AutoRecord c = {};
  int n = f.read(&c, sizeof(c));
  f.close();
  if (n != (int)sizeof(c) || c.magic != AUTO_MAGIC ||
      c.check != autoCheck(c.magic,c.mode,c.wxMin,c.parkMin,c.symbolTable,c.symbol) ||
      !validAutoConfig(c.mode,c.wxMin,c.parkMin,c.symbolTable,c.symbol)) return;
  autoWxMode = c.mode;
  wxIntervalMin = c.wxMin;
  parkWaitMin = c.parkMin;
  positionSymbolTable = c.symbolTable;
  positionSymbol = c.symbol;
  Serial.println("[AUTO] Stored settings loaded");
}
bool saveAutoSettings() {
  if (!configStorageOK) return false;
  AutoRecord c = {};
  c.magic = AUTO_MAGIC;
  c.mode = autoWxMode;
  c.wxMin = wxIntervalMin;
  c.parkMin = parkWaitMin;
  c.symbolTable = positionSymbolTable;
  c.symbol = positionSymbol;
  c.check = autoCheck(c.magic,c.mode,c.wxMin,c.parkMin,c.symbolTable,c.symbol);
  return saveRecordSafely(AUTO_FILENAME, &c, sizeof(c));
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

struct TrackerConfig {
  uint32_t magic;
  uint8_t display;
  int8_t utc;
  uint16_t check;
};
static const uint32_t CONFIG_MAGIC = 0x54414634UL;
uint16_t configCheck(uint32_t magic, uint8_t display, int8_t utc) {
  return (uint16_t)(0xA53CU ^ (uint16_t)magic ^ (uint16_t)(magic >> 16) ^ (uint16_t)(display << 8) ^ (uint8_t)utc);
}
void loadConfig() {
  configStorageOK = InternalFS.begin();
  if (!configStorageOK) { Serial.println("CONFIG FS unavailable; defaults in RAM"); return; }
  File f(InternalFS);
  if (!f.open(CONFIG_FILENAME, FILE_O_READ)) return;
  TrackerConfig c = {};
  int n = f.read(&c, sizeof(c));
  f.close();
  if (n == (int)sizeof(c) && c.magic == CONFIG_MAGIC && c.check == configCheck(c.magic, c.display, c.utc) &&
      (c.display == 1 || c.display == 2) && (c.utc == 0 || c.utc == 3)) {
    oledType = c.display; utcOffset = c.utc;
    Serial.println("Saved config loaded");
  }
}
bool saveConfig() {
  if (!configStorageOK) return false;
  TrackerConfig c = {CONFIG_MAGIC, oledType, utcOffset, 0};
  c.check = configCheck(c.magic, c.display, c.utc);
  const bool ok = saveRecordSafely(CONFIG_FILENAME, &c, sizeof(c));
  Serial.println(ok ? "Config saved" : "Config save FAILED");
  return ok;
}


void showMessage(const char *txt, uint32_t duration = 1600);
bool gpsFix();
// APRS test credentials live in their own LittleFS file to preserve V0.4/V0.6
// display/UTC configuration. No on-air transmission while NOCALL.
struct CallsignRecord {
  uint32_t magic;
  char callsign[12];
  uint16_t checksum;
};
static const uint32_t CALL_MAGIC = 0x54464336UL;
uint16_t callCheck(const char *v) {
  uint16_t n = 0x6A2DU;
  for (size_t i = 0; i < 11 && v[i]; ++i) n = (uint16_t)((n * 33U) ^ (uint8_t)v[i]);
  return n;
}
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
void loadCallsign() {
  callsignValid = false;
  strcpy(stationCall, "NOCALL");
  if (!configStorageOK) return;
  File f(InternalFS);
  if (!f.open(CALL_FILENAME, FILE_O_READ)) return;
  CallsignRecord rec = {};
  const int n = f.read(&rec, sizeof(rec));
  f.close();
  rec.callsign[sizeof(rec.callsign) - 1] = 0;
  if (n == (int)sizeof(rec) && rec.magic == CALL_MAGIC &&
      validateCall(rec.callsign) && rec.checksum == callCheck(rec.callsign)) {
    strcpy(stationCall, rec.callsign);
    callsignValid = true;
  }
}
bool storeCallsign(const char *call) {
  if (!configStorageOK || !validateCall(call)) return false;
  CallsignRecord rec = {};
  rec.magic = CALL_MAGIC;
  strncpy(rec.callsign, call, sizeof(rec.callsign) - 1);
  rec.checksum = callCheck(rec.callsign);
  bool ok = saveRecordSafely(CALL_FILENAME, &rec, sizeof(rec));
  if (ok) {
    strcpy(stationCall, rec.callsign);
    callsignValid = true;
  }
  return ok;
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
  const char table = parkedConfirmed ? '\\' : positionSymbolTable;
  const char symbol = parkedConfirmed ? 'P' : positionSymbol;
  const int n = snprintf(out, capacity, "%s>APZTFM,WIDE1-1:!%s%c%s%c%s",
                         stationCall, latText, table, lonText, symbol,
                         includeComment ? commentText : "");
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
    showMessage("WX WAIT 30s"); Serial.println("[WX BLOCK] 30s auxiliary TX guard"); return false;
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
  radio.standby();
  txLastCode = radio.transmit(frame, sizeof(APRS_HEADER) + n);
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
  loraError = radio.startReceive();
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
  radio.standby();
  txLastCode = radio.transmit(frame, sizeof(APRS_HEADER) + n);
  if (txLastCode == RADIOLIB_ERR_NONE) {
    ++txSuccesses;
    lastPositionTxAt = now;
    if (commentIntervalMin && strstr(packet, commentText)) lastCommentTxAt = now;
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
  loraError = radio.startReceive();
  loraReady = loraError == RADIOLIB_ERR_NONE;
  if (!loraReady) Serial.println("[TX] WARN: RX restart failed");
  return txLastCode == RADIOLIB_ERR_NONE;
}

// APRS status packet: information field starts with > (no GPS required).
bool manualStatusTx() {
  if (!callsignValid || !loraReady) { Serial.println("[STATUS TX] No callsign/LoRa"); return false; }
  uint32_t now = millis();
  if (everTransmitted && (uint32_t)(now-lastTxAt)<AUX_TX_GAP_MS) {
    Serial.println("[STATUS TX] WAIT 30s"); return false;
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
  radio.standby(); txLastCode=radio.transmit(frame,sizeof(APRS_HEADER)+n);
  bool ok = txLastCode==RADIOLIB_ERR_NONE;
  if (ok) { ++txSuccesses; lastStatusTxAt=now; Serial.println("[STATUS TX] RadioLib OK"); showMessage("STATUS TX OK"); }
  else { Serial.print("[STATUS TX] error="); Serial.println(txLastCode); }
  loraRxFlag=false;
  loraError=radio.startReceive();
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
      if (!movingConfirmed) Serial.println("[AUTO] MOVING CONFIRMED");
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
        // Faster movement -> shorter interval; min 90s at 90 km/h.
        float speed=(gps.speed.isValid() && gps.speed.age()<5000) ? gps.speed.kmph() : 0;
        if (speed>=8.0f) {
          float v=speed>90.0f ? 90.0f : speed;
          // Speed-based rate: at 90km/h about 30s, at lower speed slower.
          // This is NOT a fixed 10s transmitter cycle.
          dueMs=(uint32_t)(30000.0f * 90.0f / v);
          if (dueMs>1800000UL) dueMs=1800000UL;
          if (dueMs<POS_TX_GAP_MS) dueMs=POS_TX_GAP_MS;
        } else dueMs=1800000UL; // parked / slow 30 min
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
  if (auxiliaryAllowed && wxWaiting &&
      (!lastWxAttemptAt || (uint32_t)(now-lastWxAttemptAt)>=AUX_TX_GAP_MS)) {
    lastWxAttemptAt=now;
    Serial.println("[AUTO] WX due (independent)");
    if (manualWxTx()) { lastAutoWxAt=now; wxDueAt=now+(uint32_t)wxIntervalMin*60000UL; }
    return;
  }
  if (auxiliaryAllowed && statusWaiting &&
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
void handleUsbCommand(const char *line) {
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
    Serial.print("[STATUS] Call: "); Serial.println(stationCall);
    Serial.print("[STATUS] GPS fix: "); Serial.println(gpsFix() ? "YES" : "NO");
    Serial.print("[STATUS] LoRa RX: "); Serial.println(loraReady ? "OK" : "ERROR");
    Serial.print("[STATUS] TX count: "); Serial.println(txSuccesses);
    Serial.print("[STATUS] WX TX count: "); Serial.println(wxSuccesses);
    Serial.print("[AUTO] mode="); Serial.print(modeName(autoWxMode));
    Serial.print(" POS="); Serial.print(positionMode?"SMART":"FIXED");
    Serial.print(" FIXEDmin="); Serial.print(fixedPositionMin);
    Serial.print(" WXmin="); Serial.print(wxIntervalMin);
    Serial.print(" STATUSmin="); Serial.print(statusIntervalMin);
    Serial.print(" COMMENTmin="); Serial.print(commentIntervalMin);
    Serial.print(" ParkMin="); Serial.print(parkWaitMin);
    Serial.print(" Symbol="); Serial.print(positionSymbolTable); Serial.println(positionSymbol);
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
    Serial.println("[CMDS] comment TEXT | status TEXT | comment? | status? | t POS | w WX | v STATUS | s car|walker|house | p");
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

void oledFlush() {
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

uint8_t menuCount() { return bmeReady ? 9 : 8; }

const char *menuItem(uint8_t item) {
  if (bmeReady) {
    switch (item) {
      case 0: return "BME280 WX";
      case 1: return "AUTO BEACON";
      case 2: return "POS SYMBOL";
      case 3: return "UTC OFFSET";
      case 4: return "DISPLAY";
      case 5: return "GPS INFO";
      case 6: return "LORA INFO";
      case 7: return "DEVICE INFO";
      default: return "EXIT";
    }
  }
  switch (item) {
    case 0: return "AUTO BEACON";
    case 1: return "POS SYMBOL";
    case 2: return "UTC OFFSET";
    case 3: return "DISPLAY";
    case 4: return "GPS INFO";
    case 5: return "LORA INFO";
    case 6: return "DEVICE INFO";
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
  gfx->setCursor(48, 55); gfx->print("V0.9.2");
  oledFlush();
}

void drawOled() {
  if (!oledReady || !gfx) return;
  gfx->fillScreen(0);
  gfx->setTextColor(SSD1306_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(0, 0);
  gfx->print(stationCall);  // NOCALL until user saves a valid callsign
  if (page == PAGE_MAIN) drawTime();
  else {
    gfx->setCursor(91, 0);
    gfx->print("V0.9.2");
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
    gfx->setCursor(0, 49); gfx->print("FW: V0.9.2 nRF52");
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
    gfx->print(passed >= OLED_CONFIRM_MS ? 0 : (OLED_CONFIRM_MS - passed + 999) / 1000);
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
    case PAGE_OLED:
      if (oledType == pendingOledType) { returnToMenu(); break; }
      previousOledType = oledType;
      initOled(pendingOledType);
      oledConfirmStarted = millis();
      page = PAGE_OLED_CONFIRM;
      // Do not store until user physically confirms image is visible.
      break;
    case PAGE_OLED_CONFIRM:
      // Only confirm deliberately if screen is legible.
      if (oledReady) { saveConfig(); returnToMenu(); }
      break;
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
      longHandled = false;
    } else if (!longHandled) {
      shortClick();
    }
  }
  if (buttonStable == LOW && !longHandled &&
      now - pressBeganAt >= LONG_PRESS_MS) {
    longHandled = true;
    longClick();
  }
}

void setup() {
  Serial.begin(115200);
  delay(700);
  Serial.println("\nTA1FM nrf52 Tracker V0.9.2 - SMART / AUX QUEUE");
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
  loadCallsign();
  loadAutoSettings();
  loadBeaconSettings();
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
  drawOled();
  Serial.println("USB: p help; a on|off; f smart|fixed; x off|15|30|60; z off|15|30|60; j off|15|30|60; comment TEXT; status TEXT");
}

void loop() {
  // Her dongude GPS verilerini bosalt; menu GPS'i durdurmaz.
  while (Serial1.available()) gps.encode(Serial1.read());
  pollButton();
  pollLora();
  pollAutoBeacon();
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

  if (page == PAGE_OLED_CONFIRM && now - oledConfirmStarted >= OLED_CONFIRM_MS) {
    initOled(previousOledType);
    pendingOledType = previousOledType;
    returnToMenu();
    Serial.println("Display preview timeout: reverted");
  }
  if (page != PAGE_MAIN && page != PAGE_OLED_CONFIRM && now - lastUserAction >= MENU_TIMEOUT_MS)
    page = PAGE_MAIN;
  if (now - lastBattery >= 5000) { lastBattery = now; readBattery(); }
  if (now - lastBme >= 3000) { lastBme = now; readBme(); }
  if (now - lastBmeProbe >= 30000) { lastBmeProbe = now; probeBme(); }
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
