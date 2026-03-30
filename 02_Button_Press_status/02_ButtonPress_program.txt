/* Button press status on Serial monitor */

int buttonPin=0;
void setup()
{
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
}
 
void loop() {
  if (digitalRead(buttonPin) == LOW) {  
    delay(50);                          
 
    if (digitalRead(buttonPin) == LOW)
    {
      Serial.println("Button Pressed");
 
      while (digitalRead(buttonPin) == LOW);
      delay(50);
    }
  }
}