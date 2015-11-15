#ifndef __CONFIG_H_INCLUDED__
#define __CONFIG_H_INCLUDED__

#include <Arduino.h>
#include "motordrive.h"

struct ConfigData {
  uint16_t sig;
  ServoConfig lconfig;
  ServoConfig rconfig;
  double Kp;
  double Ki;
  double Kd;
  uint8_t checksum;
};

uint8_t checksum(uint8_t* data, int len);
void writeDefaultConfig();
bool readConfig(ConfigData& config);
void writeConfig(ConfigData& config);

#endif