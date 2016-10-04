// demo: CAN-BUS Shield, receive data
#include <mcp_can.h>
#include <SPI.h>

uint32_t rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

MCP_CAN CAN0(20);


void setup()
{
  Serial.begin(115200);
  while(!Serial);
   
  pinMode(17, INPUT);                            // Setting pin 17 for /INT input
  Serial.println("MCP2515 Library Receive Example...");

 pinMode(20,OUTPUT);
  digitalWrite(20,HIGH);
  pinMode(15,OUTPUT);
  digitalWrite(15,HIGH);
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);
  pinMode(10,OUTPUT);
  digitalWrite(10,LOW);
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  pinMode(21,OUTPUT);
  digitalWrite(21,LOW);
  
  if(CAN0.begin(CAN_250KBPS)==CAN_OK) Serial.println("CAN Started");
  else Serial.println("MCP CAN Init Fail.");
 
}

elapsedMillis loopTimer;

void loop()
{
   if(CAN0.readMsgBuf(&len, rxBuf)==CAN_OK)
    
    {
      loopTimer = 0;
                    // Read data: len = data length, buf = data byte(s)
      rxId = CAN0.getCanId();                    // Get message ID
      Serial.print("ID: ");
      Serial.print(rxId, HEX);
      Serial.print("  Data: ");
      for(int i = 0; i<len; i++)                // Print each byte of the data
      {
        if(rxBuf[i] < 0x10)                     // If data byte is less than 0x10, add a leading zero
        {
          Serial.print("0");
        }
        Serial.print(rxBuf[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
   }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
