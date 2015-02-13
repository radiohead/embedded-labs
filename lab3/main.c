#include <p18cxxx.h>
#include "ports.h"

#define SETUP     0
#define RED       1
#define RED_AMBER 2
#define GREEN     3
#define AMBER     4

static unsigned int millis_passed;
static unsigned int state;

static void setup_board(void);
static void start_50ms_timer(void);
static void enter_sleep_state(void);
static void toggle_led(int, int);
static int  read_input(void);

/* Interrupt service routines */
static void high_isr(void);
static void high_vector(void);

void main(void)
{
	setup_board();
	start_50ms_timer();

    while(1) {
		enter_sleep_state();
	}
}

static void setup_board() {
	// Disable watchdog
	WDTCONbits.SWDTEN = 0;

	/* Set LED ports to output */
	TRISBbits.TRISB6 = 0;
	TRISCbits.TRISC1 = 0;
	TRISGbits.TRISG1 = 0;

	/* Set button ports to input */
	TRISBbits.TRISB0 = 1;
	TRISBbits.TRISB1 = 1;

	/* Set initial timer and state */
	millis_passed = 0;
	state = SETUP;
}

#pragma interrupt high_isr save=section(".tmpdata")
static void high_isr() {
	if (INTCONbits.TMR0IF) {
		millis_passed += 50;
	
		switch(state) {
			case SETUP:
				if (millis_passed >= 5000) {
					toggle_led(AMBER, 0);
					toggle_led(RED, 1);
	
					state = RED;
					millis_passed = 0;
				}
				else {
					if ((millis_passed % 500) == 0) {
						LATCbits.LATC1 ^= 1;
					}
				}
	
				break;
			case RED:
				if ((read_input() == 1) || millis_passed >= 5000) {
					toggle_led(AMBER, 1);
	
					state = RED_AMBER;
					millis_passed = 0;
				}

				break;
			case RED_AMBER:
				if (millis_passed >= 500) {
					toggle_led(AMBER, 0);
					toggle_led(RED, 0);
					toggle_led(GREEN, 1);
	
					state = GREEN;
					millis_passed = 0;
				}

				break;
			case GREEN:
				if (millis_passed >= 5000) {
					toggle_led(AMBER, 1);
					toggle_led(GREEN, 0);
	
					state = AMBER;
					millis_passed = 0;
				}

				break;
			case AMBER:
				if (millis_passed >= 500) {
					toggle_led(AMBER, 0);
					toggle_led(RED, 1);
	
					state = RED;
					millis_passed = 0;
				}

				break;
		}

		start_50ms_timer();
	}
}

static void toggle_led(int color, int state) {
	if (state != 0 && state != 1) {
		state = 0;
	}

	switch(color) {
		case RED:
			LATBbits.LATB6 = state;
			break;
		case AMBER:
			LATCbits.LATC1 = state;
			break;
		case GREEN:
			LATGbits.LATG1 = state;
			break;
		case RED_AMBER:
			LATBbits.LATB6 = state;
			LATCbits.LATC1 = state;
			break;
	}
}

static int read_input() {
	if (PORTBbits.RB0 || PORTBbits.RB1) {
		return 0;
	}
	else {
		return 1;
	}
}

void start_50ms_timer()
{
	// Preset timer values
	// For 50 milliseconds
	TMR0H = 0x0B;
	TMR0L = 0xDC;

	/* Enable timer interrup */
	INTCONbits.TMR0IE = 1;

	/* Clear overflow */
	INTCONbits.TMR0IF = 0;

	// Start the timer
	T0CON = 0x82;

	// Stop the timer
	// T0CON = 0x00;
}

void enter_sleep_state() {
	OSCCONbits.IDLEN = 1;
	_asm
		sleep
	_endasm
}

#pragma code high_vector=0x808
void high_vector() {
	_asm
		goto high_isr
	_endasm
}
#pragma code
