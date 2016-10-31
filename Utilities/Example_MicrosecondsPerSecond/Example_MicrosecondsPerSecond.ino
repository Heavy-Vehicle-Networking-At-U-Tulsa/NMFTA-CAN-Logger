#include <TimeLib.h>

elapsedMicros microsecondsPerSecond;

time_t getTeensy3Time(){
  microsecondsPerSecond = 0;
  return Teensy3Clock.get();
}

void setup() {
  // put your setup code here, to run once:
  setSyncProvider(getTeensy3Time);
  setSyncInterval(1); 
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  char timeString[22];
  sprintf(timeString,"%04d-%02d-%02d %02d:%02d:%02d.%06d",year(),month(),day(),hour(),minute(),second(),uint32_t(microsecondsPerSecond));
  Serial.println(timeString);
  
}
