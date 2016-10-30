#include <FlexCAN.h>
#include <kinetis_flexcan.h>


static CAN_message_t txmsg, rxmsg;
unsigned long txCount, rxCount;
unsigned long txTimer, rxTimer;

// Declare pin numbers based on the schematic
#define greenLED  23
#define redLED    22
#define CANconnect 2
#define CAN1termination 16
#define CAN2termination 21

  
void confirmationBlink(){
  const int greenLed = 23;
  int ledState = LOW;
  uint32_t previousMillis = 0;
  const long blinkOn = 500;
  const long blinkOff = 500;
  uint32_t currentMillis = millis();
  digitalWrite(greenLed, HIGH);
}

void fatalBlink(){
  const int redLed = 23;
  digitalWrite(redLed, HIGH);
  delay(500);
  digitalWrite(redLed, LOW);
  delay(250);
  digitalWrite(redLed, HIGH);
  delay(500);
  digitalWrite(redLed, LOW);
  delay(250);
  digitalWrite(redLed, HIGH);
  delay(500);
  digitalWrite(redLed, LOW);
  delay(250);
  digitalWrite(redLed, HIGH);
}

uint32_t getBaudRate() {
  Serial.println("Setting Baud to 250000");
  uint32_t baudRate = 250000;
  FlexCAN testBus(baudRate);
  
  Serial.println("Beginning test at 250000");
  testBus.begin();
  rxmsg.timeout = 1000;
  Serial.println("Reading...");
  if(testBus.read(rxmsg)){
    confirmationBlink();
    Serial.print("Baudrate is ");
    Serial.println(baudRate);
    return baudRate;
    
  }
  else{
    Serial.println("Baudrate is NOT 250000");
    
    testBus.end();
  }
  Serial.println("Setting Baud to 500000");
  baudRate = 500000;
  FlexCAN testBus1(baudRate);
  static CAN_message_t txmsg1, rxmsg1;
  delay(10);
  Serial.println("Beginning test at 500000");
  testBus1.begin();
  rxmsg1.timeout = 1000;
  if(testBus1.read(rxmsg1)){
    confirmationBlink();
    Serial.print("Baudrate is ");
    Serial.println(baudRate);
    return baudRate;
    
  }
  else{
    testBus1.end();
    return 250000;
    Serial.println("Autobaud unsuccessful, default rate set to 250000");
    
    //fatalBlink(); //once Devil's CAN is added, this will have to be moved to after the failed portion of that test. 
  }
  //Devil CAN was not written into my FlexCAN library so this would not compile on my device
  /*baudRate = 666000;
  FlexCAN testBus(baudRate);
  static CAN_message_t txmsg, rxmsg;
  unsigned long txCount, rxCount;
  unsigned long txTimer, rxTimer;
  testBus.begin();
  FlexCAN testBus(baudRate);
  static CAN_message_t txmsg, rxmsg;
  unsigned long txCount, rxCount;
  unsigned long txTimer, rxTimer;
  testBus.begin();
  if(testBus.read(rxmsg)){
    return baudRate;
  }
  else{
    testBus.end(); 
  }  
    */
}
const int greenLed = 23;
const int redLed = 22;

uint32_t baudRate = 0;
FlexCAN J1939Bus(baudRate);

//Keep track of the current state of the LEDs
boolean greenLEDstate = false;
boolean redLEDstate = true;
boolean LEDstate = true;

//Declare a millisecond timer to execute the switching of the LEDs on a set time
elapsedMillis toggleTimer;
elapsedMicros microsecondsPerSecond;

#define LEDtoggleTime 500

//set up a display buffer
char displayBuffer[100]; 


void setup() {
  //Set up the pin functions
  pinMode(greenLED,OUTPUT);
  pinMode(redLED,OUTPUT);
  pinMode(CANconnect,OUTPUT);
  pinMode(CAN1termination,OUTPUT);
  pinMode(CAN2termination,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);

  digitalWrite(greenLED,greenLEDstate);
  digitalWrite(redLED,redLEDstate);
  digitalWrite(CANconnect,LOW);
  digitalWrite(CAN1termination,LOW);
  digitalWrite(CAN2termination,LOW);
  digitalWrite(LED_BUILTIN,LEDstate);
   
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);
  
  Serial.begin(115200);
  while(!Serial);
  
  Serial.println("Getting baudrate");
  baudRate = getBaudRate();
  //static FlexCAN J1939Bus(baudRate);
  FlexCAN J1939Bus(baudRate);
  J1939Bus.begin();
  rxmsg.timeout = 0;
  
  
  
  Serial.println("Teensy 3.2 FlexCAN AutoBaud Test.");
  Serial.print("Baudrate = ");
  Serial.println(baudRate);
 
 while(1){
     if(J1939Bus.read(rxmsg)){
       uint32_t ID = rxmsg.id;
       byte DLC = rxmsg.len;
       Serial.print(ID);
       Serial.print("\t");
       Serial.print(DLC);
       Serial.print("\t");
       for( int i=0; i<8; ++i){
         Serial.print(rxmsg.buf[i]);
         Serial.print("\t");
       }
       Serial.println(baudRate);
       
   }

  }
  
}

// the loop routine runs over and over again forever:
void loop() {
   
  
}
