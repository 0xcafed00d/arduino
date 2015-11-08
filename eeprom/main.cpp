#include <Arduino.h>
#include <EEPROM.h>

void setup() {
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);  // turn on the radio
  Serial.begin(115200);
}

void dump(int addr) {
  Serial.print(addr, HEX);
  Serial.print(": ");
  for (int n = 0; n < 16; n++) {
    Serial.print(EEPROM.read(addr + n), HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void processBuffer(const char* buffer) {
  Serial.println("");
  Serial.println(buffer);
  dump(0);
}

const size_t bufferSize = 32;
char inputBuffer[bufferSize + 1];
size_t inputIndex = 0;

void loop() {
  int in = Serial.read();
  if (in > 0) {
    if (in == 8 && inputIndex > 0) {
      inputIndex--;
      Serial.print((char)in);
    }

    if (in == '\r') {
      inputBuffer[inputIndex] = 0;
      processBuffer(inputBuffer);
      inputIndex = 0;
    } else {
      if (in >= ' ' && inputIndex < bufferSize) {
        inputBuffer[inputIndex++] = (char)in;
        Serial.print((char)in);
      }
    }
  }
}