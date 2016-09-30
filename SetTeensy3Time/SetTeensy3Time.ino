/*
 * Set Time based on Serial Message from PC
 * 
 */

#include <TimeLib.h>

elapsedMillis displayCounter;

void setup()  {
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);

  Serial.begin(115200);
  while (!Serial);  // Wait for a serial connection to open
  delay(100);
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }
}

void loop() {
  if (Serial.available()) {
    time_t t = processSyncMessage();
    if (t != 0) {
      Teensy3Clock.set(t); // set the RTC
      setTime(t);
    }
  }
  
  if (displayCounter >=1000){
    displayCounter = 0;
    char timeStamp[35];
    sprintf(timeStamp,"%04d-%02d-%02d %02d:%02d:%02d",year(),month(),day(),hour(),minute(),second());
    Serial.println(timeStamp);
  }  
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

uint32_t processSyncMessage() {
  uint32_t pctime = 0L;
  const uint32_t DEFAULT_TIME = 1357041600; // Jan 1 2013 

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     return pctime;
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}
