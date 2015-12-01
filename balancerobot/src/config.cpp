#include <Arduino.h>
#include "config.h"
#include "utils.h"

uint8_t checksum(uint8_t* data, int len) {
  uint8_t chk = 0;
  for (int n = 0; n < len; n++) {
    chk += data[n];
  }
  return ~chk + 1;
}

void writeDefaultConfig() {
  ConfigData cd;

  cd.sig = 0xCAFE;

  cd.lconfig.zeropoint = 1500;
  cd.lconfig.range = 500;
  cd.rconfig.zeropoint = 1500;
  cd.rconfig.range = 500;

  cd.Kp = 17.0;
  cd.Ki = 1.7;
  cd.Kd = 0.2;

  cd.checksum = 0;

  cd.checksum = checksum((uint8_t*)&cd, sizeof(cd));

  writeStructEEPROM(cd, 0);

  Serial.println(F("\n  Default Config Written \n"));
}

bool readConfig(ConfigData& config) {
  readStructEEPROM(config, 0);
  uint8_t chk = config.checksum;
  config.checksum = 0;
  config.checksum = checksum((uint8_t*)&config, sizeof(config));

  return config.sig == 0xCAFE && chk == config.checksum;
}

void writeConfig(ConfigData& config) {
  writeStructEEPROM(config, 0);
  config.checksum = 0;
  config.checksum = checksum((uint8_t*)&config, sizeof(config));
}
