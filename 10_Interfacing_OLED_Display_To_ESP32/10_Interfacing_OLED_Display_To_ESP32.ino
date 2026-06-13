#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


/*
  SDA 21
  SCL 22
  */


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{
  Serial.begin(115200);
  Wire.begin(21, 22);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D))
  {
    Serial.println("OLED init failed");
    while(1);
  }

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("Hello");

  display.display();
}

void loop()
{
}


// #include <Wire.h>

// void setup() {
//   Serial.begin(115200);
//   Wire.begin(21, 22);
//   Serial.println("Scanning I2C...");

//   for(byte addr = 1; addr < 127; addr++) {
//     Wire.beginTransmission(addr);
//     if(Wire.endTransmission() == 0) {
//       Serial.print("Device found at 0x");
//       Serial.println(addr, HEX);
//     }
//   }
//   Serial.println("Done.");
// }

// void loop() {}