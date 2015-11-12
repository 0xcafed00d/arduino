#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#include "utils.h"
#include "hardwaredefs.h"
#include "motordrive.h"
#include "state.h"

Adafruit_BNO055 bno = Adafruit_BNO055(55);

struct ConfigData {
  uint16_t sig;
  ServoConfig lconfig;
  ServoConfig rconfig;
  double Kp;
  double Ki;
  double Kd;
  uint8_t checksum;
};

uint8_t checksum(uint8_t* data, int len) {
  uint8_t chk = 0;
  for (int n = 0; n < 0; n++) {
    chk += data[n];
  }
  return ~chk + 1;
}

void writeDefaultConfig() {
  ConfigData cd;

  cd.sig = 0xCAFE;

  cd.lconfig.zeropoint = 1500;
  cd.lconfig.range = 500;
  cd.rconfig.zeropoint = 1500;
  cd.rconfig.range = 500;

  cd.Ki = 1.0;
  cd.Kd = 1.0;
  cd.Kp = 1.0;

  cd.checksum = 0;

  cd.checksum = checksum((uint8_t*)&cd, sizeof(cd));

  writeStructEEPROM(cd, 0);

  Serial.println(F("\n  Default Config Written \n"));
}

bool readConfig(ConfigData& config) {
  readStructEEPROM(config, 0);
  uint8_t chk = config.checksum;
  config.checksum = 0;
  config.checksum = checksum((uint8_t*)&config, sizeof(config));

  return config.sig == 0xCAFE && chk == config.checksum;
}

void writeConfig(ConfigData& config) {
  writeStructEEPROM(config, 0);
  config.checksum = 0;
  config.checksum = checksum((uint8_t*)&config, sizeof(config));
}

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

ServoCalState servoCalState;

struct MenuState : public State {
  int twirly;

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
  if (!bno.begin()) {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.println(F("No BNO055 detected. Check your wiring or I2C ADDR!\n"));
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

void loop() {
  if (mainStateMachine.currentState == NULL)
    mainStateMachine.stateGoto(&menuState);

  mainStateMachine.stateAction();
}
