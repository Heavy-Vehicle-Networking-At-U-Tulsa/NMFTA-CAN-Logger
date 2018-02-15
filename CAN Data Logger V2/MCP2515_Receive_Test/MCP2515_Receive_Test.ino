// demo: CAN-BUS Shield, receive data
#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];


MCP_CAN CAN0(15);                               // Set CS to pin 15


void setup()
{
  
  Serial.begin(115200);
  while(!Serial);
  if(CAN0.begin(CAN_500KBPS)==CAN_OK) Serial.println("CAN Started");
  else Serial.println("MCP CAN Init Fail.");
}
elapsedMillis loopTimer;

void loop()
{
    
    if(CAN0.readMsgBuf(&len, rxBuf)==CAN_OK)              // Read data: len = data length, buf = data byte(s)
      {
        loopTimer = 0;
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
