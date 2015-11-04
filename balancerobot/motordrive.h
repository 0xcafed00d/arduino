#ifndef __MOTORDRIVE_INCLUDED_H__
#define __MOTORDRIVE_INCLUDED_H__

#include <Servo.h>

struct ServoConfig {
  int zeropoint;
  int range;
};

class MotorDrive {
 private:
  ServoConfig m_lconf;
  ServoConfig m_rconf;
  Servo* m_lservo;
  Servo* m_rservo;

 public:
  MotorDrive(Servo* lservo, Servo* rservo);
  void setServoConfig(const ServoConfig& lconf, const ServoConfig& rconf);

  void drive(int value);
  void drive(int lvalue, int rvalue);

  void setRaw(int lvalue, int rvalue);
};

#endif
