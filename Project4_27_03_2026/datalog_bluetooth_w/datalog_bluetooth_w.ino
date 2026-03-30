//this one
#include <BluetoothSerial.h>
#include <WiFi.h>
#include "time.h"

BluetoothSerial SerialBT;

// -------- ULTRASONIC --------
#define TRIG 5
#define ECHO 18

// -------- WIFI --------
const char* ssid = "Santhosh";
const char* password = "12345678";

// -------- NTP --------
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;

struct tm timeinfo;

// -------- VARIABLES --------
bool personInside = false;
int personCount = 0;

long duration;

// ================= TIME =================
String getTimeString() {
  if (!getLocalTime(&timeinfo, 2000)) return "NoTime";

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

  duration = pulseIn(ECHO, HIGH, 30000);

  if (duration == 0) return 999;

  return duration * 0.034 / 2;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  SerialBT.begin("ESP32_Logger");

  // WiFi connect
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Time sync
  configTime(gmtOffset_sec, 0, ntpServer);

  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 20) {
    delay(500);
    retry++;
  }
}

// ================= LOOP =================
void loop() {

  float distance = getDistance();

  static String entryTime = "";
  static unsigned long lastTrigger = 0;

  // Ignore invalid readings
  if (distance == 999) return;

  if (SerialBT.hasClient()) {

    if (distance < 5 && !personInside && millis() - lastTrigger > 2000) {
      personInside = true;
      lastTrigger = millis();

      entryTime = getTimeString();
    }

    if (distance > 10 && personInside && millis() - lastTrigger > 2000) {
      personInside = false;
      lastTrigger = millis();

      String exitTime = getTimeString();
      personCount++;

      String log = "Person" + String(personCount) + "|" + entryTime + "|" + exitTime;
      Serial.println(log);
      SerialBT.println(log);
    }
  }

  delay(200);
}