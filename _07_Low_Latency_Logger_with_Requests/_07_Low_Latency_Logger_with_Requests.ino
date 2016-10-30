/*
 * This program logs data to a binary file.  Functions are included
 * to convert the binary file to a csv text file.
 *
 * Samples are logged at regular intervals.  The maximum logging rate
 * depends on the quality of your SD card and the time required to
 * read sensor data.  This example has been tested at 500 Hz with
 * good SD card on an Uno.  4000 HZ is possible on a Due.
 *
 * If your SD card has a long write latency, it may be necessary to use
 * slower sample rates.  Using a Mega Arduino helps overcome latency
 * problems since 13 512 byte buffers will be used.
 *
 * Data is written to the file using a SD multiple block write command.
 */

// log file base name.  Must be five characters or less. Change this line for each upload
// The first four characters should match the label on the cable.
#define FILE_BASE_NAME "TU01_"
 
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <FlexCAN.h>
#include "kinetis_flexcan.h"
#include <TimeLib.h>
#include <EEPROM.h>

// User data functions.  Modify these functions for your data items.
#include "UserDataType.h"  // Edit this include file to change data_t.

TimeElements tm;
//set up a variable to keep track of the timestamp
time_t previousTime = 0;

static CAN_message_t rxmsg,txmsg;

//------------------------------------------------------------------------------
uint32_t getBaudRate() {
  uint32_t baudRateList[3] = {250000,500000,125000}; 
  rxmsg.timeout = 1000;
  digitalWrite(LED_BUILTIN,HIGH);
  while (1){
    for(uint8_t i=0;i<sizeof(baudRateList);i++){
      uint32_t baudrate = baudRateList[i];
      Serial.print(F("Trying Baudrate of "));
      Serial.println(baudrate);
      FlexCAN testBus(baudrate);
      Serial.println(F("Beginning test."));
      testBus.begin();
      if(testBus.read(rxmsg)){
        Serial.print("Baudrate is confirmed to be ");
        Serial.println(baudrate);
        return baudrate;
      }
      else{
       testBus.end();
//       delete testBus
      }
      Serial.println(F("Trying again."));
    }   
  }
}

FlexCAN CANbus(getBaudRate());

IntervalTimer oneSecondReset;
elapsedMicros microsecondsPerSecond;
elapsedMillis buttonPressTimer;
elapsedMillis lastCANmessageTimer;
elapsedMillis LEDblinkTimer;

boolean redLEDstate;
boolean greenLEDstate;

uint32_t rxCount = 0;
//set up a display buffer
char displayBuffer[100]; 


void resetMicros() {
  microsecondsPerSecond = 0; //reset the timer
}

// Acquire a data record.
void acquireCANData(data_t* data) {
  data->timeStamp = now();
  data->usec = micros();
  data->ID =  rxmsg.id; 
  data->DLC = (rxmsg.len << 24) | (0x00FFFFFF & uint32_t(microsecondsPerSecond));
  memset(data->dataField,0xFF,8);
  for (uint8_t i = 0; i < rxmsg.len; i++){
    data->dataField[i] = rxmsg.buf[i];  
  }
}

// Print a data record.
void printData(Print* pr, data_t* data) {
  time_t recordTime = data->timeStamp;
  uint32_t usecPerSec = data->DLC & 0x00FFFFFF;
  char timeString[100];
  sprintf(timeString,"%04d-%02d-%02d,%02d:%02d:%02d.%06d,",year(recordTime),month(recordTime),day(recordTime),hour(recordTime),minute(recordTime),second(recordTime),usecPerSec);
  pr->print(timeString);
  
  sprintf(timeString,"%10d.%06d",data->timeStamp,usecPerSec);
  pr->print(timeString);
  
  pr->print(data->usec);
   
  char IDString[12];
  sprintf(IDString,",%08X,",data->ID);
  pr->print(IDString);
  pr->print((data->DLC & 0xFF000000) >> 24);
  for (int i = 0; i < 8; i++) {
    char entry[5];
    sprintf(entry,",%02X",data->dataField[i]);
    pr->print(entry);
  }
  pr->println();
}

