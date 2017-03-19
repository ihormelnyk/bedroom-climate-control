#include <os_type.h>
#include <osapi.h>
#include <user_interface.h>

#include "../lobaro-coap/coap.h"
#include "mq135.h"
#include "mq135_res.h"

//#include <sntp.h>
//ets_uart_printf("time %d\r\n", sntp_get_current_timestamp());

CoAP_Res_t* pMQ135_Res = NULL;

extern float mq135_co2;
extern float mq135_rzero;
extern float mq135_ppm;

extern float mq135_corr_ppm;
extern float dht22_temperature;
extern float dht22_humidity;

ICACHE_FLASH_ATTR bool Refresh_MQ135_Resource() {
	 uint32 adc_value = system_adc_read();
	 mq135_co2 = (float)adc_value;
	 //mq135_rzero = getRZero();
	 mq135_rzero = getCorrectedRZero(dht22_temperature, dht22_humidity);
	 mq135_ppm = getPPM();
	 mq135_corr_ppm = getCorrectedPPM(dht22_temperature, dht22_humidity);
	 return true;
}

static CoAP_HandlerResult_t ICACHE_FLASH_ATTR Res_ReqHandler(CoAP_Message_t* pReq, CoAP_Message_t* pResp) {
	if (mq135_co2 == 0) {
		if (!Refresh_MQ135_Resource()) return HANDLER_ERROR;
	}

	ets_uart_printf("CO2: %d ppm\r\n", (int)mq135_co2);

	char myString[10];
	coap_sprintf(myString,"%d", (int)mq135_corr_ppm);
	CoAP_SetPayload(pReq, pResp, myString, coap_strlen(myString), true);
	return HANDLER_OK;
}

CoAP_Res_t* ICACHE_FLASH_ATTR Create_MQ135_Resource() {
	//init
	mq135_co2 = 0;
	//resource
	CoAP_ResOpts_t Options = {.Cf = COAP_CF_TEXT_PLAIN, .Flags = RES_OPT_GET};
	return (pMQ135_Res=CoAP_CreateResource("co2/get", "bedroom co2",Options, Res_ReqHandler, NULL));
}
