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

int measurementCounter = 0;

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
 long encoderSpringPosition = 0;

}



void loop() {


//motor driver


int pwm_value;

long encoderPosition = myEnc.read();  // reads the number of pules seen by the encoder. 6533 = 1 rev = 8mm

endstopstate = digitalRead(endstopPin);

//STARTUP SEQUENCE - HOME THE MACHINE AND WAIT FOR BUTTON PUSH


// First Loop. Loops until start button is pressed. Moves motor up until endstop is pressed.
while (true)
{
  buttonState = digitalRead(buttonPin); // read the pushbutton input pin:
  if (buttonState == HIGH) 
    {
    break;
    }
      
  if (endstopstate == HIGH)  // if the endstop has been triggered
    {
      encoderPosition = 0;   //resets the position of the endstop to zero
      pwm_value = 0;        // no power to motor.
      digitalWrite(motordir, LOW); // motor direction = down
      break;
    }
    else
      {
       pwm_value = 25;        // low power to motor.
       digitalWrite(motordir, HIGH); // motor direction = up  
      }
      }
}
