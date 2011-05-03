/*
 * Hello.c
 *
 * Created: 30.04.2011 22:46:38
 *  Author: tobias
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "uart.h"

/* define CPU frequency in Mhz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 7372800UL
#endif

/* 9600 baud */
#define UART_BAUD_RATE      57600

#define UBRR_VAL ((F_CPU+UART_BAUD_RATE*8)/(UART_BAUD_RATE*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/UART_BAUD_RATE) // Fehler in Promille, 1000 = kein Fehler.
 
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error Systematischer Fehler der Baudrate groesser 1% und damit zu hoch! 
#endif 

#define COMMAND_LENGTH 10


enum sm_states { SEARCH_PREFIX, READ_COMMAND, WAIT_FOR_END_W, WAIT_FOR_END_I, READ_RED_H, READ_RED_L, READ_GREEN_H, READ_GREEN_L, READ_BLUE_H, READ_BLUE_L, READ_ADDRESS_H, READ_ADDRESS_L, READ_NEW_ADDRESS_H, READ_NEW_ADDRESS_L, READ_TYP_H, READ_TYP_L, READ_STUFFING_0, READ_STUFFING_1, READ_STUFFING_2, READ_STUFFING_3};
struct {
	int state;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t address;
} state_machine;

uint8_t address_persist EEMEM; // standart adresse ist 255. Bei jedem flashen wird diese überschrieben
uint8_t address;
uint8_t programing_mode = 1; //TODO FIX address
unsigned int c;

void setRGB(int red, int green, int blue) {	
		OCR1AL = 0xFF - red;
		OCR1BL = 0xFF - green;
		OCR2   = 0xFF - blue;
}

static uint8_t decodeSingleHex(char number) {
	if ((number >= '0') & (number <= '9')) {
		return number - '0';
	} else if ((number >= 'A') & (number <= 'F')) {
		return number - 'A' + 10;
	} else if ((number >= 'a') & (number <= 'f')) {
		return number - 'a' + 10;
	} else {
		return 0;	// default zero is returned, so the addition won't create an error
	}
}

void refreshStateMachine(char c) {
	switch (state_machine.state)
	{
		case SEARCH_PREFIX:
			if( 'p' == c) { 
				state_machine.state = READ_COMMAND;
			}
			break;
		case READ_COMMAND:
			switch (c)
			{
				case 'i':
					if (programing_mode)
					{
						state_machine.state = READ_STUFFING_0;
					} else state_machine.state = SEARCH_PREFIX;
					break;
				case 'w': 
					state_machine.state = READ_RED_H;
					break;
				default:
					state_machine.state = SEARCH_PREFIX;
					break;
			}
			break;
		case READ_STUFFING_0:
			state_machine.state = READ_STUFFING_1;
			break;
		case READ_STUFFING_1:
			state_machine.state = READ_STUFFING_2;
			break;
		case READ_STUFFING_2:
			state_machine.state = READ_STUFFING_3;
			break;
		case READ_STUFFING_3:
			state_machine.state = READ_TYP_H;
			break;
		case READ_TYP_H:
			/*TODO*/
			state_machine.state = READ_TYP_L;
			break;
		case READ_TYP_L:
			/*TODO*/
			state_machine.state = READ_NEW_ADDRESS_H;
			break;
		case READ_NEW_ADDRESS_H:
			state_machine.address = 0xF0 & (decodeSingleHex(c) << 4);
			state_machine.state = READ_NEW_ADDRESS_L;
			break;
		case READ_NEW_ADDRESS_L:
			state_machine.address |= 0x0F & decodeSingleHex(c);
			state_machine.state = WAIT_FOR_END_I;
			break;
		case WAIT_FOR_END_I:
			if(c=='o') {
				eeprom_write_byte(&address_persist, state_machine.address);
				address = state_machine.address;
			}
			state_machine.state = SEARCH_PREFIX;
			break;
		
		case READ_RED_H:
			state_machine.red = 0xF0 & (decodeSingleHex(c) << 4);
			state_machine.state = READ_RED_L;
			break;
		case READ_RED_L:
			state_machine.red |= 0x0F & decodeSingleHex(c);
			state_machine.state = READ_GREEN_H;
			break;
		case READ_GREEN_H:
			state_machine.green = 0xF0 & (decodeSingleHex(c) << 4);
			state_machine.state = READ_GREEN_L;
			break;
		case READ_GREEN_L:
			state_machine.green |= 0x0F & decodeSingleHex(c);
			state_machine.state = READ_BLUE_H;
			break;
		case READ_BLUE_H:
			state_machine.blue = 0xF0 & (decodeSingleHex(c) << 4);
			state_machine.state = READ_BLUE_L;
			break;
		case READ_BLUE_L:
			state_machine.red |= 0x0F & decodeSingleHex(c);
			state_machine.state = READ_ADDRESS_H;
			break;
		case READ_ADDRESS_H:
			state_machine.address = 0xF0 & (decodeSingleHex(c) << 4);
			state_machine.state = READ_ADDRESS_L;
			break;
		case READ_ADDRESS_L:
			state_machine.address |= 0x0F & decodeSingleHex(c);
			if (address == state_machine.address) {
				state_machine.state = WAIT_FOR_END_W;
			} else state_machine.state = SEARCH_PREFIX;
			break;
		case WAIT_FOR_END_W:
			if(c== 'o') {
				setRGB(state_machine.red, state_machine.green, state_machine.blue);
			}
			state_machine.state = SEARCH_PREFIX;
			break;
	}
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

/*
void init_uart(void)
{
	UCSRB |= (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);  // UART RX, TX und RX Interrupt einschalten
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);  // Asynchron 8N1 
 
  UBRRH = UBRR_VAL >> 8;
  UBRRL = UBRR_VAL & 0xFF;
}
*/

int main(void)
{
	cli();
	 /*
     *  Initialize UART library, pass baudrate and AVR cpu clock
     *  with the macro 
     *  UART_BAUD_SELECT() (normal speed mode )
     *  or 
     *  UART_BAUD_SELECT_DOUBLE_SPEED() ( double speed mode)
     */
    uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
    
    /*
     * now enable interrupt, since UART library is interrupt controlled
     */
    sei();
	address =  eeprom_read_byte(&address_persist);
	
	/*
     *  Transmit string to UART
     *  The string is buffered by the uart library in a circular buffer
     *  and one character at a time is transmitted to the UART using interrupts.
     *  uart_puts() blocks if it can not write the whole string to the circular 
     *  buffer
     */
	
	initPWM();
	
	setRGB(0,0,0);
    
    while(1)
    {
		/* TODO: Taster auswerten*/
		c = uart_getc();
        if ( c & UART_NO_DATA )
        {
            /* 
             * no data available from UART 
             */
        }
        else
        {
			refreshStateMachine(c);
        }
    }
}