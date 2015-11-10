#include <Arduino.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

void setup() {
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);  // turn on the radio
  Serial.begin(115200);
}

void printHex(Stream& str, uint16_t value, int digits) {
  static const char hex[] PROGMEM = "0123456789ABCDEF";

  value = value << ((4 - digits) * 4);

  for (int d = 0; d < digits; d++) {
    str.print((char)pgm_read_byte_near(hex + ((value >> 12) & 0xf)));
    value = value << 4;
  }
}

int parseHex(const char* str, uint16_t& value) {
  int count = 0;
  value = 0;

  while (true) {
    if (*str >= '0' && *str <= '9') {
      value <<= 4;
      value += *str++ - '0';
    } else if (*str >= 'a' && *str <= 'f') {
      value <<= 4;
      value += *str++ - 'a' + 0xa;
    } else if (*str >= 'A' && *str <= 'F') {
      value <<= 4;
      value += *str++ - 'A' + 0xa;
    } else {
      return count;
    }
  }
}

void dump(int addr) {
  printHex(Serial, addr, 4);
  Serial.print(": ");
  for (int n = 0; n < 16; n++) {
    printHex(Serial, EEPROM.read(addr + n), 2);
    Serial.print(" ");
  }
  Serial.println();
}

void processBuffer(const char* buffer) {
  uint16_t val;

  parseHex(buffer, val);
  Serial.println("");
  printHex(Serial, val, 4);
  Serial.println("");
  //  Serial.println(buffer);
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