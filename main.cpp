// Plotclock
// cc - by Johannes Heberlein 2014
// v 1.02
// thingiverse.com/joo   wiki.fablab-nuernberg.de

// PlotClock_zoomIn_LOT on nodemcu-32s
// Base on Plotclock
// Wong NingChi   https://www.instructables.com/member/wnq/instructables/
// English: https://www.instructables.com/id/PlotClockzoomInLOT/
// chinese: https://create.arduino.cc/projecthub/flyingPig507/plotclock-lot-7f5ac4

#include <Arduino.h>
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>

#define CALIBRATION // enable calibration mode
// When in calibration mode, adjust the following factor until the servos move exactly 90 degrees
#define SERVOFAKTORLEFT 700
#define SERVOFAKTORRIGHT 700

// Zero-position of left and right servo
// When in calibration mode, adjust the NULL-values so that the servo arms are at all times parallel
// either to the X or Y axis
#define SERVOLEFTNULL 1800
#define SERVORIGHTNULL 570

#define SERVOPINLIFT 21
#define SERVOPINLEFT 23
#define SERVOPINRIGHT 22

// length of arms
#define L1 90.0
#define L2 143.7
#define L3 12.0

// origin points of left and right servo
#define O1X 30
#define O1Y -80
#define O2X 120
#define O2Y -80

// lift positions of lifting servo
#define LIFT0 1500 // on drawing surface
#define LIFT1 1300  // between numbers
#define LIFT2 1000  // going towards sweeper

// speed of liftimg arm, higher is slower
//#define LIFTSPEED 1500
#define LIFTSPEED 2

int servoLift = 1500;

Servo servo1; //
Servo servo2; //
Servo servo3; //

#define darwAreaWidth 148
#define darwAreaHeight 120

volatile double lastX = darwAreaWidth;
volatile double lastY = darwAreaHeight;

// NTP
const char *ssid = "your_wifi_name";
const char *password = "your_wifi_password";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "cn.pool.ntp.org", 28800); //offset is china +8, 8h*60m*60s
//NTPClient timeClient(ntpUDP);

//path of move
const int rows = 3;
const int _path_max = 4;
int _path_of_move[_path_max][rows] = {
    {60, 50, 0},
    {80, 50, 0},
    {60, 80, 0},
    {30, 80, 0}
    };

// best area of playform, 4 corner
/* int _path_of_move[_path_max][rows] = {
    {40, 30, 0},    //left up
    {110, 30, 1},   //right up
    {150, 100, 2},  //right down
    {0, 100, 1}     //left down
    };
 */

int _alarm_H_1 = 7;
int _alarm_M_1 = 59;
int _alarm_H_2 = 16;
int _alarm_M_2 = 30;
bool _isAlarm_1 = false;
bool _isAlarm_2 = false;
int _alarm_offset = 2;
int _oldDay = 0;
int _newDay = 0;

double return_angle(double a, double b, double c)
{
  // cosine rule for angle between c and a
  return acos((a * a + c * c - b * b) / (2 * a * c));
}

void set_XY(double Tx, double Ty)
{
  delay(1);
  double dx, dy, c, a1, a2, Hx, Hy;

  // calculate triangle between pen, servoLeft and arm joint
  // cartesian dx/dy
  dx = Tx - O1X;
  dy = Ty - O1Y;

  // polar lemgth (c) and angle (a1)
  c = sqrt(dx * dx + dy * dy); //
  a1 = atan2(dy, dx);          //
  a2 = return_angle(L1, L2, c);

  servo2.writeMicroseconds(floor(((a2 + a1 - M_PI) * SERVOFAKTORLEFT) + SERVOLEFTNULL));

  // calculate joinr arm point for triangle of the right servo arm
  a2 = return_angle(L2, L1, c);
  Hx = Tx + L3 * cos((a1 - a2 + 0.621) + M_PI); //36,5°
  Hy = Ty + L3 * sin((a1 - a2 + 0.621) + M_PI);

  // calculate triangle between pen joint, servoRight and arm joint
  dx = Hx - O2X;
  dy = Hy - O2Y;

  c = sqrt(dx * dx + dy * dy);
  a1 = atan2(dy, dx);
  a2 = return_angle(L1, (L2 - L3), c);

  servo3.writeMicroseconds(floor(((a1 - a2) * SERVOFAKTORRIGHT) + SERVORIGHTNULL));
}

