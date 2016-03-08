#include <SportsLib.h>
#include <Servo.h>
#include <math.h>

/********** OUTPUT PINS****************/
#define LEFT_MOTOR_D_PIN    3 // must be these for PWM to work
#define LEFT_MOTOR_E_PIN    4
#define RIGHT_MOTOR_D_PIN   5
#define RIGHT_MOTOR_E_PIN   6

#define SERVO_1_PIN         8
#define SERVO_2_PIN         9
#define SERVO_3_PIN         10

/*********** INPUT PINS ****************/
#define LEFT_LINE_SENSOR_PIN      3//
#define MIDDLE_LINE_SENSOR_PIN    0//
#define RIGHT_LINE_SENSOR_PIN     1//
#define FAR_LEFT_LINE_SENSOR_PIN  2//

#define PHOTOTRANSISTOR_PIN 2 // must be pin 2 for interrupt to work

/*********** SENSOR THRESHOLDS **********/
// max is ~500mv, which  is .1 * 1023 = 102. half should be a good trigger, so .5 * 100 = 50
#define LINE_SENSOR_TRIGGER_LEVEL 50

/********** SPORTS BOT *****************/
#define DIRECTION_CW            0
#define DIRECTION_CCW           1

#define STAGE_LOCATE            0

#define STATE_DEBUG             10

/********** LOCATING STATES **********/
#define STATE_WAITING_TO_BEGIN          0 // turn power off until code uploaded
#define STATE_LOCATING_CW               1 // turning around looking for beacon
#define STATE_APPROACHING_LINES         2 // beacons found, turned a bit, going toward lines
#define STATE_CROSSING_VERTICAL_LINE    3 // left sensor was hit first, crossing the vertical line
#define STATE_TURNING_ON_VERTICAL_LINE  4 // right sensor was hit, rotate a bit and keep going
#define STATE_CROSSING_HORIZ_LINE       5 // right sensor was hit, crossing horizontal line
#define STATE_TURNING_ON_HORIZ_LINE     6 // turn a bit then keep going
#define STATE_ROTATING_ON_HORIZ_LINE    7 // T was detected, turn right until far left sensor is off
#define STATE_ROTATING_TO_HORIZ_LINE    8 // keep turning to orient toward buckets
#define STATE_BACKING_TO_T              9

int stage;
int state;


bool serial_flag = true;

/********** SUB SYSTEMS ****************/
SportsMotor leftMotor(LEFT_MOTOR_D_PIN, LEFT_MOTOR_E_PIN);
SportsMotor rightMotor(RIGHT_MOTOR_D_PIN, RIGHT_MOTOR_E_PIN);

SportsLineSensor leftSensor(LEFT_LINE_SENSOR_PIN, LINE_SENSOR_TRIGGER_LEVEL);
SportsLineSensor rightSensor(RIGHT_LINE_SENSOR_PIN, LINE_SENSOR_TRIGGER_LEVEL);
SportsLineSensor middleSensor(MIDDLE_LINE_SENSOR_PIN, LINE_SENSOR_TRIGGER_LEVEL);
SportsLineSensor farLeftSensor(FAR_LEFT_LINE_SENSOR_PIN, LINE_SENSOR_TRIGGER_LEVEL);

SportsPhototransistor photoT(PHOTOTRANSISTOR_PIN);

Servo servo1;
Servo servo2;
Servo servo3;

void setup() {

  if (serial_flag) {
    Serial.begin(9600);
  }

  //init components
  leftMotor.initialize();
  rightMotor.initialize();

  leftMotor.setEnabled(true);
  rightMotor.setEnabled(true);

  photoT.initialize();

  /********* START LOCATING ************/
  stage = STAGE_LOCATE;
  state = STATE_LOCATING_CW;

  delay(3000);
  turn(DIRECTION_CW, 0, .4, .4);
  state = STATE_LOCATING_CW;
}

void loop() {
  if (serial_flag) {
    if (TestForKey()) RespToKey();
  }

  if (stage == STAGE_LOCATE)
    locateStageLoop();

}

