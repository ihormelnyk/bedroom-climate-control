#include <Adafruit_GFX_AS.h>
#include <Adafruit_ILI9341_fast_as.h>
#include <time.h>
#include <c_types.h>
#include <user_interface.h>

extern "C" {
#include "mini-printf.h"
}

//320x240

//#define TARGETTEMPSCREEN

#define VGA_BLACK		0x0000
#define VGA_WHITE		0xFFFF
#define VGA_RED			0xF800
#define VGA_GREEN		0x0400
#define VGA_BLUE		0x001F
#define VGA_SILVER		0xC618
#define VGA_GRAY		0x8410
#define VGA_MAROON		0x8000
#define VGA_YELLOW		0xFFE0
#define VGA_OLIVE		0x8400
#define VGA_LIME		0x07E0
#define VGA_AQUA		0x07FF
#define VGA_TEAL		0x0410
#define VGA_NAVY		0x0010
#define VGA_FUCHSIA		0xF81F
#define VGA_PURPLE		0x8010


extern Adafruit_ILI9341 tft;

extern sint8 rssi;
extern float dht22_temperature;
extern float dht22_humidity;
extern float ds18b20_temperature;
extern float mq135_co2;
extern float mq135_rzero;
extern float mq135_ppm;
extern float mq135_corr_ppm;

extern float target_temperature;
extern float min_target_temp;
extern float max_target_temp;
time_t start_time;

extern uint32 loop_time;

#define HEADERTEXT 2
#define LINETEXT 2

bool currentChangeTempMode = false;

ICACHE_FLASH_ATTR int color(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r&248)|g>>5) << 8 | ((g&28)<<3|b>>3);
}

ICACHE_FLASH_ATTR int drawPlaceholder(int x, int y, int width, int height, int bordercolor, const char* headertext)
{
	int headersize = 18;
	tft.drawRoundRect(x, y, width, height, 3, bordercolor);
	tft.drawCentreString(headertext, x + width / 2, y + 1, HEADERTEXT);
	tft.drawFastHLine(x, y + headersize, width, bordercolor);
	
	return y + headersize;
}

ICACHE_FLASH_ATTR void drawWireFrame()
{
	tft.setTextColor(VGA_SILVER, VGA_BLACK);
}


ICACHE_FLASH_ATTR  time_t now()
{
	return system_get_rtc_time() / 100000 - start_time;
}

ICACHE_FLASH_ATTR  void drawUpdated()
{
	//tft.fillRect(200, 5, 100, 16, VGA_BLACK);
	//tft.drawNumber(loop_time, 200, 5, 2);

	tft.setTextColor(VGA_SILVER, VGA_BLACK);
	time_t cursecs = now();

	char buf[20];
	unsigned int days = cursecs / 86400ul;
	unsigned int hours = (cursecs - days * 86400ul) / 3600;
	unsigned int mins = (cursecs - days * 86400ul - hours * 3600ul) / 60ul;
	snprintf(buf, sizeof(buf), "%ud %02uh %02um", days, hours, mins);
	tft.drawString(buf, 200, 222, LINETEXT);
}


ICACHE_FLASH_ATTR void drawRSSI()
{
	tft.setTextColor(VGA_AQUA, VGA_BLACK);
	tft.fillRect(250, 2, 48, 16, VGA_BLACK);
	tft.drawNumber(rssi, 250, 2, 2);
}

ICACHE_FLASH_ATTR  void drawDht22Temperature()
{
	uint8_t colorvalue = (dht22_temperature - min_target_temp) / (max_target_temp - min_target_temp) * 255;
	int _color = color(colorvalue, (colorvalue<127 ? 255 : (255 - colorvalue) * 2), 255 - colorvalue);
	tft.setTextColor(_color, VGA_BLACK);
	tft.drawFloat(dht22_temperature, 1, 80, 40, 7);
	tft.drawString("C", 200, 40, 4);
}

ICACHE_FLASH_ATTR  void drawDht22Humidity()
{
	tft.setTextColor(VGA_RED, VGA_BLACK);
	if ((dht22_humidity > 0) && (dht22_humidity < 99.5)) {
		tft.drawFloat(dht22_humidity, 0, 110, 110, 7);
		tft.drawString("%", 180, 140, 4);
	}
}

ICACHE_FLASH_ATTR void drawDs18b20Temperature()
{
	tft.setTextColor(VGA_AQUA, VGA_BLACK);
	tft.fillRect(240, 10, 48, 16, VGA_BLACK);
	tft.drawFloat(ds18b20_temperature, 1, 240, 10, 2);
}

