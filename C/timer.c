#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "timer.h"

// The number of times timer 0 has overflowed since the program started.
// Must be volatile or gcc will optimize away some uses of it.
volatile uint32_t timer0_overflow_count;

SIGNAL(SIG_OVERFLOW0)
{
	timer0_overflow_count++;
}

uint32_t millis(void)
{
	// timer 0 increments every 64 cycles, and overflows when it reaches
	// 256.  we would calculate the total number of clock cycles, then
	// divide by the number of clock cycles per millisecond, but this
	// overflows too often.
	//return timer0_overflow_count * 64UL * 256UL / (F_CPU / 1000UL);
	
	// instead find 1/128th the number of clock cycles and divide by
	// 1/128th the number of clock cycles per millisecond
	return timer0_overflow_count * 64UL * 2UL / (F_CPU / 128000UL);
}

void delay(uint32_t ms) {
	uint32_t start = millis();
	
	while (millis() - start < ms)
		;
}

/* Ripped from wiring.c of arduino-0009
 * Delay for the given number of microseconds.  Assumes a 16 MHz clock. 
 * Disables interrupts, which will disrupt the millis() function if used
 * too frequently. */
void delayMicros(uint32_t us)
{
	uint8_t oldSREG;

	// calling avrlib's delay_us() function with low values (e.g. 1 or
	// 2 microseconds) gives delays longer than desired.
	//delay_us(us);

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call yields a delay of approximately 1 1/8 us.
	if (--us == 0)
		return;

	// the following loop takes a quarter of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2;

	// account for the time taken in the preceeding commands.
	us -= 2;

	// disable interrupts, otherwise the timer 0 overflow interrupt that
	// tracks milliseconds will make us delay longer than we want.
	oldSREG = SREG;
	cli();

	// busy wait
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);

	// reenable interrupts.
	SREG = oldSREG;
}


void TimerInit(void) {
 	timer0_overflow_count = 0;
	// on the ATmega168, timer 0 is also used for fast hardware pwm
	// (using phase-correct PWM would mean that timer 0 overflowed half as often
	// resulting in different millis() behavior on the ATmega8 and ATmega168)
//	sbi(TCCR0A, WGM01);
//	sbi(TCCR0A, WGM00);
	// set timer 0 prescale factor to 64
//	sbi(TCCR0B, CS01);
//	sbi(TCCR0B, CS00);
	// enable timer 0 overflow interrupt
//	sbi(TIMSK0, TOIE0);

#if 0
	 //Timer0 Settings: Timer Prescaler /64, 
  TCCR0B &= ~(1<<CS02); 
  TCCR0B |= ((1<<CS01) | (1<<CS00));
  // Use normal mode, do not use the OC0B and OC0A Pins.
  TCCR0A |= ~((1<<COM0B1) | (1<<COM0B0) | (1<<COM0A1) | (1<<COM0A0) | (1<<WGM01) | (1<<WGM00));   
  TCCR0B &= ~(1<<WGM02);                 
  TIMSK0 |= (1<<TOIE0) | (0<<OCIE0A);        //Timer0 Overflow Interrupt Enable  
#endif
}

