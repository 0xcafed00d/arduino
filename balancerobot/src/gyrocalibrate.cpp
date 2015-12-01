#include "gyrocalibrate.h"

GyroCalState::GyroCalState(Adafruit_BNO055* imu) : imu(imu) {}

void GyroCalState::enter() {
  Serial.println(F("\n\nEntering: Gyro Offest Calibration"));
}

void GyroCalState::action() {
  sensors_event_t event;
  imu->getEvent(&event);

  /* Display the floating point data */
  Serial.print("X: ");
  Serial.print(event.orientation.x, 4);
  Serial.print("\tY: ");
  Serial.print(event.orientation.y, 4);
  Serial.print("\tZ: ");
  Serial.print(event.orientation.z, 4);
  Serial.println("");

  delay(100);

  int ch = Serial.read();
  if (ch > 0) {
    switch (ch) {
      case 'q':
        stateGoto(NULL);
        break;
    }
  }
}

void GyroCalState::leave() {}
