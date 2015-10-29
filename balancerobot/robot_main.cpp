#include <Arduino.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Servo.h>
//#include <avr/pgmspace.h>

#include "utils.h"
#include "hardwaredefs.h"

struct ServerOffsets {
	int leftVal;
	int rightVal;
};

struct IMUOffsets {
	vec3d<int16_t> accel;
	vec3d<int16_t> gyro;
};

struct ConfigData {
	ServerOffsets servo;
	IMUOffsets imu;
};

MPU6050 accelgyro;

bool blinkState = false;


void setup() {
	
	pinMode(8, OUTPUT);
  	digitalWrite(8, HIGH); // turn on the radio
	Wire.begin();
	Serial.begin(115200);

	Serial.println(F("Initializing I2C devices..."));
	accelgyro.initialize();

	Serial.println(F("Testing device connections..."));
	Serial.println(accelgyro.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

	pinMode(internalLEDPin, OUTPUT);		
}


struct State {
	virtual void enter () = 0;
	virtual void action () = 0;
	virtual void leave () = 0;
};

struct NullState : public State{
	void enter () {
	}

	void action () {
	}

	void leave () {
	}
};

State* currentState = NULL;

void stateGoto (State* state) {
	if (currentState)
		currentState->leave();

	currentState = state;

	if (currentState)
		currentState->enter();
}

struct GyroCalState : public State{

	RunningAverage<int16_t, 16> itemp;
	int32_t samples = 1;
	vec3d<double> accelTot;
	vec3d<double> gyroTot;

	void enter () {
		Serial.println(F("\n\nEntering: Gyro Offest Calibration"));

		samples = 1;
		clearVector(accelTot);
		clearVector(gyroTot);
	}

	void action () {
		vec3d<int16_t> accel;
		vec3d<int16_t> gyro;

		itemp << accelgyro.getTemperature();
		accelgyro.getMotion6(&accel.x, &accel.y, &accel.z, 
							 &gyro.x, &gyro.y, &gyro.z);
		addVector(accel, accelTot);
		addVector(gyro, gyroTot);

		vec3d<double> avAccel = divVector(accelTot, samples);
		vec3d<double> avGyro = divVector(gyroTot, samples);

		float temp = (float)itemp.val() / 340 + 36.53f;

		Serial.print("a/g: ");
		Serial.print(samples);			Serial.print("\t");
		Serial.print(temp); 			Serial.print("\t");
		printVec3d(avAccel, Serial);	Serial.print("\t");
		printVec3d(avGyro, Serial);		Serial.print("\n\r");

		blinkState = !blinkState;
		digitalWrite(internalLEDPin, blinkState);

		samples++;

		delay(100);


		int ch = Serial.read();
		if (ch > 0)
		{
			switch (ch)
			{
				case 'q': stateGoto(NULL); break;
			}
		}
	}

	void leave () {
	}
};


GyroCalState gyroCalState;



struct ServoCalState : public State{

	Servo leftServo;
	Servo rightServo;

	ServerOffsets offsets;

	void update () {
		leftServo.writeMicroseconds(offsets.leftVal);
		rightServo.writeMicroseconds(offsets.rightVal);
		Serial.print(offsets.leftVal);
		Serial.print(' ');
		Serial.println(offsets.rightVal);		
	}

	void enter () {
		Serial.println(F("\n\nEntering: Servo Zero Point Calibration"));
		leftServo.attach(10);
		rightServo.attach(9);

		offsets.leftVal = 1500;
		offsets.rightVal = 1500;

		update();
	}

	void action () {
		int ch = Serial.read();
		if (ch > 0)
		{
			switch (ch)
			{
				case 'q': stateGoto(NULL); break;

				case '1': offsets.leftVal -= 10; break;
				case '2': offsets.leftVal -= 1;  break;
				case '3': offsets.leftVal += 1;  break;
				case '4': offsets.leftVal += 10; break;

				case '7': offsets.rightVal -= 10; break;
				case '8': offsets.rightVal -= 1;  break;
				case '9': offsets.rightVal += 1;  break;
				case '0': offsets.rightVal += 10; break;
			}
			update();
		}
	}

	void leave () {
		leftServo.detach();
		rightServo.detach();
	}
};

ServoCalState servoCalState;

struct MenuState : public State{

	int twirly;

	void writeDefaultConfig () {
		ConfigData cd;

		cd.servo.leftVal = 1500;
		cd.servo.rightVal = 1500;

		clearVector(cd.imu.accel);
		clearVector(cd.imu.gyro);

		writeStructEEPROM(cd, 0);		
		Serial.println(F("\n  Default Config Written \n"));
		enter();
	}

	void enter () {
		Serial.println(F("Mk1 Calibration. Select"));	
		Serial.println(F("  0: Default Config"));
		Serial.println(F("  1: Gyro Offest Calibration"));
		Serial.println(F("  2: Servo Zero Point Calibration"));

		twirly = 0;
	}

	void action () {
		int ch = Serial.read();
		if (ch > 0)
		{
			switch (ch)
			{
				case '1': stateGoto(&gyroCalState); break;
				case '2': stateGoto(&servoCalState); break;
				case '0': writeDefaultConfig(); break;
			}
		}
		delay(250);
		Serial.print("\\|/-"[twirly++]);
		Serial.print('\r');
		twirly &= 3;
	}

	void leave () {
	}
};

MenuState menuState;

void loop () {
	if (!currentState)
		stateGoto(&menuState);

	if (currentState)
		currentState->action();
}

