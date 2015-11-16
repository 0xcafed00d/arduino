#include "balance.h"

BalanceState::BalanceState(Adafruit_BNO055* imu)
    : imu(imu),
      drive(&leftServo, &rightServo),
      pid(&pidInput, &pidOutput, &pidTarget, 4, 5, 1, DIRECT) {}

void BalanceState::enter() {
  Serial.println(F("\n\nEntering: Robot Balance State"));

  if (!readConfig(cd)) {
    Serial.println(F("ERROR: Config Not found"));
    stateGoto(NULL);
    return;
  }

  drive.setServoConfig(cd.lconfig, cd.rconfig);
  leftServo.attach(servoLeftPin);
  rightServo.attach(servoRightPin);

  pid.SetOutputLimits(-1.0, 1.0);
  drive.drive(0);
  pid.SetMode(AUTOMATIC);
}

void BalanceState::action() {
  sensors_event_t event;
  imu->getEvent(&event);

  pidInput = event.orientation.y / 90.0;
  pidTarget = 0;
  pid.Compute();

  drive.drive(1000 * pidOutput);

  delay(50);
  int ch = Serial.read();
  if (ch == 'q') {
    stateGoto(NULL);
  }
}

void BalanceState::leave() {
  leftServo.detach();
  rightServo.detach();
}
