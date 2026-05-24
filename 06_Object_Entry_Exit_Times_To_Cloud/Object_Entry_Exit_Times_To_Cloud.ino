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
String temp;
char timeString[30];

void shiftEntries() {
  for (int i=7;i>0;i--) {
    entries[i]=entries[i-1];
  }
  entries[0]=newEntry;
}

void sendToThingSpeak() {
  HTTPClient http;

  String url = "http://api.thingspeak.com/update?api_key="+writeAPIKey;
  for (int i=0;i<num;i++) {
    url +="&field"+String(i+1)+"="+entries[i];
  }
  url+="&status="+newEntry;
  http.begin(url);
  int httpCode = http.GET();

  Serial.println("HTTP Response: "+String(httpCode));
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

  for (int i=0;i<8;i++) {
    entries[i] = "";
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time configured...");
  delay(2000);
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting...");
    WiFi.begin(ssid, password);
    connectToWiFi();
  }
  getReading();
  if (distance>3 && distance<165){
    if(getLocalTime(&timeinfo)){
      strftime(timeString,sizeof(timeString),"%d-%m-%Y %H:%M:%S",&timeinfo);
      entryTime=String(timeString);
      entryFlag=true;
    }
    else entryFlag = false;
    while (distance>3 && distance <165) {
      getReading();
      delay(20);
    }
    if (getLocalTime(&timeinfo)) {
      strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeinfo);
      exitTime=String(timeString);
      exitFlag=true;
    }
    else exitFlag = false;

    if(entryFlag && exitFlag) {
      if(num<8) num++;;
      newEntry="Object "+String(objectCnt)+": "+entryTime+" to "+exitTime;
      newEntry.replace(" ", "%20");
      newEntry.replace(":", "%3A");

      shiftEntries();
      sendToThingSpeak();
      Serial.println("______Current Data______");
      for(int i=0;i<num;i++){
        temp=entries[i];
        temp.replace("%20"," ");
        temp.replace("%3A",":");
        Serial.println(temp);
      }
      objectCnt++;
    }
  }

  delay(20000);
}