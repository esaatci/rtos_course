// AP_Lab6.c
// Runs on either MSP432 or TM4C123
// see GPIO.c file for hardware connections

// Daniel Valvano and Jonathan Valvano
// November 20, 2016
// CC2650 booster or CC2650 LaunchPad, CC2650 needs to be running SimpleNP 2.2 (POWERSAVE)

#include <stdint.h>
#include "../inc/UART0.h"
#include "../inc/UART1.h"
#include "../inc/AP.h"
#include "AP_Lab6.h"
//**debug macros**APDEBUG defined in AP.h********
#ifdef APDEBUG
#define OutString(STRING) UART0_OutString(STRING)
#define OutUHex(NUM) UART0_OutUHex(NUM)
#define OutUHex2(NUM) UART0_OutUHex2(NUM)
#define OutChar(N) UART0_OutChar(N)
#else
#define OutString(STRING)
#define OutUHex(NUM)
#define OutUHex2(NUM)
#define OutChar(N)
#endif

//****links into AP.c**************
extern const uint32_t RECVSIZE;
extern uint8_t RecvBuf[];
typedef struct characteristics{
  uint16_t theHandle;          // each object has an ID
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data, stored little endian
  void (*callBackRead)(void);  // action if SNP Characteristic Read Indication
  void (*callBackWrite)(void); // action if SNP Characteristic Write Indication
}characteristic_t;
extern const uint32_t MAXCHARACTERISTICS;
extern uint32_t CharacteristicCount;
extern characteristic_t CharacteristicList[];
typedef struct NotifyCharacteristics{
  uint16_t uuid;               // user defined
  uint16_t theHandle;          // each object has an ID (used to notify)
  uint16_t CCCDhandle;         // generated/assigned by SNP
  uint16_t CCCDvalue;          // sent by phone to this object
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data array, stored little endian
  void (*callBackCCCD)(void);  // action if SNP CCCD Updated Indication
}NotifyCharacteristic_t;
extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];
//**************Lab 6 routines*******************
// **********SetFCS**************
// helper function, add check byte to message
// assumes every byte in the message has been set except the FCS
// used the length field, assumes less than 256 bytes
// FCS = 8-bit EOR(all bytes except SOF and the FCS itself)
// Inputs: pointer to message
//         stores the FCS into message itself
// Outputs: none
void SetFCS(uint8_t *msg){
//****You implement this function as part of Lab 6*****
  uint8_t len, fcs;
  uint16_t i;

  // get the length
  len = msg[1];
  // init the byte
  fcs = 0;
  //iterate and calculate xor
  for (i = 1; i < len + 5; i++) {
    fcs ^= msg[i];
  }
  // set fcs
  msg[len + 5] = fcs;
}
//*************BuildGetStatusMsg**************
// Create a Get Status message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetStatusMsg(uint8_t *msg){
// hint: see NPI_GetStatus in AP.c
//****You implement this function as part of Lab 6*****
  msg[0] = SOF;
  msg[1] = 0x00;
  msg[2] = 0x00;
  msg[3] = 0x55;
  msg[4] = 0x06;
  msg[5] = 0x53;
}
//*************Lab6_GetStatus**************
// Get status of connection, used in Lab 6
// Input:  none
// Output: status 0xAABBCCDD
// AA is GAPRole Status
// BB is Advertising Status
// CC is ATT Status
// DD is ATT method in progress
uint32_t Lab6_GetStatus(void){volatile int r; uint8_t sendMsg[8];
  // OutString("\n\rGet Status");
  BuildGetStatusMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return (RecvBuf[4]<<24)+(RecvBuf[5]<<16)+(RecvBuf[6]<<8)+(RecvBuf[7]);
}

