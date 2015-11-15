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
#include "config.h"

struct ServoCalState : public State {
  Servo leftServo;
  Servo rightServo;

  ConfigData cd;
  MotorDrive drive;

  ServoCalState() : drive(&leftServo, &rightServo) {}

  void update() {
    drive.setServoConfig(cd.lconfig, cd.rconfig);
    drive.drive(0);

    Serial.print(cd.lconfig.zeropoint);
    Serial.print(' ');
    Serial.println(cd.rconfig.zeropoint);
  }

  void enter() {
    Serial.println(F("\n\nEntering: Servo Zero Point Calibration"));
    leftServo.attach(servoLeftPin);
    rightServo.attach(servoRightPin);

    if (!readConfig(cd)) {
      writeDefaultConfig();
      readConfig(cd);
    }

    update();
  }

  void action() {
    int ch = Serial.read();
    if (ch > 0) {
      switch (ch) {
        case 'q':
          stateGoto(NULL);
          break;

        case '1':
          cd.lconfig.zeropoint -= 10;
          break;
        case '2':
          cd.lconfig.zeropoint -= 1;
          break;
        case '3':
          cd.lconfig.zeropoint += 1;
          break;
        case '4':
          cd.lconfig.zeropoint += 10;
          break;

        case '7':
          cd.rconfig.zeropoint -= 10;
          break;
        case '8':
          cd.rconfig.zeropoint -= 1;
          break;
        case '9':
          cd.rconfig.zeropoint += 1;
          break;
        case '0':
          cd.rconfig.zeropoint += 10;
          break;
      }
      update();
    }
  }

  void leave() {
    leftServo.detach();
    rightServo.detach();

    writeConfig(cd);

    writeStructEEPROM(cd, 0);
  }
};