/************** LOOP FUNCTIONS ****************/
/* void locateStageLoop()
   loop function for bot while in locate stage.
   performs different actions based on which state the bot is in
*/
int line_follow_counter = -1;
void locateStageLoop() {
  switch (state) {
    case STATE_WAITING_TO_BEGIN:
      //do nothing
      break;

    case STATE_LOCATING_CW:
      if (photoT.isTriggered()) {
        pauseMotors(500);
        turn(DIRECTION_CW, 900, .5, .4);
        goStraight(.3);
        state = STATE_APPROACHING_LINES;
      }
      break;

    case STATE_APPROACHING_LINES:
      if (leftSensor.isTriggered()) {
        state = STATE_CROSSING_VERTICAL_LINE; //hit vertical line
        if (serial_flag) Serial.println("left sensor triggered, state = crossing vertical line");
      }
      if (rightSensor.isTriggered()) {
        state = STATE_CROSSING_HORIZ_LINE; //hit horizontal
        if (serial_flag) Serial.println("right sensor triggered, state = crossing horiz line");
      }
      break;

    case STATE_CROSSING_HORIZ_LINE:
      if (rightSensor.isTriggered()) {
        line_follow_counter += 1;
        pauseMotors(200);
        turn(DIRECTION_CCW, 0, .4, .3);
        state = STATE_TURNING_ON_HORIZ_LINE;
      }
      if (detectT()) {
        goStraight(.4);
        delay(725);
        leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
        rightMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
        score();
        pauseMotors(10000);
      }
      break;

    case STATE_TURNING_ON_HORIZ_LINE:
      if (!rightSensor.isTriggered()) {
        int pause = exp(line_follow_counter * -.5) * 100;
        delay(pause); // keep turning a bit
        goStraight(.4);
        state = STATE_CROSSING_HORIZ_LINE;
      }
      if (detectT()) {
        goStraight(.4);
        delay(725);
        leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
        rightMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
        score();
        pauseMotors(1000000);
        state = STATE_ROTATING_ON_HORIZ_LINE;
      }
      break;

    case STATE_ROTATING_ON_HORIZ_LINE:
      if (!farLeftSensor.isTriggered()) {
        turn(DIRECTION_CW, 0, .4, .4);
        goStraight(.4);
        delay(400);
        score();
        pauseMotors(100000);
        state = STATE_ROTATING_TO_HORIZ_LINE;
      }
      break;
      
    case STATE_ROTATING_TO_HORIZ_LINE:
      if (detectT()) {
        score();
        pauseMotors(10000);
      }

      break;

    case STATE_CROSSING_VERTICAL_LINE:
      if (rightSensor.isTriggered()) {
        if (serial_flag) Serial.println("right sensor triggered, stopping");
        pauseMotors(200);
        turn(DIRECTION_CW, 0, .3, .4);
        state = STATE_TURNING_ON_VERTICAL_LINE;
      }

      if (detectT()) {
        leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
        rightMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);

        Serial.println("QUADFECTA");
        state = STATE_DEBUG;
      }
      break;

    case STATE_TURNING_ON_VERTICAL_LINE:
      if (!rightSensor.isTriggered()) {
        delay(200); //keep turning for a bit
        goStraight(.4);
        state = STATE_CROSSING_VERTICAL_LINE;
      }
      if (detectT()) {
        //reached T
        leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
        rightMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
        Serial.println("QUADFECTA");
        state = STATE_DEBUG;
      }
      break;
      
    case STATE_DEBUG:
      //Serial.println("State = debug");
      if (leftSensor.isTriggered()) {
        Serial.println("LEFT SENSOR TRIGGERED");
      }
      if (rightSensor.isTriggered()) {
        Serial.println("RIGHT SENSOR TRIGGERED");
      }
      if (middleSensor.isTriggered()) {
        Serial.println("MIDDLE SENSOR TRIGGERED");
      }
      if (farLeftSensor.isTriggered()) {
        Serial.println("FAR LEFT SENSOR TRIGGERED");
      }
      //Serial.print("right sensor: ");
      //Serial.println(rightSensor.getLevel());
  }
}

bool detectT() {
  return (farLeftSensor.isTriggered() && middleSensor.isTriggered() && rightSensor.isTriggered());
}

/********** NAVIGATION FUNCTIONS *************/
/*
   stop(long duration)
   @param duration time to stop in milliseconds
   BLOCKING CODE
*/
void pauseMotors(long duration) {
  leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
  rightMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
  delay(duration);
}
/*
   function turn
   @param direction: DIRECTION_CW or DIRECTION_CCW
   @millis: how long to turn, if 0 then do not block

*/
void turn(int direction, long millis, float leftSpeed, float rightSpeed) {
  if (direction == DIRECTION_CW) {
    leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, leftSpeed * SportsMotor::SPEED_MAX);
    rightMotor.setDirectionAndSpeed(SportsMotor::BACKWARD, rightSpeed * SportsMotor::SPEED_MAX);
  }
  else if (direction == DIRECTION_CCW) {
    leftMotor.setDirectionAndSpeed(SportsMotor::BACKWARD, leftSpeed * SportsMotor::SPEED_MAX);
    rightMotor.setDirectionAndSpeed(SportsMotor::FORWARD, rightSpeed * SportsMotor::SPEED_MAX);
  }
  if (millis != 0)
    delay(millis);
}
/* score()
 * initializes the three servos
 * sequentially fires the servos and then retracts them
 */
