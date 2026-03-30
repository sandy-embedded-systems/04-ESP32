/* Configure the ESP32 to connect to your Wi-Fi network */

#include<WiFi.h>

const char *ssid="Santhosh";
const char *password="12345678";
int cnt=0;
void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    cnt++;
    if(cnt==30){
      Serial.println(".");
      Serial.println("Unable to connect to network: time Taken 15 sec");
      while(1);
    }
    delay(500);
  }
  Serial.println(".");
  Serial.println("Connected: SUCCESS");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  WiFi.disconnect();
  Serial.println("Wifi Disconnected...");
}
void loop(){
  delay(1000);
}