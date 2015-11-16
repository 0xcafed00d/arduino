#include <Arduino.h>

#include "motordrive.h"

MotorDrive::MotorDrive(Servo* lservo, Servo* rservo)
    : m_lservo(lservo), m_rservo(rservo) {
  ServoConfig c{0, 500};
  setServoConfig(c, c);
}

void MotorDrive::setServoConfig(const ServoConfig& lconf,
                                const ServoConfig& rconf) {
  m_lconf = lconf;
  m_rconf = rconf;
}

static int translate(int value, int zp, int range) {
  return map(value, -1000, 1000, zp - range, zp + range);
}

// drive value range is -1000 -> 1000
void MotorDrive::drive(int value) {
  m_lservo->writeMicroseconds(
      translate(-value, m_lconf.zeropoint, m_lconf.range));
  m_rservo->writeMicroseconds(
      translate(value, m_rconf.zeropoint, m_rconf.range));
}

// drive value range is -1000 -> 1000
void MotorDrive::drive(int lvalue, int rvalue) {
  m_lservo->writeMicroseconds(
      translate(-lvalue, m_lconf.zeropoint, m_lconf.range));
  m_rservo->writeMicroseconds(
      translate(rvalue, m_rconf.zeropoint, m_rconf.range));
}

void MotorDrive::setRaw(int lvalue, int rvalue) {
  m_lservo->writeMicroseconds(lvalue);
  m_rservo->writeMicroseconds(rvalue);
}
