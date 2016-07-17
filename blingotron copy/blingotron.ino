#include <Wire.h>
#include <VarSpeedServo.h>
#include <Arduino.h>

//ENUM
typedef enum {FULLY_FOLDED, TOP_OPEN, FRONT_OPEN, SCREEN_OPEN, SCREEN_CLOSED, FRONT_CLOSED, TOP_CLOSED, FULLY_DEPLOYED, FINISHED} displayState;
typedef enum {TOGGLE_ON, TOGGLE_OFF} toggleSwitchState;
// CONSTANTS AND PINS
const int top_lid_open      = 110;
const int top_lid_closed    = 0;
const int front_lid_open    = 60;
const int front_lid_closed  = 130;
const int servo_speed       = 50;
const int azimuth_pin       = A0;
const int led_pin           = 13;      // the number of the LED pin
const int switch_pin        = 4;
const int motor_right_pin   = 5;
const int motor_left_pin    = 6;
const long blink_interval   = 5000;           // interval at which to blink (milliseconds)

// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
const int numReadings = 5;

// GLOBAL VARIABLES
boolean didToggleSwitch = false;
int azimuth = 0;                // the azimuth
boolean is_monitor_open;
VarSpeedServo top_lid_servo;
VarSpeedServo front_lid_servo;
displayState state = FULLY_FOLDED;
toggleSwitchState toggle_state = TOGGLE_OFF;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total

// Variables will change :
int led_state = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previous_millis = 0;        // will store last time LED was updated

// constants won't change :

void setup() {
  // set the digital pin as output:
  pinMode(led_pin, OUTPUT);
  pinMode(switch_pin, INPUT_PULLUP);
  // initialize serial communication with computer:
  Serial.begin(9600);
  // initialize all the readings to 0:
  top_lid_servo.attach(8);  // attaches teh front servo at pin 8
  front_lid_servo.attach(9);
  analogWrite(motor_left_pin, 0);
  analogWrite(motor_right_pin, 0);
//  println("Finished setup");
}

void loop() {

  DirectionFinder();

  switch(state) {

    case FULLY_FOLDED: {
      FullyFoldedAction();
    }
    break;

    case TOP_OPEN: {
      TopOpenAction();
    }
    break;

    case FRONT_OPEN: {
      FrontOpenAction();
    }
    break;

    case SCREEN_OPEN: {
      ScreenOpenAction();
    }
    break;

    case SCREEN_CLOSED: {
      ScreenClosedAction();
    }
    break;

    case FRONT_CLOSED: {
      FrontClosedAction();
    }
    break;

    case FULLY_DEPLOYED: {
      FullyDeployedAction();
    }
    break;

    case TOP_CLOSED: {
      TopClosedAction();
    }
    break;

    case FINISHED: {
      FinishedAction();
    }
    break;
  }

  if (digitalRead(switch_pin) == 0 && toggle_state == TOGGLE_OFF)
  {
    Serial.println("Toggled Switch! Starting Open Sequence");
    state = TOP_OPEN;
    toggle_state = TOGGLE_ON;
  }
  if (digitalRead(switch_pin) == 1 && toggle_state == TOGGLE_ON) {
    Serial.println("Toggled Switch! Starting close sequence");
    state = FRONT_OPEN;
    toggle_state = TOGGLE_OFF;
  }
}

/* ****
* FullyFoldedAction() 
* Default State when program begins.
*
*
*
*
*
* ****/
void FullyFoldedAction() {

  Serial.println("IN FULLY FOLDED!");
  front_lid_servo.write(front_lid_closed);
  top_lid_servo.write(top_lid_closed);
  analogWrite(motor_left_pin, 0);
  analogWrite(motor_right_pin, 0);
}

/* ****
* 1. TopOpenAction()
* When user engages the toggle switch ON, this state begins the open sequence, and opens the top lid.
* Is not used in closing sequence.
*
*
*
*
* ****/

void TopOpenAction() {
  Serial.println("In Top Action!");
  top_lid_servo.write(top_lid_open, servo_speed, true);
  // delay(2000);
  // front_lid_servo.write(front_lid_open, servo_speed);

  if (toggle_state == TOGGLE_ON) {
    Serial.println("In Toggle State *ON* in Top Action");
    state = FRONT_OPEN;
  }
  else {
      Serial.println("In Toggle State *OFF* in Top Action");

    // state = SCREEN_CLOSED;
  }

}



/* ****
* 2. FrontOpenAction()
* After Top lid opens, this state opens the front lid.
* This function is used in both open and closing sequences.
*
*
*
*
* ****/

void FrontOpenAction() {

  Serial.println("In Front Action!");
  front_lid_servo.write(front_lid_open, servo_speed, true);
  // delay(2000);
  if (toggle_state == TOGGLE_ON) {
    state = SCREEN_OPEN;
  }
  else {
    state = SCREEN_CLOSED;
  }
}

/* ****
* 3. ScreenOpenAction()
* After the front lid opens, this state opens the screen.
* This function is not used in the closing sequence.
*
*
*
*
* ****/
void ScreenOpenAction() {

  if ((azimuth < 1000) && (azimuth > 980)) {
    Serial.println("IN RANGE: < 585 > 615 OF AZIMUTH");
    analogWrite(motor_left_pin, 0);
    // delay(2000);
    state = FRONT_CLOSED;
  }
  else {
    analogWrite(motor_left_pin, 200);
  }
}

/* ****
* 6. ScreenClosesAction()
* After the front lid opens, this function closes the Screen. 
* This function is not used in the opening sequence.
*
*
*
*
* ****/
void ScreenClosedAction(){

  if ((azimuth < 680) && (azimuth > 650)) {
    Serial.println("IN RANGE: < 215 > 185 OF AZIMUTH");
    analogWrite(motor_right_pin, 0);
    state = FRONT_CLOSED;

  }
  else {
    analogWrite(motor_right_pin, 100);
  }
}

/* ****
* 4./7. FrontClosesAction()
* After the screen opens, this function closes the front.
* This function is used in both open and closing sequences.
*
*
*
*
* ****/
void FrontClosedAction() {
  Serial.println("In Front Closed Action!");
  front_lid_servo.write(front_lid_closed, servo_speed, true);
  if (toggle_state == TOGGLE_ON) {
    state = FULLY_DEPLOYED;
  }
  else {
    state = TOP_CLOSED;
  }
}

/* ****
* 8. TopOpenAction()
* When user engages the toggle switch OFF, this state begins the open sequence, and closes the top lid.
* Is not used in closing sequence.
*
*
*
*
* ****/

void TopClosedAction() {
  Serial.println("In Top Close!");
  top_lid_servo.write(top_lid_closed, servo_speed, true);
  state = FULLY_FOLDED;
}

void FullyDeployedAction() {
  analogWrite(motor_left_pin, 0);
  analogWrite(motor_right_pin, 0);
}

void FinishedAction() {
  Serial.println("FINISHED");
  top_lid_servo.write(top_lid_closed, servo_speed, true);
  front_lid_servo.write(front_lid_closed, servo_speed, true);
  analogWrite(motor_left_pin, 0);
  analogWrite(motor_right_pin, 0);
}

void DirectionFinder() {
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(azimuth_pin);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }
  // calculate the azimuth:
  azimuth = total / numReadings;
  // send it to the computer as ASCII digits
  // Serial.print("A: ");

  // Serial.println(azimuth);
}

