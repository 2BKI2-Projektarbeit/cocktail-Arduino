#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

#define SDA_PIN 53
#define RST_PIN 5

MFRC522 mfrc522(SDA_PIN, RST_PIN);

// Data
volatile byte received = 0;
String data = "";
long rfidCode = 0;

// Pins
int pumps[] = { 17, 16, 6, 7, 8, 9 };

int redPin = 39;
int greenPin = 40;
int bluePin = 41;

int interruptPin = 2;

// Initial methods
void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_Reset();

  DDRH = 0xff;
  PORTH = 0xff;

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  pinMode(interruptPin, INPUT_PULLUP);
  
  Wire.begin(0x2c);
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);

  attachInterrupt(digitalPinToInterrupt(interruptPin), cancelPumps, FALLING);
}

void loop() {
  if(received) {
    handleReceive();
  }

  if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("New");
    long tmpCode = 0;
    
    for(int i = 0; i < mfrc522.uid.size; i++) {
      tmpCode = ((tmpCode + mfrc522.uid.uidByte[i]) * 10);
    }

    if(tmpCode != rfidCode) {
      rfidCode = tmpCode;
      Serial.println(rfidCode);
    }
  }
}

// Methods for TWI
void handleReceive() {
  Serial.println("Message received: " + data);
    
  if(split(data, ':', 0) == "p") {
    PORTH = 0xff;  
    digitalWrite(pumps[split(data, ':', 1).toInt() - 1], LOW);
    delay(split(data, ':', 2).toInt());
    PORTH = 0xff;
  } else if(split(data, ':', 0) == "rgb") {
    analogWrite(redPin, split(data, ':', 1).toInt());
    analogWrite(greenPin, split(data, ':', 2).toInt());
    analogWrite(bluePin, split(data, ':', 3).toInt());
  } else if(split(data, ':', 0) == "clr") {
    if(split(data, ':', 1) == "rfid") {
      rfidCode = 0;
    }
  }

  received = 0;
  data = "";
}

// Interrupt methods
void cancelPumps() {
  for(int i = 0; i < sizeof(pumps); i++) {
    digitalWrite(pumps[i], HIGH);
  }
}

// ISR for TWI
void onReceive(int howMany) {
  data = "";

  while(Wire.available()) {
    data += (char) Wire.read();
  }

  received = 1;
}

void onRequest() {
  if(rfidCode != 0)
    Wire.write(rfidCode);
  
  rfidCode = 0;
}

// Useful methods
String split(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for(int i = 0; i <= maxIndex && found <= index; i++) {
    if(data.charAt(i) == separator || i == maxIndex) {
        found++;
        strIndex[0] = strIndex[1] + 1;
        strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}
