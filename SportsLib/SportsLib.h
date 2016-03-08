/*
  SportsLib.h - Library for mechatronic subsystems
  ME210 Stanford University Win2016
  Created by Lawrence Rogers
*/

/******************* SPORTS MOTOR ******************/

#ifndef SportsMotor_h
#define SportsMotor_h

#include "Arduino.h"

class SportsMotor {
  public:
    SportsMotor(int d_pin, int e_pin);
    void initialize();
    
    void setEnabled(bool e);
    void setDirectionAndSpeed(int direction, int speed);

    int getDirection();

    const static int FORWARD = 0;
    const static int BACKWARD = 1;
    const static int SPEED_MAX = 127;

  private:
    int _d_pin;
    int _e_pin;
    int _direction;
};

#endif

/******************* SPORTS PHOTO TRANSISTOR ******************/

#ifndef SportsPhototransistor
#define SportsPhototransistor_h

#include "Arduino.h"

class SportsPhototransistor {
  public:
    SportsPhototransistor(int _pin);
    void initialize();
    
    int getLevel();
    int getMaxLevel();
    int getPreviousPeriod();
    bool isTriggered();
    bool isTriggered1k();

    const static int TRIGGER_PERIOD_MICROS = 200;
    const static int TRIGGER_PERIOD_MICROS_1k = 1000;

  private:
  	int _pin;
  	static void interrupt();
};

#endif

/******************* SPORTS LINE SENSOR ******************/

#ifndef SportsLineSensor
#define SportsLineSensor_h

#include "Arduino.h"

class SportsLineSensor {
  public:
    SportsLineSensor(int pin, int triggerLevel);
    bool isTriggered();
    int getLevel();

    const static int LEVEL_MAX = 1023;
    const static int MV_500_LEVEL = 102;

  private:
  	int _pin;
  	int _triggerLevel;
};

#endif
