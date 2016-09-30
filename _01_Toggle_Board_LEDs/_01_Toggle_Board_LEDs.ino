/*
 * NMFTA CAN Logger Project
 * 
 * Arduino Sketch to test the LED indicators.
 * 
 * Written By Dr. Jeremy S. Daily
 * The University of Tulsa
 * Department of Mechanical Engineering
 * 
 * 29 September 2016
 * 
 */


// Declare pin numbers for the LED based on the schematic
#define greenLED  23
#define redLED    22

//Keep track of the current state of the LEDs
boolean greenLEDstate = false;
boolean redLEDstate = true;

//Declare a millisecond timer to execute the switching of the LEDs on a set time
elapsedMillis toggleTimer;

#define LEDtoggleTime 500

void setup() {
  // put your setup code here, to run once:
  pinMode(greenLED,OUTPUT);
  pinMode(redLED,OUTPUT);

  digitalWrite(greenLED,greenLEDstate);
  digitalWrite(redLED,redLEDstate);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (toggleTimer >= LEDtoggleTime){
    toggleTimer = 0;
    greenLEDstate = !greenLEDstate;
    redLEDstate = !redLEDstate;
    digitalWrite(greenLED,greenLEDstate);
    digitalWrite(redLED,redLEDstate);
  }
}
