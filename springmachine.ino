
/* Spring Constant Calculator Machine --- Joshua Konstantinos and Josh Ford
 */

// include library for encoder
#include <Encoder.h>

void GoHome();

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
float calibration_factor = -215000; //-7050 worked for my 440lb max scale setup

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
  int pwm_val = 0;

  long encoderPosition = 0;
  long encoderSpringPosition = 0;
}

void loop() {
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
      if (/* we've detected enough force */) {
        state.current = kStatePreLoad;
      }
      break;
    case kStatePreLoad:
      // Move down a little bit
      if (/* we've gone far enough */) {
        state.current = kStateTakeMeasurement;
      }
      break;
    case kStateTakeMeasurement:
      if (encoderPosition > -35000) //if the position of the encoder is above the top of the spring tube.
      {
        pwm_value = 75; // motor speed - medium speed
        digitalWrite(motordir, LOW); // motor direction - down
      } else {
        while (scale.get_units() <= 0) // Hopefully this line works - while the load cell sees no force  
        {
          pwm_value = 15; // move slowly
          digitalWrite(motordir, LOW); // move down
        }
  
        encoderPosition = 0; // zeros the encoder positon
  
        // While the current state of the encoder is less that 0.05'' down from when force was detected
        if (encoderPosition == -1040) {
          scale.tare(); //Reset the scale to 0  // zeros the load cell
          // move motor down until encoder position is 6242 pulses further down.
          pwm_value = 15; // move slowly
          digitalWrite(motordir, LOW); // move down
        }
      }
      if (/* we've gone far enough */) {
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
        } else {
          state.current = kStateRetract;
        }
      }
      break;
  }

  // Second Loop - measurement loop. loops until process is repeated 5 times.
  // Quickly drives the motor down to just above spring tube. Then moves down slowly until the spring
  // is dected by the load cell. Moves down 0.05'' zeros the force at this point. Moves down .35'' calculates
  // spring constant force detected/0.35'' = spring constant measurement1. Moves back up to just above spring
  // tube and repeats 4 more times. Averages spring constant measurements and displays on LCD. 


  digitalWrite(motordir, LOW); // motor direction = down

  //START BUTTON SECTION OF CODE

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    lcd.setCursor(0, 1);
    lcd.print(encoderPosition);
  }

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):

  //LCD SECTION OF CODE
  lcd.setCursor(0, 1);

  // LOAD CELL SECTION OF CODE
  scale.set_scale(calibration_factor); //Adjust to this calibration factor

  // If the start button is not being pushed - display the live readings from the loadcell
  if (buttonState == LOW) {
  }

  //set the value of motorpwm to pwm_value
  analogWrite(motorpwm, pwm_value);
}


void GoHome() {
  int pwm_value;
  long encoderPosition = myEnc.read(); // reads the number of pules seen by the encoder. 6533 = 1 rev = 8mm

  //STARTUP SEQUENCE - HOME THE MACHINE AND WAIT FOR BUTTON PUSH

  // Move the motor slowly up until the endStopState is triggered
  endstopstate = digitalRead(endstopPin);
  if (endstopstate != HIGH) {
    pwm_value = 200; // low power to motor.
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

