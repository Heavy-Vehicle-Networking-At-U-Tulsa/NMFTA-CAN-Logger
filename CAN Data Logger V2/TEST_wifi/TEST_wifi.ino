/*
 * This example check if the firmware loaded on the WiFi101
 * shield is updated.
 *
 * Circuit:
 * - WiFi101 Shield attached
 *
 * Created 29 July 2015 by Cristian Maglie
 * This code is in the public domain.
 */
#include <SPI.h>
#include <WiFi101.h>
#include <driver/source/nmasic.h>
#include <WiFiUdp.h>


#define WIFI_MOSI_PIN     0
#define WIFI_MISO_PIN    1
#define WIFI_SCLK_PIN  32
#define WIFI_CS_PIN  31

#define WIFI_EN_PIN  5
#define WIFI_IRQ_PIN   23
#define WIFI_RST_PIN   6
#define WIFI_WAKE_PIN    36
void setup() {
  
  SPI1.begin();
  pinMode(WIFI_EN_PIN, OUTPUT);
  pinMode(WIFI_CS_PIN, OUTPUT);
  digitalWrite(WIFI_CS_PIN,HIGH);
  digitalWrite(WIFI_EN_PIN,HIGH);
  
  //Configure pins for Adafruit ATWINC1500 Breakout
  WiFi.setPins(WIFI_CS_PIN, WIFI_IRQ_PIN, WIFI_RST_PIN);
  
  // Initialize serial
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Print a welcome message
  Serial.println("WiFi101 firmware check.");
  Serial.println();

  // Check for the presence of the shield
  Serial.print("WiFi101 shield: ");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("NOT PRESENT");
    return; // don't continue
  }
  Serial.println("DETECTED");

  // Print firmware version on the shield
  String fv = WiFi.firmwareVersion();
  String latestFv;
  Serial.print("Firmware version installed: ");
  Serial.println(fv);

  if (REV(GET_CHIPID()) >= REV_3A0) {
    // model B
    latestFv = WIFI_FIRMWARE_LATEST_MODEL_B;
  } else {
    // model A
    latestFv = WIFI_FIRMWARE_LATEST_MODEL_A;
  }

  // Print required firmware version
  Serial.print("Latest firmware version available : ");
  Serial.println(latestFv);

  // Check if the latest version is installed
  Serial.println();
  if (fv == latestFv) {
    Serial.println("Check result: PASSED");
  } else {
    Serial.println("Check result: NOT PASSED");
    Serial.println(" - The firmware version on the shield do not match the");
    Serial.println("   version required by the library, you may experience");
    Serial.println("   issues or failures.");
  }
}

void loop() {
  // do nothing
}