void score() {
  pinMode(SERVO_1_PIN, OUTPUT); servo1.attach(SERVO_1_PIN); servo1.write(0);
  pinMode(SERVO_2_PIN, OUTPUT); servo2.attach(SERVO_2_PIN); servo2.write(180);
  pinMode(SERVO_3_PIN, OUTPUT); servo3.attach(SERVO_3_PIN); servo3.write(180);
  
  servo3.write(0);
  delay(1000);
  servo2.write(45);
  delay(1000);
  servo1.write(135);
  delay(2000);

  servo1.write(0);
  delay(1000);
  servo2.write(180);
  delay(1000);
  servo3.write(180);
}
/*
   goStraight
   speed float 0-1 of max speed
*/
void goStraight(float speed) {
  leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, speed * SportsMotor::SPEED_MAX);
  rightMotor.setDirectionAndSpeed(SportsMotor::FORWARD, (speed - .02) * SportsMotor::SPEED_MAX);
}

void goStraightBack(float speed) {
  leftMotor.setDirectionAndSpeed(SportsMotor::BACKWARD, speed * SportsMotor::SPEED_MAX);
  rightMotor.setDirectionAndSpeed(SportsMotor::BACKWARD, speed * SportsMotor::SPEED_MAX);
}
/********* SERIAL CODEZ ***********/
/*
    Type the character and enter
   Space = STOP
   G = Max Forward
   B = Max Backward
   ] = Spin clockwise
   [ = spin counter clockwise
*/
unsigned char TestForKey(void) {
  unsigned char KeyEventOccurred;
  KeyEventOccurred = Serial.available();
  return KeyEventOccurred;
}

void RespToKey() {
  unsigned char theKey;
  theKey = Serial.read();

  Serial.write(theKey);
  Serial.print(", ASCII=");
  Serial.println(theKey, HEX);
  //state = STATE_DEBUG;
  switch (theKey) {
    case ' ':
      rightMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
      leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, 0);
      rightMotor.setEnabled(false);
      leftMotor.setEnabled(false);
      break;
    case 'g':
      leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, SportsMotor::SPEED_MAX);
      rightMotor.setDirectionAndSpeed(SportsMotor::FORWARD, SportsMotor::SPEED_MAX);
      break;
    case 'b':
      leftMotor.setDirectionAndSpeed(SportsMotor::BACKWARD, SportsMotor::SPEED_MAX);
      rightMotor.setDirectionAndSpeed(SportsMotor::BACKWARD, SportsMotor::SPEED_MAX);
      break;
    case ']':
      leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, SportsMotor::SPEED_MAX);
      rightMotor.setDirectionAndSpeed(SportsMotor::BACKWARD, SportsMotor::SPEED_MAX);
      //state = STATE_SPINNING_CW;
      break;
    case '[':
      leftMotor.setDirectionAndSpeed(SportsMotor::BACKWARD, SportsMotor::SPEED_MAX);
      rightMotor.setDirectionAndSpeed(SportsMotor::FORWARD, SportsMotor::SPEED_MAX);
      //state = STATE_SPINNING_CCW;
      break;
    case 'c':
      leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, SportsMotor::SPEED_MAX * .4);
      rightMotor.setDirectionAndSpeed(SportsMotor::BACKWARD, SportsMotor::SPEED_MAX * .4);
      state = STATE_LOCATING_CW;
      break;
    case 'd':
      state = STATE_DEBUG;
      leftMotor.setDirectionAndSpeed(SportsMotor::FORWARD, SportsMotor::SPEED_MAX * 0);
      rightMotor.setDirectionAndSpeed(SportsMotor::BACKWARD, SportsMotor::SPEED_MAX * 0);
      break;
    case '1':
      servo1.write(180);
      break;
    case '2':
      servo2.write(35);
      break;
    case '3':
      servo3.write(35);
      break;
    case '0':
      servo1.write(0);
      delay(200);
      servo2.write(170);
      delay(200);
      servo3.write(170);
      break;
    
  }
}