// Print data header.
void printHeader(Print* pr) {
  pr->print(F("YYYY-MM-DD HH:MM:SS.usec,"));
  pr->print(F("Unix timeStamp,"));
  pr->print(F("System uSecs,"));
  pr->print(F("ID,"));
  pr->print(F("DLC"));
  for (uint8_t i = 0; i < 8; i++) {
    pr->print(F(",B"));
    pr->print(i);
  }
  pr->println();
}
//==============================================================================
// Start of configuration constants.
//==============================================================================
//Interval between data records in microseconds.
const uint32_t LOG_INTERVAL_USEC = 200;
//------------------------------------------------------------------------------
// Pin definitions.
//
// SD chip select pin.
const uint8_t SD_CS_PIN = 15;


// Reset pin 
const uint8_t MCP_RESET_PIN = 9;

// standBy pin 
const uint8_t MCP_STDBY_PIN = 10;

// chip select pin 
const uint8_t MCP_CS_PIN = 20;

// chip select pin 
const uint8_t MCP_INT_PIN = 17;

// SD Card Contact Pin 
const uint8_t SD_CONTACT_PIN = 14;

//CAN Connec Pins
const uint8_t CAN_CONNECT_PIN = 2;
const uint8_t CAN1_TERMINATION_PIN = 16;
const uint8_t CAN2_TERMINATION_PIN = 21;


//
// Digital pin to indicate an error, set to -1 if not used.
// The led blinks for fatal errors. The led goes on solid for SD write
// overrun errors and logging continues.
const uint8_t ERROR_LED_PIN = 22;
const uint8_t GREEN_LED_PIN = 23;


//------------------------------------------------------------------------------
// File definitions.
//
// Maximum file size in blocks.
// The program creates a contiguous file with FILE_BLOCK_COUNT 512 byte blocks.
// This file is flash erased using special SD commands.  The file will be
// truncated if logging is stopped early.
//const uint32_t FILE_BLOCK_COUNT = 256;
//const uint32_t FILE_BLOCK_COUNT = 2097152; //1 GB
const uint32_t FILE_BLOCK_COUNT = 4194304; //2 GB
//const uint32_t FILE_BLOCK_COUNT = 8388607; //4 GB - 512



//------------------------------------------------------------------------------
// Buffer definitions.
//
// The logger will use SdFat's buffer plus BUFFER_BLOCK_COUNT additional
// buffers.
//
#ifndef RAMEND
// Assume ARM. Use total of nine 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 8;
//
#elif RAMEND < 0X8FF
#error Too little SRAM
//
#elif RAMEND < 0X10FF
// Use total of two 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 1;
//
#elif RAMEND < 0X20FF
// Use total of five 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 4;
//
#else  // RAMEND
// Use total of 13 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 12;
#endif  // RAMEND
//==============================================================================
// End of configuration constants.
//==============================================================================
// Temporary log file.  Will be deleted if a reset or power failure occurs.
#define TMP_FILE_NAME "tmp_log.bin"

// Size of file base name.  Must not be larger than five.
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;

SdFat sd;

SdBaseFile binFile;

char binName[13] = FILE_BASE_NAME "000.bin";

// Number of data records in a block.
const uint16_t DATA_DIM = (512 - 4)/sizeof(data_t);

//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - 4 - DATA_DIM*sizeof(data_t);

struct block_t {
  uint16_t count;
  uint16_t overrun;
  data_t data[DATA_DIM];
  uint8_t fill[FILL_DIM];
};

//------------------------------------------------------------------------------
/*
 * User provided date time callback function.
 * See SdFile::dateTimeCallback() for usage.
 */
void dateTime(uint16_t* FATdate, uint16_t* FATtime) {
  // User gets date and time from GPS or real-time
  // clock in real callback function

  // return date using FAT_DATE macro to format fields
  *FATdate = FAT_DATE(year(), month(), day());

  // return time using FAT_TIME macro to format fields
  *FATtime = FAT_TIME(hour(), minute(), second());
}


const uint8_t QUEUE_DIM = BUFFER_BLOCK_COUNT + 2;

block_t* emptyQueue[QUEUE_DIM];
uint8_t emptyHead;
uint8_t emptyTail;

block_t* fullQueue[QUEUE_DIM];
uint8_t fullHead;
uint8_t fullTail;

