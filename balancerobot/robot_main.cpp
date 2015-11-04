#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

//#include <avr/pgmspace.h>

#include "utils.h"
#include "hardwaredefs.h"
#include "motordrive.h"
#include "state.h"

Adafruit_BNO055 bno = Adafruit_BNO055(55);

bool blinkState = false;

struct GyroCalState : public State {
  void enter() { Serial.println(F("\n\nEntering: Gyro Offest Calibration")); }

  void action() {
    sensors_event_t event;
    bno.getEvent(&event);

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

GyroCalState gyroCalState;

struct ServoCalState : public State {
  Servo leftServo;
  Servo rightServo;

  ServoConfig lconfig;
  ServoConfig rconfig;

  MotorDrive drive;

  ServoCalState() : drive(&leftServo, &rightServo) {}

  void update() {
    drive.setServoConfig(lconfig, rconfig);
    drive.drive(0);

    Serial.print(lconfig.zeropoint);
    Serial.print(' ');
    Serial.println(rconfig.zeropoint);
  }

  void enter() {
    Serial.println(F("\n\nEntering: Servo Zero Point Calibration"));
    leftServo.attach(10);
    rightServo.attach(9);

    lconfig.zeropoint = 1500;
    rconfig.zeropoint = 1500;

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
          lconfig.zeropoint -= 10;
          break;
        case '2':
          lconfig.zeropoint -= 1;
          break;
        case '3':
          lconfig.zeropoint += 1;
          break;
        case '4':
          lconfig.zeropoint += 10;
          break;

        case '7':
          rconfig.zeropoint -= 10;
          break;
        case '8':
          rconfig.zeropoint -= 1;
          break;
        case '9':
          rconfig.zeropoint += 1;
          break;
        case '0':
          rconfig.zeropoint += 10;
          break;
      }
      update();
    }
  }

  void leave() {
    leftServo.detach();
    rightServo.detach();
  }
};

ServoCalState servoCalState;

struct MenuState : public State {
  int twirly;

  void writeDefaultConfig() {
    /*
                    ConfigData cd;

                    cd.servo.leftVal = 1500;
                    cd.servo.rightVal = 1500;

                    writeStructEEPROM(cd, 0);
                    */
    Serial.println(F("\n  Default Config Written \n"));
    enter();
  }

  void enter() {
    Serial.println(F("Mk1 Calibration. Select"));
    Serial.println(F("  0: Default Config"));
    Serial.println(F("  1: Gyro Offest Calibration"));
    Serial.println(F("  2: Servo Zero Point Calibration"));

    twirly = 0;
  }

  void action() {
    int ch = Serial.read();
    if (ch > 0) {
      switch (ch) {
        case '1':
          stateGoto(&gyroCalState);
          break;
        case '2':
          stateGoto(&servoCalState);
          break;
        case '0':
          writeDefaultConfig();
          break;
      }
    }
    delay(250);
    Serial.print("\\|/-"[twirly++]);
    Serial.print('\r');
    twirly &= 3;
  }

  void leave() {}
};

MenuState menuState;

StateMachine mainStateMachine;

void setup() {
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);  // turn on the radio
  Serial.begin(115200);

  /* Initialise the sensor */
  if (!bno.begin()) {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.println(
        F("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!\n"));
    while (1)
      ;
  }

  Serial.println(F("\n\nBNO055 detected OK\n\n"));
  delay(1000);
  bno.setExtCrystalUse(true);
  pinMode(internalLEDPin, OUTPUT);

  // initial state is menu
  mainStateMachine.stateGoto(&menuState);
}

void loop() { mainStateMachine.stateAction(); }
