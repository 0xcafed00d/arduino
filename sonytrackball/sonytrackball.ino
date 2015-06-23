#include <TimerOne.h>

void setup() {
  Serial.begin(115200);

  Timer1.initialize(100);
  Timer1.attachInterrupt(timerISR);

  attachInterrupt(0, xAxisISR, RISING);

  pinMode(2, INPUT);

}

volatile long timeMS = 0;
volatile long lastTimeMS = 0;
volatile long xRate = 0;
volatile char dir = ' ';

void timerISR() {
  timeMS++;
}

void xAxisISR() {
  xRate = timeMS -  lastTimeMS;
  lastTimeMS = timeMS;
  
  dir = 'f';
  if (digitalRead(2)){
    dir = 'r';
  }
}

void loop() {
  
  noInterrupts();
  long rate = xRate;
  interrupts();
  Serial.print (10000L/rate);
  Serial.println (dir);
}