ICACHE_FLASH_ATTR void drawCO2Level()
{
	float min_co2 = 400;
	float max_co2 = 1000;

	uint8_t barHeight = 8;
	uint8_t barSpace = 4;
	uint8_t initialBarWidth = 8;
	int numBars = (tft.height() - 16) / (barHeight + barSpace);
	for (int i = 0; i < numBars; i++)
	{
		float barTemp = min_co2 + i * ((max_co2 - min_co2) / numBars);
		int y = i * (barHeight + barSpace);
		int width = initialBarWidth;
		uint8_t colorvalue = float(i) / numBars * 255;
		int _color = color(colorvalue, 0, 255 - colorvalue);
		if (barTemp <= mq135_corr_ppm)
		{
			tft.fillRoundRect(0, tft.height() - y - barHeight, width, barHeight, 3, _color);
		}
		else
		{
			tft.fillRoundRect(1, tft.height() - y - barHeight + 1, width - 2, barHeight - 2, 3, VGA_BLACK);
			tft.drawRoundRect(0, tft.height() - y - barHeight, width, barHeight, 3, _color);
		}
	}
	//uint8_t colorvalue = (target_temperature - min_target_temp) / (max_target_temp - min_target_temp) * 255;
	//int _color = color(colorvalue, 0, 255 - colorvalue);

	tft.drawString("CO2", 2, 2, LINETEXT);
	tft.fillRect(40, 2, 48, 16, VGA_BLACK);
	tft.drawFloat(mq135_ppm, 1, 40, 2, LINETEXT);

	tft.fillRect(40, 20, 48, 16, VGA_BLACK);
	tft.drawFloat(mq135_corr_ppm, 1, 40, 20, LINETEXT);

	//if (currentChangeTempMode) {
		//tft.drawString("CO2A", 45, 2, LINETEXT);
		tft.fillRect(100, 2, 48, 16, VGA_BLACK);
		tft.drawFloat(mq135_co2, 1, 100, 2, LINETEXT);

		//tft.drawString("rzero", 110, 2, LINETEXT);
		tft.fillRect(155, 2, 48, 16, VGA_BLACK);
		tft.drawFloat(mq135_rzero, 1, 155, 2, LINETEXT);
	//}
}

ICACHE_FLASH_ATTR void drawTargetTempScreen()
{
	uint8_t barHeight = 8;
	uint8_t barSpace = 4;
	uint8_t initialBarWidth = 8;
	int numBars = tft.height() / (barHeight + barSpace);
	for (int i = 0; i < numBars; i++)
	{
		float barTemp = min_target_temp + i * ((max_target_temp - min_target_temp) / numBars);
		int y = i * (barHeight + barSpace);
		int width = initialBarWidth + 1.0f / 1404 * y * y + 0.1624 * y;
		uint8_t colorvalue = float(i) / numBars * 255;
		int _color = color(colorvalue, 0, 255 - colorvalue);
		if (barTemp <= target_temperature)
		{
			tft.fillRoundRect(0, tft.height() - y - barHeight, width, barHeight, 3, _color);
		}
		else
		{
			tft.fillRoundRect(1, tft.height() - y - barHeight + 1, width - 2, barHeight - 2, 3, VGA_BLACK);
			tft.drawRoundRect(0, tft.height() - y - barHeight, width, barHeight, 3, _color);
		}
	}
	uint8_t colorvalue = (target_temperature - min_target_temp) / (max_target_temp - min_target_temp) * 255;
	int _color = color(colorvalue, 0, 255 - colorvalue);
	tft.setTextColor(_color, VGA_BLACK);
	tft.drawFloat(target_temperature, 1, tft.width() / 2 - 64, tft.height() / 2 - 32, 7);
}


ICACHE_FLASH_ATTR void updateScreen(bool changeTempMode)
{
	if (currentChangeTempMode != changeTempMode)
	{
		currentChangeTempMode = changeTempMode;
		tft.fillScreen(VGA_BLACK);
	}
	if (changeTempMode)
	{
		drawTargetTempScreen();
	}
	else
	{
		drawCO2Level(); //20ms
		drawRSSI();
		drawDht22Temperature();
		drawDht22Humidity();
		//drawDs18b20Temperature();
		drawUpdated();
	}
}

ICACHE_FLASH_ATTR void setupUI()
{
	start_time = system_get_rtc_time() / 100000;
	tft.setRotation(3);
	tft.setTextColor(VGA_GREEN, VGA_BLACK);
	tft.fillScreen(ILI9341_BLACK);
	drawWireFrame();
}