void drawTo(double pX, double pY)
{
  double dx, dy, c;
  int i;

  // dx dy of new point
  dx = pX - lastX;
  dy = pY - lastY;
  //path lenght in mm, times 4 equals 4 steps per mm
  c = floor(4 * sqrt(dx * dx + dy * dy));

  if (c < 1)
    c = 1;

  for (i = 0; i <= c; i++)
  {
    // draw line point by point
    set_XY(lastX + (i * dx / c), lastY + (i * dy / c));
  }

  lastX = pX;
  lastY = pY;
}

void lift(char lift)
{
  switch (lift)
  {
    // room to optimize  !

  case 0: //850

    if (servoLift >= LIFT0)
    {
      while (servoLift >= LIFT0)
      {
        servoLift--;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }
    }
    else
    {
      while (servoLift <= LIFT0)
      {
        servoLift++;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }
    }

    break;

  case 1: //150

    if (servoLift >= LIFT1)
    {
      while (servoLift >= LIFT1)
      {
        servoLift--;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }
    }
    else
    {
      while (servoLift <= LIFT1)
      {
        servoLift++;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }
    }

    break;

  case 2:

    if (servoLift >= LIFT2)
    {
      while (servoLift >= LIFT2)
      {
        servoLift--;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }
    }
    else
    {
      while (servoLift <= LIFT2)
      {
        servoLift++;
        servo1.writeMicroseconds(servoLift);
        delayMicroseconds(LIFTSPEED);
      }
    }
    break;
  }
}

void moveArm()
{
  for (int i = 0; i < _path_max; i++)
  {
    drawTo(_path_of_move[i][0], _path_of_move[i][1]);
    lift(_path_of_move[i][2]);
    delay(1000);
  }
}

void newDayisDone()
{
  _alarm_offset = random(5);
  _isAlarm_1 = false;
  _isAlarm_2 = false;
}

void setup()
{

  Serial.begin(115200);

  servo1.attach(SERVOPINLIFT);  //  lifting servo
  servo2.attach(SERVOPINLEFT);  //  left servo
  servo3.attach(SERVOPINRIGHT); //  right servo
#ifdef CALIBRATION

#else
  WiFi.begin(ssid, password);
  Serial.print("Wifi connecting ");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  timeClient.begin();

#endif

  delay(1000);
}

void loop()
{

#ifdef CALIBRATION
  // Servohorns will have 90° between movements, parallel to x and y axis
  /* drawTo(0, darwAreaHeight * 0.6);
  delay(1000);
  drawTo(darwAreaWidth, darwAreaHeight * 0.6);
  delay(1000); */

  moveArm(); //test

#else
  

   timeClient.update(); //need check wifi is OK?
  // Serial.print("time : ");
  // Serial.println(timeClient.getFormattedTime());
  // Serial.print("day : ");
  // Serial.print(timeClient.getDay());
  // Serial.print(", h : ");
  // Serial.print(timeClient.getHours());
  // Serial.print(", m : ");
  // Serial.print(timeClient.getMinutes());
  // Serial.print(", s : ");
  // Serial.print(timeClient.getSeconds());
  // Serial.println();
  delay(60000); //time loop time

  int hour = timeClient.getHours();
  int min = timeClient.getMinutes();
  _newDay = timeClient.getDay();
  if (_newDay != _oldDay)
  {
    _oldDay = _newDay;
    newDayisDone();
  }

  if (hour >= _alarm_H_1 && hour < _alarm_H_2 && !_isAlarm_1)
  {
    if (min >= (_alarm_M_1 - _alarm_offset) && min < _alarm_M_2)
    {
      moveArm();
      _isAlarm_1 = true;
    }
  }

  if (hour >= _alarm_H_2 && !_isAlarm_2)
  {
    if (min >= (_alarm_M_2 + _alarm_offset))
    {
      moveArm();
      _isAlarm_2 = true;
    }
  } 
#endif
}
