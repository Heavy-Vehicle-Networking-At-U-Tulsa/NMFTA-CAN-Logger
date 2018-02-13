#include <FlexCAN.h>
#include <kinetis_flexcan.h>
static CAN_message_t txmsg,txmsg1, rxmsg;
unsigned long txCount, rxCount;
unsigned long txTimer, rxTimer;
uint32_t currentMillis = 0;
uint32_t previousMillis100 = 0;
uint32_t previousMillis10 = 0;
#define BAUDRATE0 250000
#define BAUDRATE1 1000000
int led = 13;

uint32_t RXCount0 = 0;
uint32_t RXCount1 = 0;

#define SILENT_0 39
#define SILENT_1 38
#define SILENT_2 37

void setup() {
  // put your setup code here, to run once:
 Can1.begin(BAUDRATE0);
 Can0.begin(BAUDRATE0);
 pinMode(SILENT_0,OUTPUT);
 pinMode(SILENT_1,OUTPUT);
 pinMode(SILENT_2,OUTPUT);
 // Enable transmission for the CAN TXRX
 digitalWrite(SILENT_0,LOW);
 digitalWrite(SILENT_1,LOW);
 digitalWrite(SILENT_2,LOW);
 
Serial.begin(9600);
 delay(2);
 pinMode (led, OUTPUT);
 digitalWrite (led, HIGH);
  Serial.println("Sending Basic Traffic on bus");
  delay(2000);
}
void loop() {
  digitalWrite (led, LOW);
  delay(100);
  // put your main code here, to run repeatedly:
  currentMillis = millis();
  Serial.println();
  Serial.println("Looping");
if (currentMillis - previousMillis10 >= 10){
  digitalWrite (led, HIGH);
  delay(100);
  previousMillis10 = currentMillis;
txmsg.id = 0xFFF;
txmsg.len = 4;
txmsg.buf[0] = 0;
txmsg.buf[1] = 0;
txmsg.buf[2] = 0;
txmsg.buf[3] = 0;
txmsg1.id = 0x000;
txmsg1.len = 4;
txmsg1.buf[0] = 255;
txmsg1.buf[1] = 255;
txmsg1.buf[2] = 255;
txmsg1.buf[3] = 255;
//Can1.write(txmsg1);
Can0.write(txmsg);
}
}
