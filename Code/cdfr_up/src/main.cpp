#include <Arduino.h>
#include <Servo.h>

#define PIN_SERVO_RIGHT 11
#define PIN_SERVO_LEFT 10
#define PIN_SERVO_INNER 6
#define PIN_SERVO_OUTER 5
#define PIN_CUR_RIGHT A0
#define PIN_CUR_LEFT A1
#define PIN_CUR_INNER A2
#define PIN_CUR_OUTER A3

void detachAll();
void parseInput(uint8_t);
void attachRight();
void attachLeft();
void attachInner();
void attachOuter();
void detachRight();
void detachLeft();
void detachInner();
void detachOuter();
void initRightLeft(int);
void initRight();
void initLeft();
void initInnerOuter();
void initInner();
void initOuter();
void closeRightLeft();
void closeRight();
void closeLeft();
void openRight();
void openLeft();
// void closeInner();
// void closeOuter();
// void setInner(float);
// void setOuter(float);

int current(int);

Servo servoRight;
Servo servoLeft;
Servo servoInner;
Servo servoOuter;

float curRight = 0.0;
float curLeft = 0.0;
float curInner = 0.0;
float curOuter = 0.0;
float comRight = 0.0;
float comLeft = 0.0;
float comInner = 0.0;
float comOuter = 0.0;

float midRight = 0.0;
float midLeft = 0.0;
float minInner = 0.0;
float minOuter = 0.0;
float maxInner = 0.0;
float maxOuter = 0.0;
float errLeft = 0.0;
float errRight = 0.0;
float errInner = 0.0;
float errOuter = 0.0;
uint8_t input;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0)
  {
    input = Serial.read();
    parseInput(input - 65);
    Serial.print("Input: ");
    Serial.println((int)input);
  }
  else
  {
    Serial.println("No input");
  }

  if (servoRight.attached())
  {
    if (curRight > comRight)
    {
      curRight = max(curRight + 1.0, comRight);
      servoRight.write(curRight);
    }
    else if (curRight < comRight)
    {
      curRight = min(curRight - 1.0, comRight);
      servoRight.write(curRight);
    }
    if (current(analogRead(PIN_CUR_RIGHT)) > 250)
    {
      comRight = curRight;
    }
  }
  if (servoLeft.attached())
  {
    if (curLeft > comLeft)
    {
      curLeft = max(curLeft + 1.0, comLeft);
      servoLeft.write(curLeft);
    }
    else if (curLeft < comLeft)
    {
      curLeft = min(curLeft - 1.0, comLeft);
      servoLeft.write(curLeft);
    }
    if (current(analogRead(PIN_CUR_LEFT)) > 250)
    {
      comLeft = curLeft;
    }
  }

  delay(30);
}

int current(int read_analog)
{
  return read_analog * 49; // 0.1 ohm resistor, return in mA
}

void detachAll()
{
  servoRight.detach();
  servoLeft.detach();
  servoInner.detach();
  servoOuter.detach();
}

void parseInput(uint8_t input)
{
  if (input == 0)
  {
    detachAll();
  }
  else if (input == 1)
  {
    attachRight();
  }
  else if (input == 2)
  {
    attachLeft();
  }
  else if (input == 3)
  {
    attachInner();
  }
  else if (input == 4)
  {
    attachOuter();
  }
  else if (input == 5)
  {
    detachRight();
  }
  else if (input == 6)
  {
    detachLeft();
  }
  else if (input == 7)
  {
    detachInner();
  }
  else if (input == 8)
  {
    detachOuter();
  }
  else if (input == 9)
  {
    Serial.println("Init");
    initRight();
    initLeft();
    // initInner();
    // initOuter();
  }
  else if (input == 10)
  {
    closeRight();
  }
  else if (input == 11)
  {
    closeLeft();
  }
  else if (input == 12)
  {
    // closeInner();
  }
  else if (input == 13)
  {
    // closeOuter();
  }
  else if (input == 14)
  {
    openRight();
  }
  else if (input == 15)
  {
    openLeft();
  }
  // TODO angles with range
  // TODO close/open both
}

void attachRight()
{
  if (!servoRight.attached())
  {
    servoRight.attach(PIN_SERVO_RIGHT);
  }
}

void attachLeft()
{
  if (!servoLeft.attached())
  {
    servoLeft.attach(PIN_SERVO_LEFT);
  }
}

void attachInner()
{
  if (!servoInner.attached())
  {
    servoInner.attach(PIN_SERVO_INNER);
  }
}

void attachOuter()
{
  if (!servoOuter.attached())
  {
    servoOuter.attach(PIN_SERVO_OUTER);
  }
}

void detachRight()
{
  if (servoRight.attached())
  {
    servoRight.detach();
  }
}

void detachLeft()
{
  if (servoLeft.attached())
  {
    servoLeft.detach();
  }
}

void detachInner()
{
  if (servoInner.attached())
  {
    servoInner.detach();
  }
}

void detachOuter()
{
  if (servoOuter.attached())
  {
    servoOuter.detach();
  }
}

void initRightLeft(int pin)
{
  if (servoRight.attached())
  {
    float cur = 90.0;
    float min = 0.0;
    float max = 180.0;
    servoRight.write(cur);
    delay(200);
    while (current(analogRead(pin)) < 250 && cur > 0) // TODO find appropriate value instead of 250
    {
      cur = cur - 1.0;
      servoRight.write(cur);
      delay(20);
    }
    min = cur;
    delay(100);
    while (current(analogRead(pin)) < 250 && cur < 180)
    {
      cur = cur + 1.0;
      servoRight.write(cur);
      delay(20);
    }
    max = cur;
    delay(100);
    midRight = (min + max) / 2.0;
    comRight = midRight;
  }
}

void initRight() // TODO fuse both
{
  initRightLeft(PIN_CUR_RIGHT);
}

void initLeft()
{
  initRightLeft(PIN_CUR_LEFT);
}

void openRight()
{
  if (servoRight.attached())
  {
    servoRight.write(midRight);
    curRight = midRight;
    comRight = midRight;
  }
}

void openLeft()
{
  if (servoLeft.attached())
  {
    servoLeft.write(midLeft);
    curLeft = midLeft;
    comLeft = midLeft;
  }
}

void closeRightLeft()
{
  if (servoRight.attached())
  {
    comRight = 0;
    comLeft = 180;
  }
}

void closeRight()
{
  if (servoRight.attached())
  {
    comRight = 0;
  }
}

void closeLeft()
{
  if (servoLeft.attached())
  {
    comLeft = 180;
  }
}

void initInnerOuter()
{
  if (servoInner.attached() && servoOuter.attached())
  {
    float maxOuter = 0.0;
    float minOuter = 0.0;
    float maxInner = 0.0;
    float minInner = 0.0;
    float curOuter = 90.0;
    float curInner = 90.0;
    servoInner.write(curInner);
    servoOuter.write(curOuter);
    delay(200);
  }
}
