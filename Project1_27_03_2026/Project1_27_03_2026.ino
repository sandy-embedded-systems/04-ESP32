#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include "time.h"

// -------- ULTRASONIC --------
#define TRIG 5
#define ECHO 21

// -------- SD CARD --------
#define SD_CS 15
#define SCK_PIN 18
#define MISO_PIN 19
#define MOSI_PIN 23

// -------- WIFI --------
const char* ssid     = "Santhosh";
const char* password = "12345678";

// -------- NTP --------
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800; // IST (UTC+5:30)
const int   daylightOffset_sec = 0;

// -------- LOGGING --------
struct PersonLog {
  String entryTime;
  String exitTime;
};

PersonLog currentLog;
int totalPersons = 0;
bool personInside = false;

// ================= TIME =================
String getTimeString() {
  struct tm timeinfo;
  // If sync hasn't happened yet, this returns false
  if (!getLocalTime(&timeinfo)) {
    return "Syncing...";
  }
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

// ================= DISTANCE =================
float getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000); 
  if (duration == 0) return 400; 
  return duration * 0.034 / 2;
}

// ================= STORE LOG (SD CARD) =================
void storeLogSD(String entry, String exitTime, int personNum) {
  String newLog = "Person" + String(personNum) + "|" + entry + "|" + exitTime;

  String logList[10];
  int count = 0;

  File file = SD.open("/logs.txt", FILE_READ);
  if (file) {
    while (file.available() && count < 10) {
      String line = file.readStringUntil('\n');
      line.trim(); 
      if (line.length() > 0) {
        logList[count++] = line;
      }
    }
    file.close();
  }

  int startIdx = (count >= 10) ? 1 : 0;

  file = SD.open("/logs.txt", FILE_WRITE);
  if (!file) {
    Serial.println("SD Write Error!");
    return;
  }

  for (int i = startIdx; i < count; i++) {
    file.println(logList[i]);
  }
  file.println(newLog);
  file.close();

  Serial.println("Logged: " + newLog);
}

// ================= READ LOGS =================
void readLogsSD() {
  Serial.println("\n--- SD CONTENT ---");
  File file = SD.open("/logs.txt", FILE_READ);
  if (file) {
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  }
  Serial.println("------------------\n");
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Failed!");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK!");

  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // NEW: Wait for NTP sync
  Serial.print("Waiting for NTP time sync...");
  struct tm timeinfo;
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 20) {
    Serial.print(".");
    delay(500);
    retry++;
  }
  
  if (retry < 20) {
    Serial.println("\nTime Synced!");
  } else {
    Serial.println("\nTime Sync Failed (Check Internet)!");
  }
}

// ================= LOOP =================
void loop() {
  float distance = getDistance();

  if (distance > 1 && distance < 5 && !personInside) {
    personInside = true;
    currentLog.entryTime = getTimeString();
    Serial.println("Entry: " + currentLog.entryTime);
  }

  if (distance > 10 && personInside) {
    personInside = false;
    currentLog.exitTime = getTimeString();
    totalPersons++;

    storeLogSD(currentLog.entryTime, currentLog.exitTime, totalPersons);
    readLogsSD();
  }

  delay(200);
}
