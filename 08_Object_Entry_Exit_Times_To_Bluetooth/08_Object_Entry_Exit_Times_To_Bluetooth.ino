//this one
#include <BluetoothSerial.h>


BluetoothSerial SerialBT;

// -------- ULTRASONIC --------
#define TRIG 5
#define ECHO 18



// -------- VARIABLES --------
unsigned int entryTime,exitTime;
int personCount = 0;

long duration;



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

}

// ================= LOOP =================
void loop() {

  float distance = getDistance();

  // Ignore invalid readings
  if (distance == 999) return;

  if (SerialBT.hasClient()) {
    if(distance>3 && distance <167){
      entryTime=millis();
      while(distance>3 && distance <167)
      {
        distance= getDistance();
        delay(70);
      }
      exitTime = millis();
      personCount++;

      String log = "Person" + String(personCount) + "|" + String(entryTime) + "|" + String(exitTime);
      Serial.println(log);
      SerialBT.println(log);
    }
  }

  delay(200);
}