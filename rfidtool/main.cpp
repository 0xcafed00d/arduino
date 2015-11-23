#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9  // Configurable, see typical pin layout above
#define SS_PIN 10  // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 card

  Serial.println(F("RFID R/W tool. type h for help"));
  Serial.print(F("> "));
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
      value = (value << 4) + *str++ - '0';
      count++;
    } else if (*str >= 'a' && *str <= 'f') {
      value = (value << 4) + *str++ - 'a' + 0xa;
      count++;
    } else if (*str >= 'A' && *str <= 'F') {
      value = (value << 4) + *str++ - 'A' + 0xa;
      count++;
    } else {
      return count;
    }
  }
}

void dump(int addr, int count) {
  char buffer[17];
  buffer[16] = 0;

  while (count > 0) {
    printHex(Serial, addr, 4);
    Serial.print(": ");
    for (int n = 0; n < 16; n++) {
      buffer[n] = '.';
      if (count > 0) {
        uint8_t c = EEPROM.read(addr + n);
        printHex(Serial, c, 2);
        if (c >= ' ' && c <= 127) {
          buffer[n] = c;
        }
      } else {
        Serial.print("__");
      }
      Serial.print(" ");
      count--;
    }
    Serial.print(": ");
    Serial.print(buffer);
    Serial.println("");
    addr += 16;
  }
}

struct CommandHandler {
  char cmd;
  bool (*func)(uint16_t argv[], int argc);
};

bool helpcmd(uint16_t argv[], int argc) {
  Serial.println();
  Serial.println(F("dump data:  d <addr> <count>"));
  Serial.println(F("write byte: w <addr> <byte0> .... <byte15>"));
}

bool dumpcmd(uint16_t argv[], int argc) {
  if (argc == 2) {
    uint16_t addr = argv[0];
    Serial.println("");
    dump(addr, argv[1]);
    return true;
  }

  return false;
}

bool writecmd(uint16_t argv[], int argc) {
  if (argc >= 2) {
    uint16_t addr = argv[0];

    for (int n = 1; n < argc; n++) {
      EEPROM.write(addr + n - 1, (uint8_t)argv[n]);
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

bool parseCommand(const char* buffer, CommandHandler* commands) {
  if (isalpha(*buffer)) {
    char cmd = *buffer++;
    int argc = 0;
    uint16_t argv[17];

    while (true) {
      buffer = advance(buffer);
      int parsed = parseHex(buffer, argv[argc]);
      if (parsed > 0) {
        buffer += parsed;
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

static CommandHandler commands[] = {
    {'d', dumpcmd}, {'w', writecmd}, {'h', helpcmd}, {0, NULL}};

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
      Serial.println("");
      Serial.println(res ? F("OK") : F("ERROR"));
      inputIndex = 0;
      Serial.print(F("> "));
    } else {
      if (in >= ' ' && inputIndex < bufferSize) {
        inputBuffer[inputIndex++] = (char)in;
        Serial.print((char)in);
      }
    }
  }
}
