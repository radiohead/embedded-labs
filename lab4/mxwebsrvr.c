#include <p18cxxx.h>
#include "ports.h"

#define MAX_TASKS 5

typedef struct {
  void (*callback)(void);

  long int delay;
  unsigned int interval;
  
  unsigned short int run_times;
} scheduler_task;

static scheduler_task scheduled_tasks[MAX_TASKS];
static unsigned int millis_passed;
static unsigned short int button_pressed;
static long int temperature_data;

/* Setup/services */
static void high_vector(void);
static void setup_board(void);

/* Scheduler */
static void setup_scheduler(void);
static void scheduler_add_task(void (*)(void), unsigned int, unsigned int);
static void scheduler_delete_task(unsigned short int);
static void scheduler_dispatch_tasks(void);
static void scheduler_update(void);

/* Utilities */
static void start_50ms_timer(void);
static void enter_sleep_state(void);

/* Tasks */
void read_temperature(void);
void toggle_led(void);
void check_buttons(void);

void main(void)
{
	setup_board();
	setup_scheduler();

  scheduler_add_task(&read_temperature, 0, 100);
  scheduler_add_task(&check_buttons, 50, 500);
  scheduler_add_task(&toggle_led, 100, 50);

  start_50ms_timer();

  while(1) {
    scheduler_dispatch_tasks();
	}
}

static void setup_board() {
	/* Disable watchdog */
	WDTCONbits.SWDTEN = 0;

	/* Set LED ports to output */
	TRISCbits.TRISC1 = 0;

	/* Set button ports to input */
	TRISBbits.TRISB0 = 1;
	TRISBbits.TRISB1 = 1;

  /* Initialize analog ports */
	ADCON0 = 0x4;
	ADCON1 = 0x0;
	ADCON2 = 0x7;

	/* Set initial timer and state */
	millis_passed = 0;
	button_pressed = 0;
}

static void setup_scheduler(void) {
  unsigned short int i;
  
  for (i = 0; i < MAX_TASKS; ++i) {
    scheduler_delete_task(i);
  }
}

static void start_50ms_timer(void) {
  /*
    Preset timer values
	  for 50 milliseconds 
	*/
	TMR0H = 0x0B;
	TMR0L = 0xDC;

	/* Enable timer interrup */
	INTCONbits.TMR0IE = 1;

	/* Clear overflow */
	INTCONbits.TMR0IF = 0;

  /* Start the timer */
	T0CON = 0x82;
}

static void scheduler_delete_task(unsigned short int index) {
  if (index >= MAX_TASKS) {
    return;
  }

  scheduled_tasks[index].callback = 0;
  scheduled_tasks[index].delay = 0;
  scheduled_tasks[index].interval = 0;
  scheduled_tasks[index].run_times = 0;
}

static void scheduler_add_task(void (*callback)(), unsigned int delay, unsigned int interval) {
  unsigned short int index = 0;

  /* Find available space to fit the task */
  while ((scheduled_tasks[index].callback != 0) && (index < MAX_TASKS)) {
    ++index;
  }

  /* Cannot add any more tasks */
  if (index == MAX_TASKS) return;

  scheduled_tasks[index].callback = callback;
  scheduled_tasks[index].delay = delay;
  scheduled_tasks[index].interval = interval;

  scheduled_tasks[index].run_times = 0;
}

static void scheduler_dispatch_tasks() {
  unsigned short int index;

  for (index = 0; index < MAX_TASKS; ++index) {
    if (scheduled_tasks[index].run_times > 0) {
      (*scheduled_tasks[index].callback)();
      --scheduled_tasks[index].run_times;

      if (scheduled_tasks[index].interval == 0) {
        scheduler_delete_task(index);
      }
    }
  }

  enter_sleep_state();
}

#pragma interrupt scheduler_update save=section(".tmpdata")
static void scheduler_update() {
  unsigned short int index;

	if (INTCONbits.TMR0IF) {
		millis_passed += 50;

    for (index = 0; index < MAX_TASKS; ++index) {
      if (scheduled_tasks[index].callback != 0) {
        scheduled_tasks[index].delay -= 50;

        if (scheduled_tasks[index].delay <= 0) {
          ++scheduled_tasks[index].run_times;
          
          if (scheduled_tasks[index].interval != 0) {
            scheduled_tasks[index].delay = scheduled_tasks[index].interval;
          }
        }
      }
    }

		start_50ms_timer();
  }
}

void read_temperature() {
  /* Disable interrupt, clear overflow */
  PIR1bits.ADIF = 0;
  PIE1bits.ADIE = 0;

  /* Start conversion */
  ADCON0 = 0x7;

  /* Conversion in progress */
  while (ADCON0 == 0x7);

  /* Read the data */
  temperature_data = (ADRESH << 4) + ADRESL;
}

void toggle_led() {
  unsigned short int index;

  if (button_pressed != 1) {
    LATCbits.LATC1 ^= 1;
  }
  else {
    LATCbits.LATC1 = 0;
  }

  for (index = 0; index < MAX_TASKS; ++index) {
    if ((*scheduled_tasks[index].callback) == &toggle_led) {
      scheduled_tasks[index].delay = temperature_data;
    }
  }
}

void check_buttons() {
	if (PORTBbits.RB0 || PORTBbits.RB1) {
		button_pressed = 0;
	}
	else {
		button_pressed = 1;
	}
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
		goto scheduler_update
	_endasm
}
#pragma code
