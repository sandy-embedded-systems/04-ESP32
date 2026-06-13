#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Santhosh";
const char* password = "00000000";

#define LED_PIN 2

WebServer server(80);

void handleRoot()
{
  String html =
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<title>ESP32 LED Control</title>"
  "<meta name='viewport' content='width=device-width, initial-scale=1'>"
  "</head>"
  "<body>"
  "<h1>ESP32 Web Server</h1>"
  "<h2>LED Control</h2>"
  "<p><a href='/ledon'><button style='width:150px;height:50px;'>LED ON</button></a></p>"
  "<p><a href='/ledoff'><button style='width:150px;height:50px;'>LED OFF</button></a></p>"
  "</body>"
  "</html>";

  server.send(200, "text/html", html);
}

void handleLedOn()
{
  digitalWrite(LED_PIN, HIGH);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleLedOff()
{
  digitalWrite(LED_PIN, LOW);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/ledon", handleLedOn);
  server.on("/ledoff", handleLedOff);

  server.begin();

  Serial.println("Web Server Started");
}

void loop()
{
  server.handleClient();
}