#include <p18cxxx.h>
#include "ports.h"

static void setup_board(void);
static void start_blocking_timer(int);
static void toggle_led(void);
static int  read_input(void);

void main(void)
{
	setup_board();

    while(1) {
		if (read_input() == 0) {
			toggle_led();
			start_blocking_timer(500);
		}
	}
}

static void setup_board() {
	// Disable watchdog
	WDTCONbits.SWDTEN = 0;

	// Set LED port to output
	TRISBbits.TRISB6 = 0;

	// Set button port to input
	TRISBbits.TRISB0 = 1;
	TRISBbits.TRISB1 = 1;
}

static void toggle_led() {
	LATBbits.LATB6 ^= 1;
}

static int read_input() {
	if (PORTBbits.RB0 || PORTBbits.RB1) {
		return 0;
	}
	else {
		return 1;
	}
}

static void start_blocking_timer(int millis)
{
	unsigned int counter;
	unsigned int threshold = (millis / 50);

	for (counter = 0; counter < threshold; ++counter) {
		// Preset timer values
		// For 50 milliseconds
		TMR0H = 0x0B;
		TMR0L = 0xDC;
	
		// Clear overflow
		INTCONbits.TMR0IE = 0;
		INTCONbits.TMR0IF = 0;

		// Start the timer
		T0CON = 0x82;

		while (INTCONbits.TMR0IF == 0);

		// Stop the timer
		T0CON = 0x00;
	}
}
