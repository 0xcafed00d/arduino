#include "balance.h"

int count = 0;

BalanceState::BalanceState(Adafruit_BNO055* imu)
    : imu(imu),
      drive(&leftServo, &rightServo),
      pid(&pidInput, &pidOutput, &pidTarget, 10, 1, 1, DIRECT) {}

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

  pid.SetTunings(cd.Kp, cd.Ki, cd.Kd);
  pid.SetOutputLimits(-1.0, 1.0);
  pid.SetSampleTime(10);
  drive.drive(0);
  pid.SetMode(AUTOMATIC);
  pidTarget = 0;
}

void BalanceState::action() {
  sensors_event_t event;
  imu->getEvent(&event);

  if (event.orientation.y < 10.0 && event.orientation.y > -10.0) {
    pidInput = event.orientation.y / 90.0;
    pid.Compute();

    drive.drive(1000 * pidOutput);
  } else {
    drive.drive(0);
  }

  delay(5);
  int ch = Serial.read();
  if (ch == 'q') {
    stateGoto(NULL);
  }

  switch (ch) {
    case 'p':
      cd.Kp += 0.1;
      break;
    case 'P':
      cd.Kp -= 0.1;
      break;

    case 'i':
      cd.Ki += 0.1;
      break;
    case 'I':
      cd.Ki -= 0.1;
      break;

    case 'd':
      cd.Kd += 0.1;
      break;
    case 'D':
      cd.Kd -= 0.1;
      break;

    case 'w':
      pidTarget += 0.01;
      break;
    case 's':
      pidTarget -= 0.01;
      break;
  }

  pid.SetTunings(cd.Kp, cd.Ki, cd.Kd);

  if (count == 100) {
    Serial.print("Y: ");
    Serial.print(event.orientation.y, 2);

    Serial.print("\tKp: ");
    Serial.print(cd.Kp, 2);
    Serial.print("\tKi: ");
    Serial.print(cd.Ki, 2);
    Serial.print("\tKd: ");
    Serial.print(cd.Kd, 2);
    Serial.println("");
    count = 0;
  }

  count++;
}

void BalanceState::leave() {
  leftServo.detach();
  rightServo.detach();

  writeConfig(cd);
}
