#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "driver/gpio16.h"
#include "driver/buttons.h"

extern float target_temperature;
extern bool change_temp_mode;
extern uint32 change_temp_mode_started;

extern uint8_t pin_num[GPIO_PIN_NUM];

// GPIO_PIN_INTR_NEGEDGE - down
// GPIO_PIN_INTR_POSEDGE - up
// GPIO_PIN_INTR_ANYEDGE - both
// GPIO_PIN_INTR_LOLEVEL - low level
// GPIO_PIN_INTR_HILEVEL - high level
// GPIO_PIN_INTR_DISABLE - disable interrupt
const char *gpio_type_desc[] =
{
	    "GPIO_PIN_INTR_DISABLE (DISABLE INTERRUPT)",
	    "GPIO_PIN_INTR_POSEDGE (UP)",
	    "GPIO_PIN_INTR_NEGEDGE (DOWN)",
	    "GPIO_PIN_INTR_ANYEDGE (BOTH)",
	    "GPIO_PIN_INTR_LOLEVEL (LOW LEVEL)",
	    "GPIO_PIN_INTR_HILEVEL (HIGH LEVEL)"
};


extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;

void ICACHE_FLASH_ATTR intr_callback(unsigned pin, unsigned level)
{
	console_printf("INTERRUPT: GPIO%d = %d\r\n", pin_num[pin], level);
	if (change_temp_mode) {
		if (pin == 1) {
			if (target_temperature > 5) {
				target_temperature -= 0.5;
			}
		}
		else if (pin == 2) {
			if (target_temperature < 30) {
				target_temperature += 0.5;
			}
		}
	}

	change_temp_mode = true;
	change_temp_mode_started = system_get_rtc_time();
}

ICACHE_FLASH_ATTR void ICACHE_FLASH_ATTR InitButtons(void)
{
	GPIO_INT_TYPE gpio_type;
	uint8_t gpio_pin;

	// Pin number 1 = GPIO5
	gpio_pin = 1;
	gpio_type = GPIO_PIN_INTR_POSEDGE;
	if (set_gpio_mode(gpio_pin, GPIO_INPUT, GPIO_PULLUP)) {
		console_printf("GPIO%d set interrupt mode\r\n", pin_num[gpio_pin]);
		if (gpio_intr_init(gpio_pin, gpio_type)) {
			console_printf("GPIO%d enable %s mode\r\n", pin_num[gpio_pin], gpio_type_desc[gpio_type]);
			gpio_intr_attach(intr_callback);
		} else {
			console_printf("Error: GPIO%d not enable %s mode\r\n", pin_num[gpio_pin], gpio_type_desc[gpio_type]);
		}
	} else {
		console_printf("Error: GPIO%d not set interrupt mode\r\n", pin_num[gpio_pin]);
	}

	// Pin number 2 = GPIO4
	gpio_pin = 2;
	gpio_type = GPIO_PIN_INTR_POSEDGE;
	if (set_gpio_mode(gpio_pin, GPIO_INPUT, GPIO_PULLUP)) {
		console_printf("GPIO%d set interrupt mode\r\n", pin_num[gpio_pin]);
		if (gpio_intr_init(gpio_pin, gpio_type)) {
			console_printf("GPIO%d enable %s mode\r\n", pin_num[gpio_pin], gpio_type_desc[gpio_type]);
			gpio_intr_attach(intr_callback);
		} else {
			console_printf("Error: GPIO%d not enable %s mode\r\n", pin_num[gpio_pin], gpio_type_desc[gpio_type]);
		}
	} else {
		console_printf("Error: GPIO%d not set interrupt mode\r\n", pin_num[gpio_pin]);
	}
}

