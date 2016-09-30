/*
 * Title: CAN Bus Stress Test Generator
 * 
 * Description: A sketch to generate CAN messages as fast
 * as the hardware will go. The messages are extended ID and
 * transmit a count (32-bit word) and the microseconds (32-bit word)
 * This way a receiving node can test if it can handle 100% bus load.
 * 
 * Author: Dr. Jeremy S. Daily
 * The University of Tulsa, Department of Mechanical Engineering
 * 
 */

#include <FlexCAN.h>
#define BAUDRATE 250000

FlexCAN CANbus(BAUDRATE);
static CAN_message_t txmsg,rxmsg;

uint32_t txCount,rxCount;
uint32_t txOverruns;
uint32_t microSeconds, highCount;
uint32_t txCountRate;
uint32_t txOverrunRate;

boolean txRetVal;
boolean rxRetVal;

int serialOutputInterval = 1000;
elapsedMillis serialOutputTimer;

char printBuffer[4];

    
void setup() {
  // put your setup code here, to run once:
  Serial.println();
  
  CANbus.begin();
  txmsg.len = 8;
  txmsg.ext = 1;
  txmsg.timeout = 0;
  txmsg.id = 0x1CFFFFFF;
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  delay(1000);
  digitalWrite(LED_BUILTIN, 1);

  Serial.print(F("uSec"));
  Serial.print(F("\t"));
  Serial.print(F("Total Count"));
  Serial.print(F("\t"));
  Serial.print(F("Count Per Period"));
  Serial.print(F("\t"));
  Serial.print(F("Total Overruns"));
  Serial.print(F("\t"));
  Serial.print(F("Overruns Per Period"));
  Serial.print(F("\t"));
  Serial.print(F("Dir"));
  Serial.print(F("\t"));
  Serial.print(F("ID"));
  for (uint8_t i = 0; i < 8; i++)
  {
    sprintf(printBuffer,"\tB%i",txmsg.buf[i]);
    Serial.print(F(printBuffer));
  }
  Serial.println();
  

}

void loop() {
  // put your main code here, to run repeatedly:
  microSeconds=micros();
  
  txmsg.buf[0]=(txCount & 0xFF000000) >> 24;
  txmsg.buf[1]=(txCount & 0x00FF0000) >> 16;
  txmsg.buf[2]=(txCount & 0x0000FF00) >> 8;
  txmsg.buf[3]=(txCount & 0x000000FF) >> 0;
  txmsg.buf[4]=(microSeconds & 0xFF000000) >> 24;
  txmsg.buf[5]=(microSeconds & 0x00FF0000) >> 16;
  txmsg.buf[6]=(microSeconds & 0x0000FF00) >> 8;
  txmsg.buf[7]=(microSeconds & 0x000000FF) >> 0;
  txRetVal = CANbus.write(txmsg);
  if (txRetVal) {
    txCount++ ;
    txCountRate++;
  }
  else {
    txOverruns++ ;
    txOverrunRate++ ;
  }
  
  if (serialOutputTimer >= serialOutputInterval)
    {
      serialOutputTimer = 0;
      Serial.print(microSeconds);
      Serial.print("\t");
      Serial.print(txCount);
      Serial.print("\t");
      Serial.print(txCountRate);
      Serial.print("\t");
      Serial.print(txOverruns);
      Serial.print("\t");
      Serial.print(txOverrunRate);
      Serial.print("\t");
      Serial.print("TX\t");
      Serial.print(txmsg.id,HEX);
      for (uint8_t i = 0; i < txmsg.len; i++)
      {
        sprintf(printBuffer,"\t%02X",txmsg.buf[i]);
        Serial.print(printBuffer);
      }
      Serial.print("\t");
      Serial.println();
      txOverrunRate=0;
      if (txCountRate == 0){
        CANbus.end();
        Serial.println(F("Restarting CAN bus"));
        delay(1);
        CANbus.begin();
      }
      txCountRate=0;
  }  
  
}
