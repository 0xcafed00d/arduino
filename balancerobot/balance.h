#ifndef __BALANCE_H_INCLUDED__
#define __BALANCE_H_INCLUDED__

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <PID_v1.h>

#include "utils.h"
#include "motordrive.h"
#include "state.h"
#include "hardwaredefs.h"
#include "config.h"

struct BalanceState : public State {
  Adafruit_BNO055* imu;

  Servo leftServo;
  Servo rightServo;

  ConfigData cd;
  MotorDrive drive;

  double pidTarget;
  double pidInput;
  double pidOutput;

  PID pid;

  BalanceState(Adafruit_BNO055* imu);
  void enter();
  void action();
  void leave();
};

#endif