#include <FreeRTOS.h>
#include <task.h>

#include "Drivers/interrupts.h"
#include "Drivers/gpio.h"

#define LED_PORT   16
#define BUTTON_PORT 9

static int button_pressed_times = 0;
static int led_blinked = 1;

void task2(void *pParam) {
	int i;
	while(1) {
		if (led_blinked == 0) {
			for (i = 0; i < button_pressed_times; ++i) {
				SetGpio(LED_PORT, 0);
				vTaskDelay(200);
				SetGpio(LED_PORT, 1);
				vTaskDelay(200);
			}

			led_blinked = 1;
		}
	}
}

void task1(void *pParam) {
	while(1) {
		if (ReadGpio(BUTTON_PORT) == 0 && led_blinked == 1) {
			++button_pressed_times;
			led_blinked = 0;
		}
	}
}

/**
 *	This is the systems main entry, some call it a boot thread.
 *
 *	-- Absolutely nothing wrong with this being called main(), just it doesn't have
 *	-- the same prototype as you'd see in a linux program.
 **/
int main(void) {

	DisableInterrupts();
	InitInterruptController();

	SetGpioFunction(LED_PORT, 1);
	SetGpioFunction(BUTTON_PORT, 0);

	xTaskCreate(task1, "BUTTON", 128, NULL, 0, NULL);
	xTaskCreate(task2, "LED_1", 128, NULL, 0, NULL);

	vTaskStartScheduler();

	/*
	 *	We should never get here, but just in case something goes wrong,
	 *	we'll place the CPU into a safe loop.
	 */
	while(1) {
		;
	}

	return 0;
}
