/*
 * NMFTA CAN Logger Project
 * 
 * Arduino Sketch to test the ability to receive CAN messages
 * 
 * Written By Dr. Jeremy S. Daily
 * The University of Tulsa
 * Department of Mechanical Engineering
 * 
 * 29 September 2016
 * 
 * Released under the MIT License
 *
 * Copyright (c) 2016        Jeremy S. Daily
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//Include the CAN libraries for the Teensy microprocessor
#include <FlexCAN.h>

//Include the ability to keep track of realtime
#include <TimeLib.h>

#include <mcp_can.h>
#include <SPI.h>

//initiate the CAN library at 250kbps for the CAN 1 or J1939
FlexCAN CANbus(250000);
MCP_CAN CAN0(20);

//Set up the CAN data structures
static CAN_message_t rxmsg,txmsg;
uint32_t rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char vinRequestMessage[8] = {0xEB,0xFE,0x00};
char request[8];
unsigned char vin1[8];
unsigned char vin2[8];
unsigned char vin3[8];
unsigned char vin4[8];
unsigned char vin5[8];
unsigned char vin6[8];
unsigned char vin7[8];
unsigned char vin8[8];
uint32_t requestID = 0x18EBFF00;

//set up a counter for each received message
uint32_t rxCount = 0;
uint32_t vinCount = 0;

// Declare pin numbers based on the schematic
#define greenLED  23
#define redLED    22
#define CANconnect 2
#define CAN1termination 16
#define CAN2termination 21
#define SDchipSelectPin 15
#define MCPstandbyPin 10
#define MCPresetPin 9

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

//set up a variable to keep track of the timestamp
time_t previousTime = 0;

//Declare a function to get the RTC clock
time_t getTeensy3Time(){
  return Teensy3Clock.get();
}

void setup() {
  //Set up the pin functions
  pinMode(greenLED,OUTPUT);
  pinMode(redLED,OUTPUT);
  pinMode(CANconnect,OUTPUT);
  pinMode(CAN1termination,OUTPUT);
  pinMode(CAN2termination,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(SDchipSelectPin,OUTPUT);
  pinMode(MCPstandbyPin,OUTPUT);
  pinMode(MCPresetPin,OUTPUT);

  digitalWrite(greenLED,greenLEDstate);
  digitalWrite(redLED,redLEDstate);
  digitalWrite(CANconnect,LOW);
  digitalWrite(CAN1termination,LOW);
  digitalWrite(CAN2termination,LOW);
  digitalWrite(LED_BUILTIN,LEDstate);
  digitalWrite(SDchipSelectPin,HIGH);
  digitalWrite(MCPstandbyPin,HIGH);
  digitalWrite(MCPresetPin,HIGH);
  
  Serial.begin(115200);
  while(!Serial); //remove this line if not being used with a serial console.
  
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);
  //Set System Time
  if (timeStatus()!= timeSet) {
    Serial.println(F("Unable to sync with the RTC"));
  } 
  else {
    Serial.println(F("RTC has set the system time"));
  }
  //print the time:
  sprintf(displayBuffer,"Current Time:\t%04i-%02i-%02i %02i:%02i:%02i",year(),month(),day(),hour(),minute(),second());
  Serial.println(displayBuffer); 


  //before entering the loop, set the microsecondprevious time
  previousTime = now();
  //synchronize the millisecondPerSecond timer
  while (now() - previousTime < 1){
    microsecondsPerSecond = 0;
  }  

  
  //start the CAN access
  CANbus.begin();
  rxmsg.timeout = 0;
  if(CAN0.begin(CAN_250KBPS)==CAN_OK) Serial.println("CAN Started");
  else Serial.println("MCP CAN Init Fail.");
  
  Serial.println(F("Teensy 3.2 Flex CAN to Serial Receive Test."));
 
  //print a header
  sprintf(displayBuffer,"%10s %4s-%2s-%2s %2s:%2s:%2s.%06s %8s %1s","Count","YYYY","MM","DD","hh","mm","ss","micros","CAN ID","N");
  Serial.print(displayBuffer); 
  for (uint8_t i = 1; i<9;i++){ //label the byte columns according to J1939 (i.e. start at 1 instead of 0)
    char byteDigits[7]; //declare a byte display array
    sprintf(byteDigits," B%i",i);
    Serial.print(byteDigits); 
  }
  Serial.println();

  //Ask for vehicle information
   txmsg.id = 0x18EAFFF9;
   txmsg.ext = 1;
   txmsg.len = 3;
   txmsg.buf[0] = 0xEB;
   txmsg.buf[1] = 0xFE;
   txmsg.buf[2] = 0x00;
   CANbus.write(txmsg);
   
  // record vehicle information in arrays
   while(vinCount >= 50){
    if (CANbus.read(rxmsg)){
      vinCount++;
      if(rxmsg.id = 0x18EBFF00){
        if(rxmsg.buf[0] == 0x01){
          for(int q; q<= rxmsg.len; q++){
            vin1[q] = rxmsg.buf[q];
          }
        }
        if(rxmsg.buf[0] == 0x02){
          for(int w; w<= rxmsg.len; w++){
            vin2[w] = rxmsg.buf[w];
          }
        }
        if(rxmsg.buf[0] == 0x03){
          for(int e; e<= rxmsg.len; e++){
            vin3[e] = rxmsg.buf[e];
          }
        }
        if(rxmsg.buf[0] == 0x04){
          for(int r; r<= rxmsg.len; r++){
            vin4[r] = rxmsg.buf[r];
          }
        }
        if(rxmsg.buf[0] == 0x05){
          for(int t; t<= rxmsg.len; t++){
            vin5[t] = rxmsg.buf[t];
          }
        }
        if(rxmsg.buf[0] == 0x06){
          for(int y; y<= rxmsg.len; y++){
            vin6[y] = rxmsg.buf[y];
          }
        }
        if(rxmsg.buf[0] == 0x07){
          for(int u; u<= rxmsg.len; u++){
            vin7[u] = rxmsg.buf[u];
            return;
          }
        }
      }
         
    }
   }
}

void loop() {
  
  //check to see if the number of seconds has changed to reset the millisecond timer for each second
  if (now() - previousTime == 1){
    previousTime = now();
    microsecondsPerSecond = 0;
  }
  if(CAN0.readMsgBuf(&len, rxBuf)==CAN_OK){
    if(CAN0.getCanId() == 0x18EBFF00){
      for(int p=0; p<=len; p++){
        rxBuf[p]=request[p];
      }
    }
  }
  if(strcmp(request, vinRequestMessage) == 0){
    CAN0.sendMsgBuf(requestID, 1, 8, vin1);
    CAN0.sendMsgBuf(requestID, 1, 8, vin2);
    CAN0.sendMsgBuf(requestID, 1, 8, vin3);
    CAN0.sendMsgBuf(requestID, 1, 8, vin4);
    CAN0.sendMsgBuf(requestID, 1, 8, vin5);
    CAN0.sendMsgBuf(requestID, 1, 8, vin6);
    CAN0.sendMsgBuf(requestID, 1, 8, vin7);
  }
  //Read and display a CAN Message
  if(CANbus.read(rxmsg)){
    rxCount++;
    
    uint32_t ID = rxmsg.id;
    uint8_t len = rxmsg.len;
    int ext = rxmsg.ext;
    unsigned char message[len];
    for(int i; i<=len; i++){
      message[i] = rxmsg.buf[i];
    }
    CAN0.sendMsgBuf(ID, ext, len, message);
    
    sprintf(displayBuffer,"%10i %04i-%02i-%02i %02i:%02i:%02i.%06d %08X %1i",rxCount,year(),month(),day(),hour(),minute(),second(),int(microsecondsPerSecond),ID,len);
    Serial.print(displayBuffer); 
      
    for (uint8_t i = 0; i<len;i++){ 
      char byteDigits[4]; 
      sprintf(byteDigits," %02X",rxmsg.buf[i]);
      Serial.print(byteDigits); 
    }
    Serial.println();
  }
    
  
  if (toggleTimer >= LEDtoggleTime){
    toggleTimer = 0;
    greenLEDstate = !greenLEDstate;
    redLEDstate = !redLEDstate;
    LEDstate = !LEDstate;
    digitalWrite(greenLED,greenLEDstate);
    digitalWrite(redLED,redLEDstate);
    digitalWrite(LED_BUILTIN,LEDstate);
  }
}
