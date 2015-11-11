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

struct CommandHandler {
  char cmd;
  bool (*func)(uint16_t argv[], int argc);
};

bool dumpcmd(uint16_t argv[], int argc) {
  if (argc >= 1) {
    uint16_t addr = argv[0];
    dump(addr);
    return true;
  }
  return false;
}

bool writecmd(uint16_t argv[], int argc) {
  Serial.println(argc);

  if (argc >= 2) {
    uint16_t addr = argv[0];

    for (int n = 1; n < argc; n++) {
      EEPROM.write(addr + n, (uint8_t)argv[n]);
    }

    return true;
  }
  return false;
}

const char* advance(const char* buffer) {
  while (*buffer && !isHexadecimalDigit(*buffer)) {
    buffer++;
  }
  return buffer;
}

bool parseCommand(const char* buffer, const CommandHandler* commands) {
  if (isAlpha(*buffer)) {
    char cmd = *buffer++;
    int argc = 0;
    uint16_t argv[17];

    while (true) {
      buffer = advance(buffer);
      if (parseHex(buffer, argv[argc]) > 0) {
        argc++;
      } else {
        break;
      }
    }

    while (commands->cmd) {
      if (cmd == commands->cmd) {
        return commands->func(argv, argc);
      }

      commands++;
    }
  }
  return false;
}

static CommandHandler commands[] = {{'d', dumpcmd}, {'w', writecmd}, {0, NULL}};

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
      bool res = parseCommand(inputBuffer, commands);
      Serial.println(res ? "\nOK" : "\nERROR");
      inputIndex = 0;
    } else {
      if (in >= ' ' && inputIndex < bufferSize) {
        inputBuffer[inputIndex++] = (char)in;
        Serial.print((char)in);
      }
    }
  }
}
