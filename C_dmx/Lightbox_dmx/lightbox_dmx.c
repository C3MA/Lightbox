/*
 * Hello.c
 *
 * Created: 30.04.2011 22:46:38
 *  Author: tobias
 */ 
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "lib_dmx_in.h"



/* define CPU frequency in Mhz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

/* 9600 baud */
#define UART_BAUD_RATE      250000

#define UBRR_VAL ((F_CPU+UART_BAUD_RATE*8)/(UART_BAUD_RATE*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/UART_BAUD_RATE) // Fehler in Promille, 1000 = kein Fehler.
 
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error Systematischer Fehler der Baudrate groesser 1% und damit zu hoch! 
#endif 


#define CHANNELS 4

uint8_t address_persist EEMEM; // standart adresse ist 255. Bei jedem flashen wird diese überschrieben
uint16_t address_offset = 1;
uint8_t programing_mode = 1; //TODO Addressconfig
unsigned int c;

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

int main(void)
{
	//address =  eeprom_read_byte(&address_persist);
	cli();
	initPWM();
	DmxAddress = address_offset;
	init_DMX_RX();
	sei();
	
	setRGB(128, DmxRxField[1], DmxRxField[2]);
    
    while(1)
    {
		//setRGB(128, DmxRxField[1], DmxRxField[2]);
    }
}