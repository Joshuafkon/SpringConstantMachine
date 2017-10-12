
/* Spring Constant Calculator Machine --- Joshua Konstantinos 
 */

// include library for encoder
#include <Encoder.h>

void GoHome(); // declares function GoHome
void GoAboveSpring(); // declares function GoAboveSpring
void DetectSpring();	// declares function DetectSpring
void PreLoad(); // declares function PreLoad
void TakeMeasurement(); // declares function TakeMeasurement

// set pins (both with interupt capability) for the outputs of the encoder
Encoder myEnc(18, 19);

//include library for the load cell amplifier
#include "HX711.h"

//define the Data out (DOUT) and Clock (CLK) pins
#define DOUT 12
#define CLK 13

//Motor driver
#define motorpwm 9
#define motordir 8

// Set the calibration factor for the load cell it its current setup
HX711 scale(DOUT, CLK);
float calibration_factor = -215000; //appears to be well calibrated 

enum {
  kStateGoHome,
  kStateRetract,
  kStateDetectSpring,
  kStatePreLoad,
  kStateTakeMeasurement
};

#define NUM_MEASUREMENTS 5

struct {
  uint8_t current;
  float measurements[NUM_MEASUREMENTS];
  uint8_t currentMeasurement;
} state;

// include the library for the LCD display 
#include <LiquidCrystal.h>
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 6,
  en = 5,
  d4 = 4,
  d5 = 3,
  d6 = 2,
  d7 = 1;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Setup the pin for the start button
const int buttonPin = 7; // the number of the pushbutton pin
// variable for the state of the button (high or low)

int endstopstate = 0; //variable for the endstop
const int endstopPin = 11; // the end stop pin (HIGH when pressed)
int buttonPressed = 0;
int buttonPushCounter = 0; // counter for the number of button presses
int buttonState = 0; // current state of the button
int lastButtonState = 0; // previous state of the button

int measurementCounter = 0;

int pwm_value;
long encoderPosition = myEnc.read(); // reads the number of pules seen by the encoder. 6533 = 1 rev = 8mm

void setup() {

  state.current = kStateGoHome;

  // LCD SETUP 
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Spring Machine!");

  //load cell setup
  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  // set the start button as an input
  pinMode(buttonPin, INPUT);

  pinMode(endstopPin, INPUT); // sets the endstop pin as an INPUT

  // motor driver
  pinMode(motorpwm, OUTPUT);
  pinMode(motordir, OUTPUT);

  //sets inital value for motor speed to 0
  int pwm_value = 0;

  long encoderPosition = 0;
  long encoderSpringPosition = 0;
}

void loop() {

 //set the value of motorpwm to pwm_value
  analogWrite(motorpwm, pwm_value);
 // LOAD CELL loads calibration factor
 scale.set_scale(calibration_factor); //Adjust to this calibration factor
  
  uint8_t i;
  float avgMeasurement;
  switch (state.current) {
    case kStateGoHome:
      GoHome();
      state.current = kStateIdle;
      break;
    case kStateIdle:
      // wait for button press
      if (digitalRead(buttonPin) == HIGH) {
        GoAboveSpring();
		state.current = kStateDetectSpring;
        lcd.print("Beginning test");
      }
      break;
    case kStateRetract: // Retract partially (just above where we detected the spring the first time)
      if (/* we've gone far enough */) {
        state.current = kStateDetectSpring;
      }
      break;
    case kStateDetectSpring:
		pwm_value = 10; // motor speed - slow speed
        digitalWrite(motordir, LOW); // motor direction - down
	  if (scale.get_units() > 0) {
	  DetectSpring();
        state.current = kStatePreLoad;
      }
      break;
    case kStatePreLoad:
      // Move down a little bit (0.05'') and zero the load cell and the encoder position
      PreLoad();
        state.current = kStateTakeMeasurement;
      }
      break;
    case kStateTakeMeasurement:
  
          TakeMeasurement();
     
        } else {
          state.current = kStateRetract;
        }
      }
      break;
  }

void GoHome() {
  int pwm_value;
  long encoderPosition = myEnc.read(); // reads the number of pules seen by the encoder. 6533 = 1 rev = 8mm

  //STARTUP SEQUENCE - HOME THE MACHINE AND WAIT FOR BUTTON PUSH

  // Move the motor slowly up until the endStopState is triggered
  endstopstate = digitalRead(endstopPin);
  if (endstopstate != HIGH) {
    pwm_value = 200; //  power to motor.
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, HIGH); // motor direction = up  
  }
  while (endstopstate != HIGH) {
    endstopstate = digitalRead(endstopPin); // read the value 
  }

  // now we know that the motor is in the starting position
  encoderPosition = 0; //resets the position of the endstop to zero
  pwm_value = 0; // no power to motor.
  analogWrite(motorpwm, pwm_value);
}
void GoAboveSpring() {
  //STARTUP SEQUENCE - HOME THE MACHINE AND WAIT FOR BUTTON PUSH

  // Move the motor slowly up until the endStopState is triggered
  endstopstate = digitalRead(endstopPin);
  if (encoderPosition > -35000) {
    pwm_value = 100; //  power to motor.
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, LOW); // motor direction = down  
  }
  pwm_value = 0; // no power to motor.
  analogWrite(motorpwm, pwm_value);
}
void DetectSpring() {
 
 encoderPosition = 0; // zeros the encoder positon
}
void PreLoad() {
 
 while (encoder position > -1040) {
  pwm_value = 10; //  power to motor.
  analogWrite(motorpwm, pwm_value);
  digitalWrite(motordir, LOW); // motor direction = down  
 }
 encoderPosition = 0; // zeros the encoder positon
 scale.tare();          //Reset the scale to 0  // zeros the load cell
}
void TakeMeasurement(){
 if (encoderPosition > -6242 ) {
        state.measurements[state.currentMeasurement] = /* whatever */;
        state.currentMeasurement++;
        if (state.currentMeasurement == NUM_MEASUREMENTS) {
          // now stop, average measurements, & display result
          for (i = 0, avgMeasurement = 0.0f; i < NUM_MEASUREMENTS; i++) {
            avgMeasurement += state.measurements[i];
          }
          avgMeasurement /= (float) NUM_MEASUREMENTS;
          // TODO: display this value
          
          lcd.print("Reading: ");
          lcd.print(scale.get_units(), 1);
          lcd.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI 
          state.current = kStateGoHome;
		  }