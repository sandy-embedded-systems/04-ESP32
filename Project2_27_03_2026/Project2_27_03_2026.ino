#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Santhosh";
const char* password = "12345678";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

int trigPin = 25;
int echoPin = 26;

float distance;
int pingTime;

int objectCnt = 1;
int num=0;
struct tm timeinfo;
bool entryFlag = false;
bool exitFlag = false;

String writeAPIKey = "TE4RH84R55I36TNJ";
String entries[8];

String entryTime;
String exitTime;
String newEntry;
char timeString[30];

void shiftEntries() {
  for (int i = 7; i > 0; i--) {
    entries[i] = entries[i - 1];
  }
  entries[0] = newEntry;
}

void sendToThingSpeak() {
  HTTPClient http;

  String url = "http://api.thingspeak.com/update?api_key=" + writeAPIKey;

  // Send fields
  for (int i = 0; i < 8; i++) {
    url += "&field" + String(i + 1) + "=" + entries[i];
  }

  // Send status (for readable display)
  url += "&status=" + newEntry;

  Serial.println(url); // debug

  http.begin(url);
  int httpCode = http.GET();

  Serial.println("HTTP Response: " + String(httpCode));
  http.end();
}

void getReading() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  pingTime = pulseIn(echoPin, HIGH, 30000);
  distance = (pingTime * 0.034) / 2.0;

  delay(25);
}

void connectToWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  WiFi.begin(ssid, password);
  connectToWiFi();
  Serial.println("Connected");

  for (int i = 0; i < 8; i++) {
    entries[i] = "";
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time configured");
  delay(2000);
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting...");
    WiFi.begin(ssid, password);
    connectToWiFi();
  }

  getReading();

  if (distance > 3 && distance < 165) {

    // ENTRY TIME
    if (getLocalTime(&timeinfo)) {
      strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeinfo);
      entryTime = String(timeString);
      entryFlag = true;
    } else entryFlag = false;

    // WAIT UNTIL OBJECT LEAVES (with timeout)
    unsigned long start = millis();
    while (distance > 2 && distance < 167) {
      getReading();
      delay(20);
    }

    // EXIT TIME
    if (getLocalTime(&timeinfo)) {
      strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeinfo);
      exitTime = String(timeString);
      exitFlag = true;
    } else exitFlag = false;

    if (entryFlag && exitFlag) {

      // Create readable string
      newEntry = "Object " + String(objectCnt) + ": " + entryTime + " - " + exitTime;
      // URL encoding
      newEntry.replace(" ", "%20");
      newEntry.replace(":", "%3A");

      shiftEntries();
      sendToThingSpeak();
      for(int i=0;i<num;i++){
        Serial.println(entries[i]);
      }
      if(num<8) num++;
      objectCnt++;
    }
  }

  delay(20000); // ThingSpeak limit
}