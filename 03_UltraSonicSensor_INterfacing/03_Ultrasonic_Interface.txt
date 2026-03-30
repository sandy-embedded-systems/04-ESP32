/* Ultrasonic Sensor Readings on Serial monitor */

#define TRIG_PIN 5
#define ECHO_PIN 18

long duration;
float distance;

void setup(){
  Serial.begin(9600);
  pinMode(TRIG_PIN,OUTPUT);
  pinMode(ECHO_PIN,INPUT);
}

void loop(){
  digitalWrite(TRIG_PIN,LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN,LOW);
  duration=pulseIn(ECHO_PIN,HIGH);
  distance=(duration*0.0343)/2.;
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(500);
}