//*************BuildGetVersionMsg**************
// Create a Get Version message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetVersionMsg(uint8_t *msg){
// hint: see NPI_GetVersion in AP.c
//****You implement this function as part of Lab 6*****
  msg[0] = SOF;
  msg[1] = 0x00;
  msg[2] = 0x00;
  msg[3] = 0x35;
  msg[4] = 0x03;
  msg[5] = 0x36;
}
//*************Lab6_GetVersion**************
// Get version of the SNP application running on the CC2650, used in Lab 6
// Input:  none
// Output: version
uint32_t Lab6_GetVersion(void){volatile int r;uint8_t sendMsg[8];
  // OutString("\n\rGet Version");
  BuildGetVersionMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return (RecvBuf[5]<<8)+(RecvBuf[6]);
}

//*************BuildAddServiceMsg**************
// Create an Add service message, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        pointer to empty buffer of at least 9 bytes
// Output none
// build the necessary NPI message that will add a service
void BuildAddServiceMsg(uint16_t uuid, uint8_t *msg){
//****You implement this function as part of Lab 6*****
  uint8_t uuidB1, uuidB2;

  msg[0] = SOF;
  // length = 3
  msg[1] = 0x03;
  msg[2] = 0x00;
  // SNP Add Service
  msg[3] = 0x35;
  msg[4] = 0x81;
  // Primary Service
  msg[5] = 0x01;
  // UUID
	uuidB1 = (uint8_t) (uuid & 0xFF);
  uuidB2 = (uint8_t) (uuid >> 8);
  msg[6] = uuidB1;
  msg[7] = uuidB2;

  SetFCS(msg);
}
//*************Lab6_AddService**************
// Add a service, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
// Output APOK if successful,
//        APFAIL if SNP failure
int Lab6_AddService(uint16_t uuid){ int r; uint8_t sendMsg[12];
  // OutString("\n\rAdd service");
  BuildAddServiceMsg(uuid,sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}
//*************AP_BuildRegisterServiceMsg**************
// Create a register service message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will register a service
void BuildRegisterServiceMsg(uint8_t *msg){
//****You implement this function as part of Lab 6*****

  msg[0] = SOF;
  //length
  msg[1] = 0x00;
  msg[2] = 0x00;
  // SNP Register Service
  msg[3] = 0x35;
  msg[4] = 0x84;
  msg[5] = 0xB1;
}
//*************Lab6_RegisterService**************
// Register a service, used in Lab 6
// Inputs none
// Output APOK if successful,
//        APFAIL if SNP failure
int Lab6_RegisterService(void){ int r; uint8_t sendMsg[8];
  OutString("\n\rRegister service");
  BuildRegisterServiceMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}

//*************BuildAddCharValueMsg**************
// Create a Add Characteristic Value Declaration message, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write, 0x10=notify
//        pointer to empty buffer of at least 14 bytes
// Output none
// build the necessary NPI message that will add a characteristic value
void BuildAddCharValueMsg(uint16_t uuid,
  uint8_t permission, uint8_t properties, uint8_t *msg){
// set RFU to 0 and
// set the maximum length of the attribute value=512
// for a hint see NPI_AddCharValue in AP.c
// for a hint see first half of AP_AddCharacteristic and first half of AP_AddNotifyCharacteristic
//****You implement this function as part of Lab 6*****
  uint8_t uuidB1, uuidB2;
	
  msg[0] = SOF;
  // length
  msg[1] = 0x08;
  msg[2] = 0x00;
  // SNP Add Characteristic Value Declaration
  msg[3] = 0x35;
  msg[4] = 0x82;
  // permission
  msg[5] = permission;
	//properties
  msg[6] = properties;
	msg[7] = 0x00;
  // rfu
  msg[8] = 0x00;
  // Maximum length of the attribute value=512
  msg[9] = 0x00;
  msg[10] = 0x02;
  // UUID
  uuidB1 = (uint8_t) (uuid & 0xFF);
  uuidB2 = (uint8_t) (uuid >> 8);
  msg[11] = uuidB1;
  msg[12] = uuidB2;
  SetFCS(msg);
}

//*************BuildAddCharDescriptorMsg**************
// Create a Add Characteristic Descriptor Declaration message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least 32 bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// for a hint see NPI_AddCharDescriptor in AP.c
// for a hint see second half of AP_AddCharacteristic
//****You implement this function as part of Lab 6*****
  uint8_t name_len, i, j;
	
	name_len = 0;
  for (i = 0; name[i] != 0; i++) {
    name_len++;
  }
  // add null terminator to the len
  name_len++;

  msg[0] = SOF;
  // length
  msg[1] = 6 + name_len;
  msg[2] = 0x00;
  // SNP Add Characteristic Descriptor Declaration
  msg[3] = 0x35;
  msg[4] = 0x83;
  // User Description String
  msg[5] = 0x80;
  // GATT Read Permissions
  msg[6] = 0x01;
  // Maximum Possible length of the user description string
  msg[7] = name_len;
  msg[8] = 0x00;
  // Initial Length of the user string
  msg[9] = name_len;
  msg[10] = 0;
  // Initial user description string
  for (i = 11, j = 0; i < name_len + 11; i++, j++) {
    msg[i] = name[j];
  }

  SetFCS(msg);
}

