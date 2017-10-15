/* Spring Constant Calculator Machine --- Joshua Konstantinos
*/

// include library for encoder
#include <Encoder.h>

void GoHome(); // declares function GoHome
void GoAboveSpring(); // declares function GoAboveSpring
void PreLoad(); // declares function PreLoad
void TakeMeasurement(); // declares function TakeMeasurement
void retract();//declares function retract
void detect();//declares function detect
void SetSpring();//declares function SetSpring


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
  kStateSetSpring,
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



float repeatability;
int setSpringCount = 0;

int pwm_value;
long encoderPosition = myEnc.read(); // reads the number of pules seen by the encoder. 6533 = 1 rev = 8mm



void setup() {

  //apparently there are only two global variable to clear. Neither of which should effect the encoder reading....
  int setSpringCount = 0;
  int endstopstate = 0; //variable for the endstop

  state.current = kStateGoHome;
  state.currentMeasurement = 0;

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
  float repeatability;

  switch (state.current) {

    case kStateGoHome:

      setup();
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
      scale.tare();           //Reset the scale to 0  // zeros the load cell
      myEnc.write(0);         // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm
      lcd.clear();
      lcd.setCursor(8, 0);
      lcd.print ("DETECT     ");
      detect();
      break;

    case kStateSetSpring:
      myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm
      lcd.clear();
      lcd.setCursor(8, 0);
      lcd.print ("SET SPRING     ");
      // Move down a little bit (0.05'') and zero the load cell and the encoder position
      SetSpring();
      myEnc.write(0);
      state.current = kStateRetract;
      break;

    case kStatePreLoad:
      myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm
      lcd.clear();
      lcd.setCursor(8, 0);
      lcd.print ("PRELOAD     ");
      // Move down a little bit (0.05'') and zero the load cell and the encoder position
      PreLoad();
      state.current = kStateTakeMeasurement;
      break;

    case kStateTakeMeasurement:
      myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm
      lcd.setCursor(0, 1);
      lcd.print(myEnc.read());
      lcd.setCursor(8, 0);
      lcd.print ("MEASUREMENT   ");
      TakeMeasurement();
      myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm

      break;
    case kStateRetract: // Retract partially (just above where we detected the spring the first time)
      lcd.setCursor(0, 1);
      lcd.print(myEnc.read());
      lcd.setCursor(8, 0);
      lcd.print("Retract");
      myEnc.write(0);
      retract();
      delay(1000);
      state.current = kStateDetectSpring;
      break;
  }
}


//STARTUP SEQUENCE - HOME THE MACHINE AND WAIT FOR BUTTON PUSH
void GoHome() {

  int pwm_value;

  // If the endstop is NOT HIGH, move the motor up at speed 200
  endstopstate = digitalRead(endstopPin);
  if (endstopstate != HIGH) {
    pwm_value = 200; //  power to motor.
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, HIGH); // motor direction = up
  }

  // Continue to read the value of the enstop switch
  while (endstopstate != HIGH) {
    endstopstate = digitalRead(endstopPin); // read the value
  }

  // now we know that the motor is in the starting position - Turn off power to the motor and zero the encoder position.
  myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm
  scale.tare();
  pwm_value = 0; // no power to motor.
  analogWrite(motorpwm, pwm_value);
}


//MOVES THE MOTOR QUICKLY TO JUST ABOVE WHERE THE LONGEST SPRING COULD BE
void GoAboveSpring() {


  if (myEnc.read() > -35000) {   // If the encoder position hasn't moved down to the position just above the spring
    pwm_value = 200; //  power to motor.
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, LOW); // motor direction = down
  }

  // just keep looping and doing nothing until the position is correct
  while (myEnc.read() > -35000) {
    lcd.setCursor(0, 1);
    lcd.print(myEnc.read());
    continue;  // continues to check the position of the encoder
  }


  //If we've reached this point in the code we turn off the motor now that we've reached the correct point
  pwm_value = 0; // no power to motor.
  analogWrite(motorpwm, pwm_value);
}


//MOVES DOWN SLOWLY UNTIL THE LOADCELL DETECTS A FORCE (WE'VE REACHED THE SPRING)
void detect() {

  //Sets the speed and direction of the motor to slow and down.
  pwm_value = 25; // motor speed - slow speed
  analogWrite(motorpwm, pwm_value);
  digitalWrite(motordir, LOW); // motor direction - down

  //display loadcell reading
  lcd.setCursor(0, 0);
  lcd.print(scale.get_units());

  //display encoder reading
  lcd.setCursor(0, 1);
  lcd.print(myEnc.read());


  //If the Load cell senses more than 0.01 lbs-force - turn off the motor.
  if (scale.get_units() > 0.01) {
    pwm_value = 0; // motor speed - slow speed
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, LOW); // motor direction - down

    //If the spring set function hasn't been run before for this spring, set the state to "kStateSetSpring" if it has been run before
    //go to the standard kStatePreLoad. *The first measurement for each spring seems to be a little more innacurate. so I'm setting spring with
    //more force and then retracting.
    if (setSpringCount == 0) {
      state.current = kStateSetSpring;
    }
    else
    {
      state.current = kStatePreLoad;
    }
  }
}


//MOVES DOWN TO "SET" THE SPRING,BUT DOES NOT CONTINUE TO TAKE MEASUREMENT. FIRST MEASUREMENTS SEEMED INACCURATE.
void SetSpring() {

  //Increments a variable so that I can tell if this function has been called before for this spring.
  setSpringCount = 1;

  //Same as the preLoad function, but moves down further.
  while (myEnc.read() > -6000 && scale.get_units() < 8) {
    pwm_value = 25; //  power to motor.
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, LOW); // motor direction = down

    // display loadcell reading
    lcd.setCursor(0, 0);
    lcd.print(scale.get_units());
  }


  // turn off motor when preload is finished
  pwm_value = 0; //  power to motor.
  analogWrite(motorpwm, pwm_value);
  digitalWrite(motordir, LOW); // motor direction = down


}


