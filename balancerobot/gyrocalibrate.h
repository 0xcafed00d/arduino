#ifndef __GYROCALIBRATE_H_INCLUDED__
#define __GYROCALIBRATE_H_INCLUDED__

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#include "utils.h"
#include "state.h"

struct GyroCalState : public State {
  Adafruit_BNO055* imu;

  GyroCalState(Adafruit_BNO055* imu);
  void enter();
  void action();
  void leave();
};

#endif