//*************Lab6_AddCharacteristic**************
// Add a read, write, or read/write characteristic, used in Lab 6
//        for notify properties, call AP_AddNotifyCharacteristic
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8
//        pt is a pointer to the user data, stored little endian
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*ReadFunc) called before it responses with data from internal structure
//        (*WriteFunc) called after it accepts data into internal structure
// Output APOK if successful,
//        APFAIL if name is empty, more than 8 characteristics, or if SNP failure
int Lab6_AddCharacteristic(uint16_t uuid, uint16_t thesize, void *pt, uint8_t permission,
  uint8_t properties, char name[], void(*ReadFunc)(void), void(*WriteFunc)(void)){
  int r; uint16_t handle;
  uint8_t sendMsg[32];
  if(thesize>8) return APFAIL;
  if(name[0]==0) return APFAIL;       // empty name
  if(CharacteristicCount>=MAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,permission,properties,sendMsg);
  OutString("\n\rAdd CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  CharacteristicList[CharacteristicCount].theHandle = handle;
  CharacteristicList[CharacteristicCount].size = thesize;
  CharacteristicList[CharacteristicCount].pt = (uint8_t *) pt;
  CharacteristicList[CharacteristicCount].callBackRead = ReadFunc;
  CharacteristicList[CharacteristicCount].callBackWrite = WriteFunc;
  CharacteristicCount++;
  return APOK; // OK
}


//*************BuildAddNotifyCharDescriptorMsg**************
// Create a Add Notify Characteristic Descriptor Declaration message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddNotifyCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// set User Description String
// set CCCD parameters read+write
// for a hint see NPI_AddCharDescriptor4 in VerySimpleApplicationProcessor.c
// for a hint see second half of AP_AddNotifyCharacteristic
//****You implement this function as part of Lab 6*****
  uint8_t name_len, i, j;

  name_len = 0;
  for (i = 0; name[i] != 0; i++) {
    name_len++;
  }
  // add null terminator to the len
  name_len++;

  msg[0] = SOF;
  // length
  msg[1] = 7 + name_len;
  msg[2] = 0x00;
  // SNP Add Characteristic Descriptor Declaration
  msg[3] = 0x35;
  msg[4] = 0x83;
  // User Description String
  msg[5] = 0x84;
  msg[6] = 0x03;
  // GATT Read Permissions
  msg[7] = 0x01;
  // Maximum Possible length of the user description string
  msg[8] = name_len;
  msg[9] = 0x00;
  // Initial Length of the user string
  msg[10] = name_len;
  msg[11] = 0;
  // Initial user description string
  for (i = 12, j = 0; i < name_len + 12; i++, j++) {
    msg[i] = name[j];
  }

  SetFCS(msg);
}

