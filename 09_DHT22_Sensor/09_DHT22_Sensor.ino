#include "DHTesp.h"

const int DHT_PIN = 4;
DHTesp dhtSensor;

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void loop() 
{
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  Serial.print("Temp: ");
  Serial.print(data.temperature);
  Serial.print(" °C   ");

  Serial.print("Humidity: ");
  Serial.print(data.humidity);
  Serial.println(" %");

  delay(2000); // Wait for a new reading from the sensor (DHT22 has ~0.5Hz sample rate)
}