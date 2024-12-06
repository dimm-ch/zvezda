//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: MSG_NON_VITAL_DATA
//
// MessageText:
//
//  <Init> Undefined vital data into init source
//
#define MSG_NON_VITAL_DATA               0xC0000001L

//
// MessageId: MSG_ADM_PLD
//
// MessageText:
//
//  ADM PLD 
//
#define MSG_ADM_PLD                      0x00000002L

//
// MessageId: MSG_BASE_ICR
//
// MessageText:
//
//  ID & CFG EEPROM on Base module
//
#define MSG_BASE_ICR                     0x00000003L

//
// MessageId: MSG_ADM_ICR
//
// MessageText:
//
//  ID & CFG EEPROM on subunit
//
#define MSG_ADM_ICR                      0xC0000004L

//
// MessageId: MSG_NON_DEVICE_DRIVER
//
// MessageText:
//
//  <Init> No device driver for
//
#define MSG_NON_DEVICE_DRIVER            0x80000005L

//
// MessageId: MSG_DSP_PLD
//
// MessageText:
//
//  DSP PLD 
//
#define MSG_DSP_PLD                      0x80000006L

