#ifndef __SERVOCALIBRATE_H_INCLUDED__
#define __SERVOCALIBRATE_H_INCLUDED__

#include <Arduino.h>

#include "utils.h"
#include "hardwaredefs.h"
#include "motordrive.h"
#include "state.h"
#include "config.h"

struct ServoCalState : public State {
  Servo leftServo;
  Servo rightServo;

  ConfigData cd;
  MotorDrive drive;

  ServoCalState();
  void update();
  void enter();
  void action();
  void leave();
};

#endif