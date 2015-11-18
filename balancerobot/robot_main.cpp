#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#include "gyrocalibrate.h"
#include "servocalibrate.h"
#include "balance.h"
#include "config.h"

Adafruit_BNO055 IMU = Adafruit_BNO055(55);

GyroCalState gyroCalState(&IMU);
ServoCalState servoCalState;
BalanceState balanceState(&IMU);

struct MenuState : public State {
  int twirly;

  void enter() {
    Serial.println(F("Mk1 Calibration. Select"));
    Serial.println(F("  0: Default Config"));
    Serial.println(F("  1: Gyro Offest Calibration"));
    Serial.println(F("  2: Servo Zero Point Calibration"));
    Serial.println(F("  3: Balance"));

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
        case '3':
          stateGoto(&balanceState);
          break;
        case '0':
          writeDefaultConfig();
          enter();
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
  if (!IMU.begin()) {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.println(F("No BNO055 detected. Check your wiring or I2C ADDR!\n"));
    while (1)
      ;
  }

  Serial.println(F("\n\nBNO055 detected OK\n\n"));
  delay(1000);
  IMU.setExtCrystalUse(true);
  pinMode(internalLEDPin, OUTPUT);

  // initial state is menu
  //  mainStateMachine.stateGoto(&menuState);
}

void loop() {
  if (mainStateMachine.currentState == NULL)
    mainStateMachine.stateGoto(&menuState);

  mainStateMachine.stateAction();
}
