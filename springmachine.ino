/* Spring Constant Calculator Machine --- Joshua Konstantinos and Josh Ford
 *  
 *  

*/

// include library for encoder
#include <Encoder.h>

// set pins (both with interupt capability) for the outputs of the encoder
Encoder myEnc(18, 19);

//include library for the load cell amplifier
#include "HX711.h"

//define the Data out (DOUT) and Clock (CLK) pins
#define DOUT  12
#define CLK  13

//Motor driver
#define motorpwm      9
#define motordir      8

// Set the calibration factor for the load cell it its current setup
HX711 scale(DOUT, CLK);
float calibration_factor = -215000; //-7050 worked for my 440lb max scale setup



// include the library for the LCD display 
#include <LiquidCrystal.h>
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 6, en = 5, d4 = 4, d5 = 3, d6 = 2, d7 = 1;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


//Setup the pin for the start button
const int buttonPin = 7;     // the number of the pushbutton pin
// variable for the state of the button (high or low)


int endstopstate = 0; //variable for the endstop
const int endstopPin = 11; // the end stop pin (HIGH when pressed)
int buttonPressed = 0;
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

void setup() {



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
 pinMode(motorpwm,OUTPUT);
 pinMode(motordir,OUTPUT);

//sets inital value for motor speed to 0
 int pwm_val = 0;

 long encoderPosition = 0;

}



void loop() {


//motor driver


int pwm_value;

long encoderPosition = myEnc.read();  // reads the number of pules seen by the encoder. 6533 = 1 rev = 8mm

endstopstate = digitalRead(endstopPin);

//STARTUP SEQUENCE - HOME THE MACHINE AND WAIT FOR BUTTON PUSH

//determine if the start button has been pressed or not. If it has not been pressed 
//then we are in the Startup Sequence 

// read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) 
    {
    // if the state has changed, increment the counter
      if (buttonState == HIGH) 
        {
      // if the current state is HIGH then the button went from off to on:
         buttonPushCounter++;
        }
    }
      

while (buttonPushCounter = 0)
{
  if (endstopstate == HIGH)  // if the endstop has been triggered
    {
      encoderPosition = 0;   //resets the position of the endstop to zero
      pwm_value = 0;        // no power to motor.
      digitalWrite(motordir, LOW); // motor direction = down
    }
  else
    {
      pwm_value = 25;
      digitalWrite(motordir, HIGH);
    }
}

while (buttonPushCounter = 1)
{
  if (encoderPosition > -35000)
    {
      pwm_value = 75;
      digitalWrite(motordir, LOW);
    }
  else
    {
      pwm_value = 0;
      digitalWrite(motordir, HIGH); 
    }
}
 //START BUTTON SECTION OF CODE
 
 // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) 
  {

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
if (buttonState == LOW) 
  {
  lcd.print("Reading: ");
  lcd.print(scale.get_units(), 1);
  lcd.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI 
  }

//set the value of motorpwm to pwm_value
  analogWrite(motorpwm,pwm_value);
 

    }
 

  


