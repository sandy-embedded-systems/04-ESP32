#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Santhosh";
const char* password = "00000000";

WebServer server(80);

void handleRoot()
{
  String html =
    "<!DOCTYPE html>"
    "<html>"
    "<head><title>ESP32 Web Server</title></head>"
    "<body>"
    "<h1>Hello from ESP32!</h1>"
    "<p>This webpage is served by the ESP32.</p>"
    "</body>"
    "</html>";

  server.send(200, "text/html", html);
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.begin();
  Serial.println("Web Server Started");
}

void loop()
{
  server.handleClient();
}