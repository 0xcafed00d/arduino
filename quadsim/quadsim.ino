#include <TimerOne.h>

int states[] = {0, 1, 1, 0};
volatile int state = 0;
volatile int dir = 1;
volatile int enable = 0;
volatile int count = 0;
long rate = 1000;

void setup() {
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  Timer1.initialize(rate);
  Timer1.attachInterrupt(intFunc);

  Serial.begin(115200);
}


void intFunc() {
  if (enable) {
    digitalWrite(10, states[state]);
    digitalWrite(9, states[state + 1 & 0x3]);
    state = (state + dir) & 0x3;
    count--;
    if (count == 0) 
      enable = false;
  }
}

void loop() {
  int ch = Serial.read();
  if (ch > 0)
  {
    switch (ch)
    {
      case 'f': dir = 1; break;
      case 'r': dir = -1; break;
      case 'e': enable = 1; break;
      case 'd': enable = 0; break;
      case 'q':
        rate += 10;
        Timer1.setPeriod(rate);
        break;
      case 'a':
        rate -= 10;
        if (rate < 100) rate = 100;
        Timer1.setPeriod(rate);
        break;
      case 'p':
        enable = 1;
        count = 20;
        break;
    }


    if (dir == 1)
      Serial.print("Forward ");
    else
      Serial.print("Backward ");

    if (enable) 
      Serial.print("Enabled ");
    else
      Serial.print("Disabled ");

    Serial.print(rate * 4);
    Serial.println("uS");

  }
}
