#ifndef UserDataType_h
#define UserDataType_h
struct data_t {
  time_t timeStamp;
  uint32_t usec;
  uint32_t DLC;
  uint32_t ID;
  uint8_t dataField[8];
  
};
#endif  // UserDataType_h