// Advance queue index.
inline uint8_t queueNext(uint8_t ht) {
  return ht < (QUEUE_DIM - 1) ? ht + 1 : 0;
}
//==============================================================================
// Error messages stored in flash.
#define error(msg) errorFlash(F(msg))
//------------------------------------------------------------------------------
void errorFlash(const __FlashStringHelper* msg) {
  sd.errorPrint(msg);
  fatalBlink();
}
//------------------------------------------------------------------------------
//
void fatalBlink() {
  while (true) {
    if (ERROR_LED_PIN >= 0) {
      digitalWrite(ERROR_LED_PIN, HIGH);
      delay(80);
      digitalWrite(ERROR_LED_PIN, LOW);
      delay(80);
      
    }
  }
}
//==============================================================================
// Convert binary file to csv file.
void binaryToCsv() {
  uint8_t lastPct = 0;
  block_t block;
  uint32_t t0 = millis();
  uint32_t syncCluster = 0;
  SdFile csvFile;
  char csvName[13];

  if (!binFile.isOpen()) {
    Serial.println();
    Serial.println(F("No current binary file"));
    return;
  }
  binFile.rewind();
  // Create a new csvFile.
  strcpy(csvName, binName);
  strcpy(&csvName[BASE_NAME_SIZE + 3], "csv");

  if (!csvFile.open(csvName, O_WRITE | O_CREAT | O_TRUNC)) {
    error("open csvFile failed");
  }
  Serial.println();
  Serial.print(F("Writing: "));
  Serial.print(csvName);
  Serial.println(F(" - type any character to stop"));
  printHeader(&csvFile);
  uint32_t tPct = millis();
  while (!Serial.available() && binFile.read(&block, 512) == 512) {
     
    uint16_t i;
    if (block.count == 0) {
      break;
    }
    if (block.overrun) {
      csvFile.print(F("OVERRUN,"));
      csvFile.println(block.overrun);
    }
    for (i = 0; i < block.count; i++) {
      printData(&csvFile, &block.data[i]);
    }
    if (csvFile.curCluster() != syncCluster) {
      csvFile.sync();
      syncCluster = csvFile.curCluster();
    }
    if ((millis() - tPct) > 1000) {
      uint8_t pct = binFile.curPosition()/(binFile.fileSize()/100);
      if (pct != lastPct) {
        tPct = millis();
        lastPct = pct;
        Serial.print(pct, DEC);
        Serial.println('%');
      }
    }
    if (Serial.available()) {
      break;
    }
  }
  csvFile.close();
  Serial.print(F("Done: "));
  Serial.print(0.001*(millis() - t0));
  Serial.println(F(" Seconds"));
}
//------------------------------------------------------------------------------
// read data file and check for overruns
void checkOverrun() {
  bool headerPrinted = false;
  block_t block;
  uint32_t bgnBlock, endBlock;
  uint32_t bn = 0;

  if (!binFile.isOpen()) {
    Serial.println();
    Serial.println(F("No current binary file"));
    return;
  }
  if (!binFile.contiguousRange(&bgnBlock, &endBlock)) {
    error("contiguousRange failed");
  }
  binFile.rewind();
  Serial.println();
  Serial.println(F("Checking overrun errors - type any character to stop"));
  while (binFile.read(&block, 512) == 512) {
    if (block.count == 0) {
      break;
    }
    if (block.overrun) {
      if (!headerPrinted) {
        Serial.println();
        Serial.println(F("Overruns:"));
        Serial.println(F("fileBlockNumber,sdBlockNumber,overrunCount"));
        headerPrinted = true;
      }
      Serial.print(bn);
      Serial.print(',');
      Serial.print(bgnBlock + bn);
      Serial.print(',');
      Serial.println(block.overrun);
    }
    bn++;
  }
  if (!headerPrinted) {
    Serial.println(F("No errors found"));
  } else {
    Serial.println(F("Done"));
  }
}
//------------------------------------------------------------------------------
// dump data file to Serial
void dumpData() {
  block_t block;
  if (!binFile.isOpen()) {
    Serial.println();
    Serial.println(F("No current binary file"));
    return;
  }
  binFile.rewind();
  Serial.println();
  Serial.println(F("Type any character to stop"));
  delay(100);
  printHeader(&Serial);
  while (!Serial.available() && binFile.read(&block , 512) == 512) {
  
    if (block.count == 0) {
      break;
    }
    if (block.overrun) {
      Serial.print(F("OVERRUN,"));
      Serial.println(block.overrun);
    }
    for (uint32_t i = 0; i < block.count; i++) {
      printData(&Serial, &block.data[i]);
    }
  }
  Serial.println(F("Done"));
}
//------------------------------------------------------------------------------
// log data
// max number of blocks to erase per erase call
uint32_t const ERASE_SIZE = 262144L;
void logData() {
  uint32_t bgnBlock, endBlock;
  lastCANmessageTimer = 0;
  // Allocate extra buffer space.
  block_t block[BUFFER_BLOCK_COUNT];
  block_t* curBlock = 0;
  Serial.println();

  // Find unused file name.
  if (BASE_NAME_SIZE > 5) {
    error("FILE_BASE_NAME too long");
  }
  while (sd.exists(binName)) {
    if (binName[BASE_NAME_SIZE + 2] != '9') {
      binName[BASE_NAME_SIZE + 2]++;
    }
    else {
      binName[BASE_NAME_SIZE + 2] = '0';
      if (binName[BASE_NAME_SIZE + 1] != '9') {
        binName[BASE_NAME_SIZE + 1]++;
      }
      else {
        binName[BASE_NAME_SIZE+1] = '0';
        binName[BASE_NAME_SIZE]++;
      }
    }
    
   if (binName[BASE_NAME_SIZE] == '9' & binName[BASE_NAME_SIZE+1] == '9' & binName[BASE_NAME_SIZE+2] == '9') {
        error("Can't create file name");
    }
  }
  
  // Delete old tmp file.
  if (sd.exists(TMP_FILE_NAME)) {
    Serial.println(F("Deleting tmp file"));
    if (!sd.remove(TMP_FILE_NAME)) {
      error("Can't remove tmp file");
    }
  }
  // Create new file.
  Serial.println(F("Creating new file"));
  binFile.close();
  if (!binFile.createContiguous(sd.vwd(),
                                TMP_FILE_NAME, 512 * FILE_BLOCK_COUNT)) {
    error("createContiguous failed");
  }
  // Get the address of the file on the SD.
  if (!binFile.contiguousRange(&bgnBlock, &endBlock)) {
    error("contiguousRange failed");
  }
  // Use SdFat's internal buffer.
  uint8_t* cache = (uint8_t*)sd.vol()->cacheClear();
  if (cache == 0) {
    error("cacheClear failed");
  }

  // Flash erase all data in the file.
  Serial.println(F("Erasing all data"));
  uint32_t bgnErase = bgnBlock;
  uint32_t endErase;
  while (bgnErase < endBlock) {
    endErase = bgnErase + ERASE_SIZE;
    if (endErase > endBlock) {
      endErase = endBlock;
    }
    if (!sd.card()->erase(bgnErase, endErase)) {
      error("erase failed");
    }
    bgnErase = endErase + 1;
  }
  // Start a multiple block write.
  if (!sd.card()->writeStart(bgnBlock, FILE_BLOCK_COUNT)) {
    error("writeBegin failed");
  }
  // Initialize queues.
  emptyHead = emptyTail = 0;
  fullHead = fullTail = 0;

  // Use SdFat buffer for one block.
  emptyQueue[emptyHead] = (block_t*)cache;
  emptyHead = queueNext(emptyHead);

  // Put rest of buffers in the empty queue.
  for (uint8_t i = 0; i < BUFFER_BLOCK_COUNT; i++) {
    emptyQueue[emptyHead] = &block[i];
    emptyHead = queueNext(emptyHead);
  }

  //wait for a CAN message to show up
  Serial.println(F("Waiting for CAN Messages..."));
  while (CANbus.read(rxmsg) == 0);
  Serial.println(F("CAN Message Found."));
  
  Serial.println(F("Logging - type any character to stop"));
  // Wait for Serial Idle.
  Serial.flush();
  delay(10);
  uint32_t bn = 0;
  uint32_t t0 = millis();
  uint32_t t1 = t0;
  uint32_t overrun = 0;
  uint32_t overrunTotal = 0;
  uint32_t count = 0;
  uint32_t maxLatency = 0;
  int32_t diff;
  // Start at a multiple of interval.
  uint32_t logTime = micros()/LOG_INTERVAL_USEC + 1;
  logTime *= LOG_INTERVAL_USEC;
  bool closeFile = false;

  //Write the new message to RAM
  //acquireCANData(&curBlock->data[curBlock->count++]);
  
  digitalWrite(GREEN_LED_PIN,HIGH);
  while (1) {
    if (now() - previousTime >= 1){
      previousTime = now();
      microsecondsPerSecond = 0;
    }
  
    // Time for next data record.
    //logTime += LOG_INTERVAL_USEC;
    if (Serial.available()) {
      closeFile = true;
      buttonPressTimer = 0;
    }

    if (closeFile) {
      Serial.println(F("Closing Temp Buffer File."));
     // Serial.print("curBlock: ");
     // Serial.println(curBlock);
      Serial.print("curBlock->count: ");
      Serial.println(curBlock->count);
      
      
      if (curBlock != 0 && curBlock->count >= 0) {
        // Put buffer in full queue.
        fullQueue[fullHead] = curBlock;
        fullHead = queueNext(fullHead);
        Serial.print(F("Updated fullHead to "));
        Serial.println(fullHead);
        curBlock = 0;
      }
    } 
    else
    {
      if (curBlock == 0 && emptyTail != emptyHead) {
        curBlock = emptyQueue[emptyTail];
        emptyTail = queueNext(emptyTail);
        curBlock->count = 0;
        curBlock->overrun = overrun;
        overrun = 0;
      }
//      do {
//        diff = logTime - micros();
//      } while(diff > 0);
//      if (diff < -10) {
//        error("LOG_INTERVAL_USEC too small");
//      }
      if (curBlock == 0) {
        overrun++;
        Serial.print(F("Overrun: "));
        Serial.println(overrun);
      } 
      else 
      {
        if (CANbus.read(rxmsg)){
          acquireCANData(&curBlock->data[curBlock->count++]);
          lastCANmessageTimer = 0;
//          rxCount++;
//          
//          uint32_t ID = rxmsg.id;
//          uint8_t len = rxmsg.len;
//          
//          sprintf(displayBuffer,"%10i %04i-%02i-%02i %02i:%02i:%02i.%06d %08X %1i",rxCount,year(),month(),day(),hour(),minute(),second(),int(microsecondsPerSecond),ID,len);
//          Serial.print(displayBuffer); 
//            
//          for (uint8_t i = 0; i<len;i++){ 
//            char byteDigits[4]; 
//            sprintf(byteDigits," %02X",rxmsg.buf[i]);
//            Serial.print(byteDigits); 
//          }
//          Serial.println();
        }
        
        if (LEDblinkTimer >= 500){
          LEDblinkTimer = 0; 
          greenLEDstate = !greenLEDstate;
          digitalWrite(GREEN_LED_PIN,greenLEDstate);
        }

        if (lastCANmessageTimer > 30000) closeFile = true; //Close the file after 30 seconds of CAN silence
        
        if (curBlock->count >= DATA_DIM) {
          //Serial.println(F("curBlock->count >= DATA_DIM"));
          fullQueue[fullHead] = curBlock;
          fullHead = queueNext(fullHead);
          curBlock = 0;
        }
      }
    }

    if (fullHead == fullTail) {
      //Serial.println(F("fullHead == fullTail"));
      // Exit loop if done.
      if (closeFile) {
        break;
      }
    } else if (!sd.card()->isBusy()) {
      //Serial.println(F("!sd.card()->isBusy()"));
      // Get address of block to write.
      block_t* pBlock = fullQueue[fullTail];
      fullTail = queueNext(fullTail);
      // Write block to SD.
      uint32_t usec = micros();
      if (!sd.card()->writeData((uint8_t*)pBlock)) {
        error(F("write data failed"));
      }
      usec = micros() - usec;
      t1 = millis();
      if (usec > maxLatency) {
        maxLatency = usec;
      }
      count += pBlock->count;

      // Add overruns and possibly light LED.
      if (pBlock->overrun) {
        overrunTotal += pBlock->overrun;
        if (ERROR_LED_PIN >= 0) {
          digitalWrite(ERROR_LED_PIN, HIGH);
        }
      }
      // Move block to empty queue.
      emptyQueue[emptyHead] = pBlock;
      emptyHead = queueNext(emptyHead);
      bn++;
      if (bn == FILE_BLOCK_COUNT) {
         Serial.println(F("bn == FILE_BLOCK_COUNT"));
        // File full so stop
        closeFile = true;
        break;
      }
    }
  }
  if (!sd.card()->writeStop()) {
    error("writeStop failed");
  }
  digitalWrite(ERROR_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  // Truncate file if recording stopped early.
  if (bn != FILE_BLOCK_COUNT) {
    Serial.println(F("Truncating file"));
    Serial.print(F("uint32_t(512 * bn)"));
    Serial.println(uint32_t(512 * bn));
    if (!binFile.truncate(uint32_t(512 * bn))) {
      error("Can't truncate file");
    }
  }
  if (!binFile.rename(sd.vwd(), binName)) {
    error("Can't rename file");
  }
  Serial.print(F("File renamed: "));
  Serial.println(binName);
  Serial.print(F("Max block write usec: "));
  Serial.println(maxLatency);
  Serial.print(F("Record time sec: "));
  Serial.println(0.001*(t1 - t0), 3);
  Serial.print(F("Sample count: "));
  Serial.println(count);
  Serial.print(F("Samples/sec: "));
  Serial.println((1000.0)*count/(t1-t0));
  Serial.print(F("Overruns: "));
  Serial.println(overrunTotal);
  Serial.println(F("Done"));
}
//------------------------------------------------------------------------------

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void setup(void) {
  CANbus.begin();
  rxmsg.timeout = 0;

  pinMode(ERROR_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(CAN1_TERMINATION_PIN, OUTPUT); 
  pinMode(CAN2_TERMINATION_PIN, OUTPUT);
  pinMode(CAN_CONNECT_PIN, OUTPUT);
  pinMode(MCP_RESET_PIN, OUTPUT);
  pinMode(MCP_STDBY_PIN, OUTPUT);
  pinMode(MCP_INT_PIN, OUTPUT);
  pinMode(MCP_CS_PIN, OUTPUT);
  
  
  pinMode(SD_CONTACT_PIN, INPUT_PULLUP);

  digitalWrite(CAN1_TERMINATION_PIN,HIGH);
  digitalWrite(CAN2_TERMINATION_PIN,HIGH);
  digitalWrite(CAN_CONNECT_PIN,HIGH);
  digitalWrite(MCP_RESET_PIN,HIGH);
  digitalWrite(MCP_STDBY_PIN,HIGH);
  digitalWrite(MCP_CS_PIN,HIGH);
 
 
  Serial.begin(9600);
  digitalWrite(ERROR_LED_PIN,HIGH);
  digitalWrite(GREEN_LED_PIN,HIGH);
  delay(1000);
  digitalWrite(ERROR_LED_PIN,LOW);
  digitalWrite(GREEN_LED_PIN,LOW);
  
  setSyncProvider(getTeensy3Time);
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

 
  
  char timeString[22];
  time_t previousTime = now();
  while (now() - previousTime < 1) resetMicros();
  oneSecondReset.begin(resetMicros,1000000);
  
  sprintf(timeString,"%04d-%02d-%02d %02d:%02d:%02d.%06d",year(),month(),day(),hour(),minute(),second(),uint32_t(microsecondsPerSecond));
  Serial.println(timeString);
  
  Serial.print(F("FreeRam: "));
  Serial.println(FreeRam());
  Serial.print(F("Records/block: "));
  Serial.println(DATA_DIM);
  if (sizeof(block_t) != 512) {
    error("Invalid block size");
  }
  // initialize file system.
  if (!sd.begin(SD_CS_PIN, SPI_FULL_SPEED)) {
    sd.initErrorPrint();
    fatalBlink();
  }
  // set date time callback function
  SdFile::dateTimeCallback(dateTime);

  //before entering the loop, set the microsecondprevious time
  previousTime = now();
  //synchronize the millisecondPerSecond timer
  while (now() - previousTime < 1){
    microsecondsPerSecond = 0;
  }  
}




//------------------------------------------------------------------------------

void loop(void) {
  logData(); // automatically start logging. Otherwise use the button
}

/*
 * Old Loop code
 * 
 * if (ERROR_LED_PIN >= 0) {
    digitalWrite(ERROR_LED_PIN, LOW);
  }
  
  // discard any input
  while (Serial.read() >= 0) {}
  
  Serial.println();
  Serial.println(F("type:"));
  Serial.println(F("c - convert file to csv"));
  Serial.println(F("d - dump data to Serial"));
  Serial.println(F("e - overrun error details"));
  Serial.println(F("r - record data"));

 while(!Serial.available() )  ;
  
  buttonPressTimer = 0;
  delay(50); //debounce
  char c;
  if (Serial.available()) c = tolower(Serial.read());
  while (Serial.read() >= 0);
 // if (!digitalRead(BUTTON_PIN)) c = 'r';
 // while (!digitalRead(BUTTON_PIN)){
//    if (buttonPressTimer > 2000){
//      c='c';
//      break;
//    }
//  }
  
  if (c == 'c') {
    analogWrite(ERROR_LED_PIN,64);
    binaryToCsv();
    analogWrite(ERROR_LED_PIN,0);
  } else if (c == 'd') {
    dumpData();
  } else if (c == 'e') {
    checkOverrun();
  } else if (c == 'r') {
    logData();
  } else {
    Serial.println(F("Invalid entry"));
  }
  
  digitalWrite(ERROR_LED_PIN, LOW);
  
 */
