// list of all libs the project uses.
// all other source moved to cpp & h files as the arduino 
// IDE barfs on template functions in ino files
#include <I2Cdev.h>
#include <MPU6050.h>
#include <EEPROM.h>
#include <Servo.h>


#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
	#include <Wire.h>
#endif
