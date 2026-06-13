#define RELAY_PIN 23

void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);

  // Relay OFF (active LOW module)
  digitalWrite(RELAY_PIN, HIGH);
}

void loop()
{
  Serial.println("Relay ON");
  digitalWrite(RELAY_PIN, LOW);
  delay(5000);

  Serial.println("Relay OFF");
  digitalWrite(RELAY_PIN, HIGH);
  delay(5000);
}