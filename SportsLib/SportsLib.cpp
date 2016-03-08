/*
  SportsLib.cpp - Library for mechatronic subsystems
  ME210 Stanford University Win2016
  Created by Lawrence Rogers
*/

#include "Arduino.h"
#include "SportsLib.h"

/******************* SPORTS MOTOR ******************/

SportsMotor::SportsMotor(int d, int e) {
 	_d_pin = d;
 	_e_pin = e;
}

void SportsMotor::initialize() {
 	pinMode(_e_pin, OUTPUT);
 	pinMode(_d_pin, OUTPUT);
 	setEnabled(false);
 	setDirectionAndSpeed(FORWARD, 0);
 	setDirectionAndSpeed(FORWARD, 0);
}

void SportsMotor::setEnabled(bool enabled) {
	if (enabled) 
		digitalWrite(_e_pin, HIGH);
	else 
		digitalWrite(_e_pin, LOW);
}

/* setDirectionAndSpeed
 * @param direction - either DIRECTION_FORWARD or DIRECTION_BACKWARD
 * @param speed - int from 0 to 127
 * sets the speed and direction of the motor.
 */ 
void SportsMotor::setDirectionAndSpeed(int direction, int speed) {
	if (direction == FORWARD) {
		analogWrite(_d_pin, 127 + speed);
		_direction = direction;
	}
	else if (direction == BACKWARD) {
		analogWrite(_d_pin, 127 - speed);
		_direction = direction;
	}
}

int SportsMotor::getDirection() {
	return _direction;
}


/******************* SPORTS PHOTO TRANSISTOR ******************/

static volatile unsigned long previous_time_micros = 0;
static volatile unsigned long previous_period_micros = 0;

SportsPhototransistor::SportsPhototransistor(int pin) {
	_pin = pin;
}

/* Phototransistor initialize
 * binds the pin to the interrupt function
 */
void SportsPhototransistor::initialize() {
	attachInterrupt(digitalPinToInterrupt(_pin), interrupt, RISING);
}

void SportsPhototransistor::interrupt() {
	int current_time_micros = micros();
	previous_period_micros = current_time_micros - previous_time_micros;
	previous_time_micros = micros();
}

int SportsPhototransistor::getPreviousPeriod() {
	return previous_period_micros;
}

bool SportsPhototransistor::isTriggered() {
	unsigned long period = getPreviousPeriod();
	return (period > TRIGGER_PERIOD_MICROS * .95 && period < TRIGGER_PERIOD_MICROS * 1.05);
}

bool SportsPhototransistor::isTriggered1k() {
	unsigned long period = getPreviousPeriod();
	return (period > TRIGGER_PERIOD_MICROS_1k * .95 && period < TRIGGER_PERIOD_MICROS_1k * 1.05);
}
/******************* SPORTS LINE SENSOR ******************/

SportsLineSensor::SportsLineSensor(int pin, int triggerLevel) {
	_pin = pin;
	_triggerLevel = triggerLevel;
}

int SportsLineSensor::getLevel() {
	return analogRead(_pin);
}

bool SportsLineSensor::isTriggered() {
	int level = getLevel();
	return level < _triggerLevel;
}