#include <WiFi.h>
#include <SPI.h>
#include "time.h"

// -------- CONFIG --------
#define TRIG 5
#define ECHO 21
#define CS_PIN 15
#define LOG_SIZE 256
#define MAX_LOGS 10
#define META_ADDR  0x0000   
#define DATA_START 0x1000   // Sector 1

SPISettings flashSPI(1000000, MSBFIRST, SPI_MODE0);

const char* ssid     = "Santhosh";
const char* password = "12345678";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; 

struct Meta {
  int flashIndex;
  int totalLogs;
  int totalPersons;
};

Meta meta;
bool personInside = false;
struct tm timeinfo;

// ================= FLASH CORE =================

void flashWriteEnable() {
  SPI.beginTransaction(flashSPI);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x06); 
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

void flashWaitBusy() {
  uint8_t status = 0;
  do {
    SPI.beginTransaction(flashSPI);
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(0x05); 
    status = SPI.transfer(0x00);
    digitalWrite(CS_PIN, HIGH);
    SPI.endTransaction();
    delay(1); 
  } while (status & 0x01); 
}

void flashEraseSector(uint32_t addr) {
  flashWriteEnable();
  SPI.beginTransaction(flashSPI);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x20); 
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  flashWaitBusy();
}

void flashWritePage(uint32_t addr, uint8_t *data, int len) {
  flashWriteEnable();
  SPI.beginTransaction(flashSPI);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x02); 
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  for (int i = 0; i < len; i++) SPI.transfer(data[i]);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
  flashWaitBusy();
}

void flashReadData(uint32_t addr, char *buffer, int len) {
  SPI.beginTransaction(flashSPI);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x03); 
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  for (int i = 0; i < len; i++) buffer[i] = (char)SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

// ================= LOGGING LOGIC (UPDATED) =================

void saveMeta() {
  flashEraseSector(META_ADDR);
  flashWritePage(META_ADDR, (uint8_t*)&meta, sizeof(Meta));
}

void storeLog(String ent, String ext) {
  char tempLogs[MAX_LOGS][LOG_SIZE];
  
  // 1. Read existing logs into RAM (to preserve them)
  for (int i = 0; i < meta.totalLogs; i++) {
    flashReadData(DATA_START + (i * LOG_SIZE), tempLogs[i], LOG_SIZE);
  }

  // 2. Prepare the new log line (Starts from P1)
  meta.totalPersons++;
  String newLog = "P" + String(meta.totalPersons) + "|" + ent + "|" + ext;
  
  // 3. Update the temporary array (Replace oldest if full)
  newLog.toCharArray(tempLogs[meta.flashIndex], LOG_SIZE);

  // 4. ERASE the sector and REWRITE all logs
  // This ensures individual replacement without losing the rest
  flashEraseSector(DATA_START);
  
  int logsToSave = (meta.totalLogs < MAX_LOGS) ? (meta.totalLogs + 1) : MAX_LOGS;
  for (int i = 0; i < logsToSave; i++) {
    flashWritePage(DATA_START + (i * LOG_SIZE), (uint8_t*)tempLogs[i], LOG_SIZE);
  }

  // 5. Update Metadata
  meta.flashIndex = (meta.flashIndex + 1) % MAX_LOGS;
  if (meta.totalLogs < MAX_LOGS) meta.totalLogs++;
  saveMeta();
}

void readLogs() {
  Serial.println("\n===== CURRENT HISTORY =====");
  for (int i = 0; i < meta.totalLogs; i++) {
    // This logic ensures the OLD logs appear first even after wrap-around
    int idx = (meta.totalLogs == MAX_LOGS) ? (meta.flashIndex + i) % MAX_LOGS : i;
    uint32_t addr = DATA_START + (idx * LOG_SIZE);
    char buf[LOG_SIZE];
    flashReadData(addr, buf, LOG_SIZE);
    if ((uint8_t)buf[0] != 0xFF && buf[0] != 0x00) {
      Serial.println(buf);
    }
  }
  Serial.println("===========================\n");
}

void loadMeta() {
  flashReadData(META_ADDR, (char*)&meta, sizeof(Meta));
  if (meta.totalLogs < 0 || meta.totalLogs > MAX_LOGS) {
    Serial.println("Fresh start: Clearing Flash...");
    meta.flashIndex = 0; meta.totalLogs = 0; meta.totalPersons = 0;
    saveMeta();
    flashEraseSector(DATA_START);
  }
}

// ================= UTILS =================

String getTimeString() {
  if (!getLocalTime(&timeinfo)) return "00:00:00";
  char buf[20];
  strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
  return String(buf);
}

float getDistance() {
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long dur = pulseIn(ECHO, HIGH, 30000);
  return (dur == 0) ? 999 : dur * 0.034 / 2;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  SPI.begin(18, 19, 23, CS_PIN);
  pinMode(CS_PIN, OUTPUT); digitalWrite(CS_PIN, HIGH);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  configTime(gmtOffset_sec, 0, ntpServer);
  while (!getLocalTime(&timeinfo)) { delay(500); }

  loadMeta();
  Serial.println("System Ready.");
  readLogs();
}

void loop() {
  float dist = getDistance();
  static String entT = "";
  static int entC = 0, extC = 0;

  if (dist < 20) {
    entC++;
    if (entC >= 3 && !personInside) {
      personInside = true; entT = getTimeString();
      Serial.println(">> ENTRY");
      entC = 0;
    }
  } else if (dist > 40 && dist < 600) {
    extC++;
    if (extC >= 3 && personInside) {
      personInside = false;
      Serial.println("<< EXIT");
      storeLog(entT, getTimeString());
      readLogs();
      extC = 0;
    }
  } else { entC = 0; extC = 0; }
  delay(100);
}