//TO AVOID ANY INACCURANCES ABOUT THE START OF THE SPRING DUE TO LOADCELL RESOLUTION, THIS FUNCTION MOVES DOWN 0.05'' AND ZEROS THE
//LOADCELL AND ENCODER READING. THE MEASUREMENT CAN NO BE TAKEN FROM THIS POINT.
void PreLoad() {

  //Moves the motor down slightly (0.05'')
  while (myEnc.read() > -1040 && scale.get_units() < 8) {
    pwm_value = 25; //  power to motor.
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, LOW); // motor direction = down

    // display loadcell reading
    lcd.setCursor(0, 0);
    lcd.print(scale.get_units());
  }

  // turn off motor when preload is finished
  pwm_value = 0; //  power to motor.
  analogWrite(motorpwm, pwm_value);
  digitalWrite(motordir, LOW); // motor direction = down

  //Zeros the encoder and the loadcell
  myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm
  scale.tare();          //Reset the scale to 0  // zeros the load cell
  delay(300);
}


//THE IMPORTANT FUNCTION THAT CALUCLATIONS AND PRINTS THE SPRING CONSTANT!
void TakeMeasurement() {

  myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm

  //variables for the measurement
  uint8_t i;    //used to increment the number of measurements taken
  float avgMeasurement;  // used to store the average of the five measurements


  //while the econder hasn't moved down 0.35'' AND (as a saftey measure) the loadcell does not see more than 8 lbs
  while ( myEnc.read() > -7282  && scale.get_units() < 8 ) {

    //display the reading of the loadcell
    lcd.setCursor(0, 0);
    lcd.print(scale.get_units());

    //Set the motor to a low power
    pwm_value = 20; //  power to motor.
    digitalWrite(motordir, LOW); // motor direction = down
    analogWrite(motorpwm, pwm_value);
  }

  // turns off the motor after it's moved down that far
  pwm_value = 0; //  power to motor.

  digitalWrite(motordir, LOW); // motor direction = down
  analogWrite(motorpwm, pwm_value);

  //CALCULATES AND STORES SPRING CONSTANT
  state.measurements[state.currentMeasurement] = scale.get_units() / .349980;

  //prints single spring constant
  lcd.setCursor(0, 0);
  lcd.print("Spring Constant: ");
  lcd.setCursor(0, 1);
  lcd.print(state.measurements[state.currentMeasurement]);
  lcd.setCursor(9, 1);
  lcd.print(" lbs/in"); // units for spring constant

  //pauses for 3 seconds so you can read the value of the spring constant
  delay(3000);


  //increments so that it repeats five times
  state.currentMeasurement++;



  // After five cycles it displays the average measurement
  if (state.currentMeasurement == NUM_MEASUREMENTS) {




    // now stop, average measurements, & display result
    for (i = 0, avgMeasurement = 0.0f; i < NUM_MEASUREMENTS; i++) {
      avgMeasurement += state.measurements[i];
    }

    avgMeasurement /= (float) NUM_MEASUREMENTS;


    //Stop motor after final calc - rather than continuing to run and crashing!
    pwm_value = 0; //  power to motor.
    digitalWrite(motordir, HIGH); // motor direction = up


    //Print Spring Calculation value
    lcd.setCursor(0, 0);
    lcd.print("Ave Spring Constant: ");
    lcd.setCursor(0, 1);
    lcd.print(avgMeasurement);
    lcd.setCursor(9, 1);
    lcd.print(" lbs/in"); // units for spring constant

    //delay long enough to read the value
    delay(10000);

    //calculate repeatability

    repeatability = sqrt((sq(state.measurements[0] - avgMeasurement) + sq(state.measurements[1] - avgMeasurement) + sq(state.measurements[2] - avgMeasurement)  + sq(state.measurements[3] - avgMeasurement)  + sq(state.measurements[4] - avgMeasurement)) / 5);

    //Print Repeatability
    lcd.setCursor(0, 0);
    lcd.print("Repeatability: ");
    lcd.setCursor(0, 1);
    lcd.print(repeatability);
    lcd.setCursor(9, 1);
    lcd.print(" lbs/in"); // units for spring constant

    //delay long enough to read the value
    delay(10000);

    state.current = kStateGoHome;
  } else {
    state.current = kStateRetract;
  }
}


// RETRACTS THE MOTOR TO JUST ABOVE WHERE THE SPRING COULD BE TO START TAKING ANOTHER MEASURMENT
void retract() {

  //clears the value of the encoder - for saftey
  myEnc.write(0); // zeros the number of pules seen by the encoder. 6533 = 1 rev = 8mm

  //prints the encoder value for testing
  lcd.setCursor(0, 1);
  lcd.print(myEnc.read());

  //Check the value of the endstop switch
  endstopstate = digitalRead(endstopPin); // read the value

  //While the the motor is below where we want it AND the endstop has not been triggered (as a saftey measure) move up quickly.
  while ( myEnc.read() < 15000 && endstopstate != HIGH) {
    pwm_value = 150; //  power to motor.
    analogWrite(motorpwm, pwm_value);
    digitalWrite(motordir, HIGH); // motor direction = up
    endstopstate = digitalRead(endstopPin);
  }

  // Turn off the motor
  pwm_value = 0;
  analogWrite(motorpwm, pwm_value);

}

