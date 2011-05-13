/*
 * Hello.c
 *
 * Created: 30.04.2011 22:46:38
 *  Author: tobias
 */ 
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include "lib_dmx_in.h"
#include "taster.h"

uint8_t address_persist EEMEM; // standart adresse ist 255. Bei jedem flashen wird diese überschrieben

taster_t program_taster;
uint8_t programing_mode = 0;
uint8_t programing_switch_counter;
uint8_t programing_switch_counter_start = 0;
uint8_t programing_switch_counter_ov;
uint8_t blink_counter = 0;

enum {DIGIT_ONE_OFF, DIGIT_ONE_ON, DIGIT_TWO_OFF, DIGIT_TWO_ON, BREAK_ON, BREAK_OFF};

uint8_t deviceno;
uint8_t blink_state = BREAK_ON;
uint8_t blink_digit;
uint8_t blink_digit_count = 0;

#define PROGRAMMING_SWITCH_COUNTER_TICKS 31
#define BLINK_COUNTER_TICKS 20

void setRGB(int red, int green, int blue) {	
		OCR1AL = 0xFF - red;
		OCR1BL = 0xFF - green;
		OCR2   = 0xFF - blue;
}

void initPWM() {
	DDRB  = 0xff;                  // use all pins on PortB for output 
    PORTB = 0xff;                  // set output high -> turn all LEDs off
	// set OC1A pin as output, required for output toggling
    DDRB |= _BV(PB1); // red
	DDRB |= _BV(PB2); // green
	DDRB |= _BV(PB3); // blue
	
	/* PWM-Quelle: http://www.mikrocontroller.net/topic/158322 */
	
	// Init Pins
	PORTB &=~ ((1 << PB1)  | (1 << PB2)  | (1 << PB3));  
	DDRB  |=  ((1 << DDB1) | (1 << DDB2) | (1 << DDB3));  
	
	// Init PWM Systems
	TCCR1A |= (1 << COM1A1) | (1 << COM1A0) | (1 << COM1B1) | (1 << COM1B0) | (1 << WGM10);
	TCCR1B |= (1 << CS10);
	TCCR2  |= (1 << COM21) | (1 << COM20) | (1 << WGM20) | (1 << CS20);
}

void programming_switch_pressed() {
	programing_switch_counter = 0;
	programing_switch_counter_ov = 0;
	programing_switch_counter_start = 1;
}

void programming_switch_realeased() {
	if ((!programing_switch_counter_ov) && programing_mode)
	{
		deviceno++;
		deviceno = deviceno % 64;
		blink_state = BREAK_OFF;
		blink_counter = 0;
	}
	
	programing_switch_counter_start = 0;
	programing_switch_counter_ov = 0;
}

void init_Int0() {
	DDRD &= ~(1<<DDD2);
	PORTD |= (1<<PD2);
	
	taster_init(&program_taster, &PIND, PIND2, programming_switch_pressed, programming_switch_realeased);
}

void start_Timer0() {
	TCNT0 = 0x00;
	TCCR0 = (1<<CS02) | (1<<CS00); //Prescaler 1024	
	TIMSK |= (1<<TOIE0);
}

int main(void)
{
	deviceno =  eeprom_read_byte(&address_persist);
	
	if(deviceno > 63){
		deviceno = 0;
	}
	 
	DmxAddress = deviceno * 4 +1;
	
	cli();
	initPWM();
	init_DMX_RX();
	init_Int0();
	start_Timer0();
	sei();
	
	DDRD |= (1<<PIND3);
	PORTD &= ~(1<<PIND3);
	
	set_sleep_mode(SLEEP_MODE_IDLE);
	
    while(1)
    {
		sleep_mode();
		if (!programing_mode) setRGB(DmxRxField[0], DmxRxField[1], DmxRxField[2]);
    }
}

/* Timerinterrupt wird alle 32,768 ms ausgeführt. */
ISR(TIMER0_OVF_vect) {
	taster_refresh(&program_taster);
	
	if(programing_switch_counter_start) {
		if (++programing_switch_counter == PROGRAMMING_SWITCH_COUNTER_TICKS) {
			programing_switch_counter_ov = 1;
			if (programing_mode) {
				programing_mode = 0;
				if(eeprom_read_byte(&address_persist) != deviceno) {
					eeprom_write_byte(&address_persist, deviceno);	
				}
				DmxAddress = deviceno * 4 +1;
			} else {
				programing_mode = 1;
				setRGB(0x00,0x00,0x00);
				blink_state = BREAK_OFF;
				blink_counter = 0;
			}
			
			programing_switch_counter_start = 0;
		}
	}
	if (programing_mode)
	{			
		if (++blink_counter == BLINK_COUNTER_TICKS)
		{
			blink_counter = 0;
			switch (blink_state)
			{
				case BREAK_OFF:
								setRGB(0,0,0xFF);
								blink_state = BREAK_ON;
								break;
				case BREAK_ON:
								setRGB(0,0,0);
								blink_digit = (deviceno) % 10;
								blink_digit_count = 0;
								if (blink_digit == 0)
								{
									blink_digit = (deviceno/10) % 10;
									blink_digit_count = 0;
									if (blink_digit == 0)
									{
										blink_state = BREAK_OFF;
									} else {
										blink_state = DIGIT_TWO_OFF;
									}
								} else {
									blink_state = DIGIT_ONE_OFF;
								}
								break;
				case DIGIT_ONE_OFF:
								setRGB(0xFF,0,0);
								blink_state = DIGIT_ONE_ON;
								break;
				case DIGIT_ONE_ON:
								setRGB(0,0,0);
								if(++blink_digit_count == blink_digit)
								{
									blink_digit = (deviceno/10) % 10;
									blink_digit_count = 0;
									if (blink_digit == 0)
									{
										blink_state = BREAK_OFF;
									} else {
										blink_state = DIGIT_TWO_OFF;
									}
								} else 	{
									blink_state = DIGIT_ONE_OFF;
								}
								break;
				case DIGIT_TWO_OFF:
								setRGB(0,0xFF,0);
								blink_state = DIGIT_TWO_ON;
								break;
				case DIGIT_TWO_ON:
								setRGB(0,0,0);
								if(++blink_digit_count == blink_digit)
								{
									blink_state = BREAK_OFF;
								} else 	{
									blink_state = DIGIT_TWO_OFF;
								}
								break;
			}
		}
	}
}