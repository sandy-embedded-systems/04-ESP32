//flash read
#include <SPI.h>

// -------- CONFIG (Must match your Main Code) --------
#define CS_PIN 15
#define LOG_SIZE 256
#define MAX_LOGS 10
#define META_ADDR  0x0000   
#define DATA_START 0x1000   

struct Meta {
  int flashIndex;
  int totalLogs;
  int totalPersons;
};

Meta meta;
SPISettings flashSPI(1000000, MSBFIRST, SPI_MODE0);

// ================= FLASH READ CORE =================

void flashReadData(uint32_t addr, char *buffer, int len) {
  SPI.beginTransaction(flashSPI);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x03); 
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  for (int i = 0; i < len; i++) buffer[i] = (char)SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

// ================= SETUP =================

void setup() {
  Serial.begin(115200);
  delay(1500);

  SPI.begin(18, 19, 23, CS_PIN);
  pinMode(CS_PIN, OUTPUT); 
  digitalWrite(CS_PIN, HIGH);

  // 1. Load Meta to find the counts
  flashReadData(META_ADDR, (char*)&meta, sizeof(Meta));

  Serial.println("\n===========================================");
  Serial.println("       PERSON TRACKING DATA LOGS           ");
  Serial.println("===========================================");

  if (meta.totalLogs <= 0 || meta.totalLogs > MAX_LOGS) {
    Serial.println("No timestamps found in Flash memory.");
  } else {
    // 2. Loop through logs (Chronological Order: Oldest to Newest)
    for (int i = 0; i < meta.totalLogs; i++) {
      
      int idx = (meta.totalLogs == MAX_LOGS) ? (meta.flashIndex + i) % MAX_LOGS : i;
      uint32_t addr = DATA_START + (idx * LOG_SIZE);
      
      char buf[LOG_SIZE];
      flashReadData(addr, buf, LOG_SIZE);
      
      // 3. Print the log
      if ((uint8_t)buf[0] != 0xFF && buf[0] != '\0') {
        // Since format is: PersonX|Entry:HH:MM:SS|Exit:HH:MM:SS
        // We replace the pipes with spaces for a cleaner look
        String logEntry = String(buf);
        logEntry.replace("|", "  "); 
        
        Serial.print("[LOG "); 
        Serial.print(i + 1); 
        Serial.print("]: ");
        Serial.println(logEntry);
      }
    }
  }

  Serial.println("===========================================");
  Serial.println("Read Complete.");
}

void loop() {
  // Standalone reader - does nothing in loop
}
