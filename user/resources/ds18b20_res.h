#ifndef USER_RESOURCES_DS18B20_RES_H_
#define USER_RESOURCES_DS18B20_RES_H_

// Scratchpad locations
#define TEMP_LSB        0
#define TEMP_MSB        1
#define HIGH_ALARM_TEMP 2
#define LOW_ALARM_TEMP  3
#define CONFIGURATION   4
#define INTERNAL_BYTE   5
#define COUNT_REMAIN    6
#define COUNT_PER_C     7
#define SCRATCHPAD_CRC  8

// Device resolution
#define TEMP_9_BIT  0x1F //  9 bit
#define TEMP_10_BIT 0x3F // 10 bit
#define TEMP_11_BIT 0x5F // 11 bit
#define TEMP_12_BIT 0x7F // 12 bit

// Error Codes
#define DEVICE_DISCONNECTED_C -127
#define DEVICE_DISCONNECTED_F -196.6
#define DEVICE_DISCONNECTED_RAW -7040

CoAP_Res_t*  Create_DS18B20_Resource();
extern CoAP_Res_t* pDS18B20_Res;
bool Refresh_DS18B20_Resource();

#endif /* USER_RESOURCES_DS18B20_RES_H_ */
