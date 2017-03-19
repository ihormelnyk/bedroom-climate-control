#include <os_type.h>
#include <osapi.h>
#include <user_interface.h>

#include "../lobaro-coap/coap.h"
#include "ds18b20_res.h"
#include "driver/ds18b20.h"

CoAP_Res_t* pDS18B20_Res = NULL;
char addr[8] = {0x28, 0xff, 0xbf, 0x2d, 0xa3, 0x15, 0x01, 0xda};

#define sleepms(x) os_delay_us(x*1000);

extern float ds18b20_temperature;

ICACHE_FLASH_ATTR bool HasAddresss() {
	if (addr[0] != 0) return true;

	int r = ds_search(addr);
	if(r)
	{
		ets_uart_printf("Found Device @ %02x %02x %02x %02x %02x %02x %02x %02x\r\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
		if(crc8(addr, 7) != addr[7])
			ets_uart_printf( "CRC mismatch, crc=%xd, addr[7]=%xd\r\n", crc8(addr, 7), addr[7]);

		switch(addr[0])
		{
		case 0x10:
			ets_uart_printf("Device is DS18S20 family.\r\n");
			break;

		case 0x28:
			ets_uart_printf("Device is DS18B20 family.\r\n");
			break;

		default:
			ets_uart_printf("Device is unknown family.\r\n");
			return false;
		}
	}
	else {
		ets_uart_printf("No DS18B20 detected, sorry.\r\n");
		return false;
	}
	return true;
}

ICACHE_FLASH_ATTR char* Float2String(char* buffer, float value)
{
  os_sprintf(buffer, "%d.%d", (int)(value),(int)((value - (int)value)*100));
  return buffer;
}

ICACHE_FLASH_ATTR bool Refresh_DS18B20_Resource() {
	if (!HasAddresss()) return false;
	int i;
	uint8_t data[12];

	// perform the conversion
	reset();
	select(addr);

	write(DS1820_CONVERT_T, 1); // perform temperature conversion

	sleepms(750); // sleep 750ms

	//ets_uart_printf("Scratchpad: ");
	reset();
	select(addr);
	write(DS1820_READ_SCRATCHPAD, 0); // read scratchpad

	for(i = 0; i < 9; i++)
	{
		data[i] = read();
		//ets_uart_printf("%2x ", data[i]);
	}
	//ets_uart_printf("\r\n");

	int16_t raw =
	    (((int16_t) data[TEMP_MSB]) << 11) |
	    (((int16_t) data[TEMP_LSB]) << 3);

	 if (raw <= DEVICE_DISCONNECTED_RAW)
	     return false;
	 // C = RAW/128
	 ds18b20_temperature = (float)raw * 0.0078125;
	 return true;
}

static CoAP_HandlerResult_t ICACHE_FLASH_ATTR Res_ReqHandler(CoAP_Message_t* pReq, CoAP_Message_t* pResp) {
	if (ds18b20_temperature == 0) {
		if (!Refresh_DS18B20_Resource()) return HANDLER_ERROR;
	}

	char myString[10];
	Float2String(myString, ds18b20_temperature);
	ets_uart_printf("Temperature: %s C*\r\n", myString);

	CoAP_SetPayload(pReq, pResp, myString, coap_strlen(myString), true);
	return HANDLER_OK;
}

CoAP_Res_t* ICACHE_FLASH_ATTR Create_DS18B20_Resource() {
	//init
	ds18b20_temperature = 0;
	ds_init();
	//resource
	CoAP_ResOpts_t Options = {.Cf = COAP_CF_TEXT_PLAIN, .Flags = RES_OPT_GET};
	return (pDS18B20_Res=CoAP_CreateResource("temp/get", "bedroom temperature",Options, Res_ReqHandler, NULL));
}
