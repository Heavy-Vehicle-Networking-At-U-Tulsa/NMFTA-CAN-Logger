#include <FlexCAN.h>
#include <kinetis_flexcan.h>

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

int getBaudRate() {
  
  int baudRate = 250000;
  FlexCAN testBus(baudRate);
  static CAN_message_t txmsg, rxmsg;
  unsigned long txCount, rxCount;
  unsigned long txTimer, rxTimer;
  testBus.begin();
  if(testBus.read(rxmsg)){
    return baudRate;
    confirmationBlink();
  }
  else{
    testBus.end();
  }
  baudRate = 500000;
  FlexCAN testBus1(baudRate);
  testBus1.begin();
  if(testBus1.read(rxmsg)){
    return baudRate;
    confirmationBlink();
  }
  else{
    testBus.end();
    fatalBlink(); //once Devil's CAN is added, this will have to be moved to after the failed portion of that test. 
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
int baudRate = getBaudRate();
FlexCAN J1939Bus(baudRate);
static CAN_message_t txmsg, rxmsg;
unsigned long txCount, rxCount;


// the setup routine runs once when you press reset:
void setup() {    
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);
  
  Serial.begin(115200);
  while(!Serial);
  
  J1939Bus.begin();
  rxmsg.timeout = 0;
  
  Serial.println(F("Teensy 3.2 FlexCAN AutoBaud Test."));
  Serial.println("Baudrate = ");
  Serial.print(baudRate);
}

// the loop routine runs over and over again forever:
void loop() {
   
   J1939Bus.read(rxmsg);
   uint32_t ID = rxmsg.id;
   byte DLC = rxmsg.len;
   Serial.println(ID);
   Serial.print("/t");
   Serial.print(DLC);
   Serial.print("/t");
   for( int i=0; i<8; ++i){
     Serial.print(rxmsg.buf[i]);
     Serial.print("/t");
   }


}