//*************Lab6_AddNotifyCharacteristic**************
// Add a notify characteristic, used in Lab 6
//        for read, write, or read/write characteristic, call AP_AddCharacteristic
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8
//        pt is a pointer to the user data, stored little endian
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*CCCDfunc) called after it accepts , changing CCCDvalue
// Output APOK if successful,
//        APFAIL if name is empty, more than 4 notify characteristics, or if SNP failure
int Lab6_AddNotifyCharacteristic(uint16_t uuid, uint16_t thesize, void *pt,
  char name[], void(*CCCDfunc)(void)){
  int r; uint16_t handle;
  uint8_t sendMsg[36];
  if(thesize>8) return APFAIL;
  if(NotifyCharacteristicCount>=NOTIFYMAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,0,0x10,sendMsg);
  OutString("\n\rAdd Notify CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddNotifyCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  NotifyCharacteristicList[NotifyCharacteristicCount].uuid = uuid;
  NotifyCharacteristicList[NotifyCharacteristicCount].theHandle = handle;
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDhandle = (RecvBuf[8]<<8)+RecvBuf[7]; // handle for this CCCD
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDvalue = 0; // notify initially off
  NotifyCharacteristicList[NotifyCharacteristicCount].size = thesize;
  NotifyCharacteristicList[NotifyCharacteristicCount].pt = (uint8_t *) pt;
  NotifyCharacteristicList[NotifyCharacteristicCount].callBackCCCD = CCCDfunc;
  NotifyCharacteristicCount++;
  return APOK; // OK
}

//*************BuildSetDeviceNameMsg**************
// Create a Set GATT Parameter message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message to set Device name
void BuildSetDeviceNameMsg(char name[], uint8_t *msg){
// for a hint see NPI_GATTSetDeviceNameMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_GATTSetDeviceName in AP.c
//****You implement this function as part of Lab 6*****
  uint8_t i, j;

  msg[0] = SOF;

	msg[2] = 0x00;
  // SNP Set GATT Parameter (0x8C)
  msg[3] = 0x35;
  msg[4] = 0x8C;
  // Generic Access Service
  msg[5] = 0x01;
  // Device Name
  msg[6] = 0x00;
  msg[7] = 0x00;

  for (i = 0, j = 8; name[i] != 0; i++, j++) {
    msg[j] = name[i];
  }
  msg[j] = name[i];
  // length
  msg[1] = i + 3;

  SetFCS(msg);
}
//*************BuildSetAdvertisementData1Msg**************
// Create a Set Advertisement Data message, used in Lab 6
// Inputs pointer to empty buffer of at least 16 bytes
// Output none
// build the necessary NPI message for Non-connectable Advertisement Data
void BuildSetAdvertisementData1Msg(uint8_t *msg){
// for a hint see NPI_SetAdvertisementMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_SetAdvertisement1 in AP.c
// Non-connectable Advertisement Data
// GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR
// Texas Instruments Company ID 0x000D
// TI_ST_DEVICE_ID = 3
// TI_ST_KEY_DATA_ID
// Key state=0
//****You implement this function as part of Lab 6*****
  
  msg[0] = SOF;
  // length
  msg[1] = 0x0B;
  msg[2] = 0x00;
  // SNP Set Advertisement Data
  msg[3] = 0x55;
  msg[4] = 0x43;
  // Not connected Advertisement Data
  msg[5] = 0x01;
  // GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR
  msg[6] = 0x02;
  msg[7] = 0x01;
  msg[8] = 0x06;
  // length, manufacturer specific
  msg[9] = 0x06;
  msg[10] = 0xFF;
  // Texas Instruments Company ID
  msg[11] = 0x0D;
  msg[12] = 0x00;
  // TI_ST_DEVICE_ID
  msg[13] = 0x03;
  // TI_ST_KEY_DATA_ID
  msg[14] = 0x00;
  // Key State
  msg[15] = 0x00;

  SetFCS(msg);
}

//*************BuildSetAdvertisementDataMsg**************
// Create a Set Advertisement Data message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message for Scan Response Data
void BuildSetAdvertisementDataMsg(char name[], uint8_t *msg){
// for a hint see NPI_SetAdvertisementDataMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_SetAdvertisementData in AP.c
//****You implement this function as part of Lab 6*****

  uint8_t len, i;
  for (len = 0; name[len] != 0; len++);

  msg[0] = SOF;
  msg[1] = 12 + len;
  msg[2] = 0x00;
  // SNP Set Advertisement Data
  msg[3] = 0x55;
  msg[4] = 0x43;
  // Scan Response Data
  msg[5] = 0x00;

  msg[6] = len + 1;
  msg[7] = 0x09;

  for (i = 0; i < len; i++) {
    msg[8 + i] = name[i];
  }

  // connection interval range
  msg[8 + len + 0] = 0x05; // length of this data
  msg[8 + len + 1] = 0x12; // GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE
  msg[8 + len + 2] = 0x50; // DEFAULT_DESIRED_MIN_CONN_INTERVAL
  msg[8 + len + 3] = 0x00; //
  msg[8 + len + 4] = 0x20; // DEFAULT_DESIRED_MAX_CONN_INTERVAL
  msg[8 + len + 5] = 0x03; //
  // Tx power level
  msg[8 + len + 6] = 0x02; // length of this data
  msg[8 + len + 7] = 0x0A; // GAP_ADTYPE_POWER_LEVEL
  msg[8 + len + 8] = 0x00; // 0dBm

  SetFCS(msg);
}
//*************BuildStartAdvertisementMsg**************
// Create a Start Advertisement Data message, used in Lab 6
// Inputs advertising interval
//        pointer to empty buffer of at least 20 bytes
// Output none
// build the necessary NPI message to start advertisement
void BuildStartAdvertisementMsg(uint16_t interval, uint8_t *msg){
// for a hint see NPI_StartAdvertisementMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_StartAdvertisement in AP.c
//****You implement this function as part of Lab 6*****
  uint8_t intervalB1, intervalB2;
  msg[0] = SOF;
  // length
  msg[1] = 0x0E;
  msg[2] = 0x00;
  // SNP Start Advertisement
  msg[3] = 0x55;
  msg[4] = 0x42;
  // Connectable Undirected Advertisements
  msg[5] = 0x00;
  // Advertise infinitely.
  msg[6] = 0x00;
  msg[7] = 0x00;
  // Advertising Interval (100 * 0.625 ms=62.5ms)
  intervalB1 = (uint8_t) (interval & 0xFF);
  intervalB2 = (uint8_t) (interval >> 8);
  msg[8] = intervalB1;
  msg[9] = intervalB2;
  // Filter Policy RFU
  msg[10] = 0x00;
  // Initiator Address Type RFU
  msg[11] = 0x00;
  // RFU
  msg[12] = 0x00;
  msg[13] = 0x01;
  msg[14] = 0x00;
  msg[15] = 0x00;
  msg[16] = 0x00;
  msg[17] = 0xC5;
  // Advertising will restart with connectable advertising when a connection is terminated
  msg[18] = 0x02;

  SetFCS(msg);
}

//*************Lab6_StartAdvertisement**************
// Start advertisement, used in Lab 6
// Input:  none
// Output: APOK if successful,
//         APFAIL if notification not configured, or if SNP failure
int Lab6_StartAdvertisement(void){volatile int r; uint8_t sendMsg[40];
  OutString("\n\rSet Device name");
  BuildSetDeviceNameMsg("Shape the World",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement1");
  BuildSetAdvertisementData1Msg(sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement Data");
  BuildSetAdvertisementDataMsg("Shape the World",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rStartAdvertisement");
  BuildStartAdvertisementMsg(100,sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}
