#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <EEPROM.h>

#include "utils.h"
#include "hardwaredefs.h"
#include "motordrive.h"
#include "state.h"

struct GyroCalState : public State {
  Adafruit_BNO055* imu;

  GyroCalState(Adafruit_BNO055* imu) : imu(imu) {}

  void enter() { Serial.println(F("\n\nEntering: Gyro Offest Calibration")); }

  void action() {
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

  void leave() {}
};
