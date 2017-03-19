#include <os_type.h>
#include <osapi.h>
#include <user_interface.h>

#include "../lobaro-coap/coap.h"
#include "dht22_res.h"
#include "driver/dht22.h"
#include "driver/gpio16.h"

CoAP_Res_t* pDHT22_Res = NULL;

extern uint8_t pin_num[GPIO_PIN_NUM];
DHT_Sensor sensor;

extern float dht22_temperature;
extern float dht22_humidity;

bool Refresh_DHT22_Resource() {
	DHT_Sensor_Data data;
	if(DHTRead(&sensor, &data))
	{
		dht22_temperature = data.temperature;
		dht22_humidity = data.humidity;
		return true;
	}
	return false;
}

static CoAP_HandlerResult_t ICACHE_FLASH_ATTR Res_ReqHandler(CoAP_Message_t* pReq, CoAP_Message_t* pResp) {
	if ((dht22_temperature == 0) && (dht22_humidity == 0)) {
		if (!Refresh_DHT22_Resource()) {
			ets_uart_printf("Failed to read temperature and humidity sensor on GPIO%d\n", pin_num[sensor.pin]);
			return HANDLER_ERROR;
		}
	}

	char buff[20];
	ets_uart_printf("GPIO%d\r\n", pin_num[sensor.pin]);
	ets_uart_printf("Temperature: %s *C\r\n", DHTFloat2String(buff, dht22_temperature));
	ets_uart_printf("Humidity: %s %%\r\n", DHTFloat2String(buff, dht22_humidity));

	char myString[30];
	char* pStrWorking = myString;
	pStrWorking+= coap_sprintf(pStrWorking,"t=%s *C", DHTFloat2String(buff, dht22_temperature));
	pStrWorking+= coap_sprintf(pStrWorking,", h=%s %%", DHTFloat2String(buff, dht22_humidity));
	CoAP_SetPayload(pReq, pResp, myString, coap_strlen(myString), true);
	return HANDLER_OK;
}

CoAP_Res_t* ICACHE_FLASH_ATTR Create_DHT22_Resource() {
	dht22_temperature = 0;
	dht22_humidity = 0;
	//Pin number 3 = GPIO0
	sensor.pin = 3;
	sensor.type = DHT22;
	ets_uart_printf("DHT22 init on GPIO%d\r\n", pin_num[sensor.pin]);
	DHTInit(&sensor);
	//resource
	CoAP_ResOpts_t Options = {.Cf = COAP_CF_TEXT_PLAIN, .Flags = RES_OPT_GET};
	return (pDHT22_Res=CoAP_CreateResource("humidity/get", "bedroom humidity",Options, Res_ReqHandler, NULL));
}
