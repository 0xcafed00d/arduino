#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

//#include <avr/pgmspace.h>

#include "utils.h"
#include "hardwaredefs.h"

Adafruit_BNO055 bno = Adafruit_BNO055(55);

struct ServerOffsets {
	int leftVal;
	int rightVal;
};

struct ConfigData {
	ServerOffsets servo;
};

bool blinkState = false;

void setup() {
	
	pinMode(8, OUTPUT);
  digitalWrite(8, HIGH); // turn on the radio
	Serial.begin(115200);

  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.println(F("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!\n"));
    while(1);
  }

  Serial.println(F("\n\nBNO055 detected OK\n\n"));
  
  delay(1000);
      
  bno.setExtCrystalUse(true);
 
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


	void enter () {
		Serial.println(F("\n\nEntering: Gyro Offest Calibration"));

	}

	void action () {

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

