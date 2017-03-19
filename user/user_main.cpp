#define TFT

#include "Adafruit_ILI9341_fast_as.h"
// =============================================================================================
// C includes and declarations
// =============================================================================================
#include "cpp_routines/routines.h"
#include <time.h>
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include <espconn.h>
#include <ip_addr.h>

extern "C"
{
#include "espmissingincludes.h"
#include "lobaro-coap/coap.h"
#include "lobaro-coap/interface/esp8266/lobaro-coap_esp8266.h"

#include "about_res.h"
#include "wifi_ip.h"
#include "wifi_cfg_res.h"
#include "wifi_scan_res.h"
#include "rtc_res.h"
#include "led_res.h"
#include "dht22_res.h"
#include "ds18b20_res.h"
#include "mq135_res.h"
#include "buttons.h"

// declare lib methods
extern int ets_uart_printf(const char *fmt, ...);
void ets_timer_disarm(ETSTimer *ptimer);
void ets_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg);
}

//internal ESP8266 Control Structures
esp_udp UdpCoAPConn;
struct espconn CoAP_conn;

//"Main Loop" Speed
#define DELAY_LOOP 10 // milliseconds
LOCAL os_timer_t MainLoop_timer;
#ifdef TFT
Adafruit_ILI9341 tft;
#endif

extern void updateScreen(bool mode);
extern void setupUI();

sint8 rssi = 0;
float dht22_temperature = 0;
float dht22_humidity = 0;
float ds18b20_temperature = 0;
float mq135_co2 = 0;
float mq135_rzero = 0;
float mq135_ppm = 0;
float mq135_corr_ppm = 0;

float target_temperature = 19;
float min_target_temp = 15;
float max_target_temp = 30;

bool change_temp_mode = false;
uint32 change_temp_mode_started = 0;

uint32 st_time = 0;
uint32 loop_time = 0;

unsigned long loop=0;

ICACHE_FLASH_ATTR static void updateScreen(void)
{
	if (change_temp_mode &&
			(((system_get_rtc_time() - change_temp_mode_started) / 100000) > 2))
	{
		change_temp_mode = false;
	}
	updateScreen(change_temp_mode);
}


ICACHE_FLASH_ATTR static void Refresh_RSII(void)
{
	rssi = wifi_station_get_rssi();
}

ICACHE_FLASH_ATTR static void refreshValues(void)
{
	Refresh_RSII(); //15ms
	Refresh_DHT22_Resource(); //450ms
	//Refresh_DS18B20_Resource();
	Refresh_MQ135_Resource(); //15ms
}


ICACHE_FLASH_ATTR void handler_task (os_event_t *e)
{
	loop_time = system_get_rtc_time() - st_time;
	st_time = system_get_rtc_time();
	switch (e->sig)
	{
		case UPDATE_SCREEN:
		{
			updateScreen(); //175ms
			break;
		}
		case COAP_DOWORK:
		{
			if(CoAP_ESP8266_States.TxSocketIdle) CoAP_doWork(); //1ms
			break;
		}
		case REFRESH_VALUES:
		{
			refreshValues(); //500ms
			break;
		}
		default: break;
	}
}

LOCAL void ICACHE_FLASH_ATTR mainLoopTimer_cb(void *arg)
{
	loop++;
	if (loop%20 == 0) {
#ifdef TFT
	system_os_post(USER_TASK_PRIO_0, UPDATE_SCREEN, 'a');
#endif
	}

	if (loop%100 == 0) {
			system_os_post(USER_TASK_PRIO_0, REFRESH_VALUES, 'a');
			return;
	}
	system_os_post(USER_TASK_PRIO_0, COAP_DOWORK, 'a');
}

extern "C" void ICACHE_FLASH_ATTR init_done(void) {
	static uint8_t CoAP_WorkMemory[4096]; //Working memory of CoAPs internal memory allocator
	CoAP_Init(CoAP_WorkMemory, 4096);

	CoAP_ESP8266_CreateInterfaceSocket(0, &CoAP_conn, 5683, CoAP_onNewPacketHandler, CoAP_ESP8266_SendDatagram);

	Create_Wifi_IPs_Resource(); 		//example of simple GET resource
	Create_About_Resource();			//example of large resource (blockwise transfers)
	Create_Wifi_Config_Resource(); 		//example of uri-query usage
	Create_RTC_Resource(); 				//example of observable resource
	Create_Led_Resource(); 				//example of observable resource triggered by itself + uri-query usage
	Create_Wifi_Scan_Resource(); 		//example of longer lasting "postponed" resource with two responses (1. empty ACK, 2. actual resource)
	//Create_DS18B20_Resource();
	Create_DHT22_Resource();
	Create_MQ135_Resource();

	ets_uart_printf("- CoAP init done! Used CoAP ram memory:\r\n"); //note: static ram footprint depends primary on resource count+uri lengths
	coap_mem_determinateStaticMem();
	coap_mem_stats();

	// Set up a "main-loop" timer
	// Set up a timer to send the message to handler
	os_timer_disarm(&MainLoop_timer);
	os_timer_setfn(&MainLoop_timer, (os_timer_func_t *)mainLoopTimer_cb, (void *)0);
	os_timer_arm(&MainLoop_timer, DELAY_LOOP, 1);

	// Set up a timerHandler to send the message to handler
	os_event_t *handlerQueue;
	handlerQueue = (os_event_t *)os_malloc(sizeof(os_event_t)*TEST_QUEUE_LEN);
	system_os_task(handler_task, USER_TASK_PRIO_0, handlerQueue, TEST_QUEUE_LEN);
}

extern "C" void ICACHE_FLASH_ATTR user_init(void) {
	CoAP_conn.proto.udp = &UdpCoAPConn;
	// Configure the UART
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	ets_uart_printf("\r\nSystem init...\r\n");
	do_global_ctors();
	ets_uart_printf("\r\nGlobal constructors invoked\r\n");
	InitButtons();
#ifdef TFT
	// Initialize TFT
	tft.begin();
	tft.fillScreen(0);
	setupUI();
#endif
	//Config ESP8266 network
	CoAP_ESP8266_ConfigDevice();
	system_init_done_cb(init_done);
	ets_uart_printf("System init done \r\n");
}
