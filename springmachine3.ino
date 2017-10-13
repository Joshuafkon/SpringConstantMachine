/* Spring Constant Calculator Machine --- Joshua Konstantinos
*/

// include library for encoder
#include <Encoder.h>

void GoHome(); // declares function GoHome
void GoAboveSpring(); // declares function GoAboveSpring
void PreLoad(); // declares function PreLoad
void TakeMeasurement(); // declares function TakeMeasurement
void retract();//declares function retract

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
  kStateTakeMeasurement,
  kStateIdle,
  kGoAboveSpring,
};

#define NUM_MEASUREMENTS 5

struct {
  long encoderZeroPosition;
  long encoderSpringPosition;
  long encoderTarePosition;
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
      lcd.setCursor(0, 1);
      lcd.print(myEnc.read());
      lcd.setCursor(0, 0);
      lcd.print ("GO HOME");
      GoHome();
      state.current = kStateIdle;
      break;
    case kStateIdle:
      myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm
      lcd.setCursor(0, 1);
      lcd.print(myEnc.read());
      lcd.setCursor(0, 0);
      lcd.print ("IDLE       ");
      // wait for button press
      if (digitalRead(buttonPin) == HIGH) {
        GoAboveSpring();
        state.current = kStateDetectSpring;
      }
      break;
    case kGoAboveSpring: // move quickly to just above spring
      lcd.setCursor(0, 1);
      lcd.print(myEnc.read());
      lcd.setCursor(0, 0);
      lcd.print ("ABOVE SPRING");
      GoAboveSpring();
      state.current = kStateDetectSpring;
      break;
    case kStateDetectSpring:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(scale.get_units());
      lcd.setCursor(0, 1);
      lcd.print(myEnc.read());
      lcd.setCursor(0, 0);
      lcd.print ("DETECT     ");
      pwm_value = 25; // motor speed - slow speed
      digitalWrite(motordir, LOW); // motor direction - down
      if (scale.get_units() > 0) {
        state.encoderSpringPosition = myEnc.read(); // reads the number of pules seen by the encoder. 6533 = 1 rev = 8mm
        state.current = kStatePreLoad;
      }
      break;
    case kStatePreLoad:
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(myEnc.read());
      lcd.setCursor(0, 0);
      lcd.print ("PRELOAD     ");
      // Move down a little bit (0.05'') and zero the load cell and the encoder position
      PreLoad();
      state.current = kStateTakeMeasurement;
      break;
    case kStateTakeMeasurement:
      lcd.setCursor(0, 1);
      lcd.print(myEnc.read());
      lcd.setCursor(0, 0);
      lcd.print ("MEASUREMENT   ");
      TakeMeasurement();
      break;
    case kStateRetract: // Retract partially (just above where we detected the spring the first time)
      lcd.setCursor(0, 1);
      lcd.print(myEnc.read());
      lcd.setCursor(0, 0);
      lcd.print("Reteact");
      retract();
      delay(3000);
      state.current = kStateDetectSpring;
      break;
  }
}

void GoHome() {
  int pwm_value;

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

  myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm

  // now we know that the motor is in the starting position
  pwm_value = 0; // no power to motor.
  analogWrite(motorpwm, pwm_value);
}

void GoAboveSpring() {
  if (myEnc.read() > -25000) {
    pwm_value = 200; //  power to motor.
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, LOW); // motor direction = down
  }
  // just keep looping and doing nothing until the position is correct
  while (myEnc.read() > -25000) {
    lcd.setCursor(0, 1);
    lcd.print(myEnc.read());
    continue;
  }
  // turn off the motor now that we've reached the correct point
  pwm_value = 0; // no power to motor.
  analogWrite(motorpwm, pwm_value);
}




void PreLoad() {
  myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm
  while (myEnc.read() > -1040) {
    pwm_value = 25; //  power to motor.
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, LOW); // motor direction = down

    lcd.setCursor(0, 1);
    lcd.print(scale.get_units());
    lcd.setCursor(15, 0);
    lcd.print(myEnc.read() - state.encoderSpringPosition);
  }
  myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm
  scale.tare();          //Reset the scale to 0  // zeros the load cell



}

void TakeMeasurement() {
  uint8_t i;
  float avgMeasurement;
  myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm
  while ( myEnc.read() > -6242 ) {
    pwm_value = 25; //  power to motor.
    digitalWrite(motordir, LOW); // motor direction = down
    analogWrite(motorpwm, pwm_value);
  }
  //actually calculate the spring constant
  //increments so that it repeats five times
  state.measurements[state.currentMeasurement] = scale.get_units() / .3;
  state.currentMeasurement++;

  // After five cycles it displays the measurements
  if (state.currentMeasurement == NUM_MEASUREMENTS) {
    // now stop, average measurements, & display result
    for (i = 0, avgMeasurement = 0.0f; i < NUM_MEASUREMENTS; i++) {
      avgMeasurement += state.measurements[i];
    }
    avgMeasurement /= (float) NUM_MEASUREMENTS;
    // TODO: display this value

    lcd.print("Constant: ");
    lcd.print(avgMeasurement);
    lcd.print(" lbs/in"); // units for spring constant
    state.current = kStateGoHome;
  } else {
    state.current = kStateRetract;
  }
}

void retract() {
  while ( myEnc.read() > -10000) {
    pwm_value = 25; //  power to motor.
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, HIGH); // motor direction = up

  }

  pwm_value = 0;
  analogWrite(motorpwm, pwm_value);

}

