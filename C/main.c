#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>	// for itoa() call
#include <string.h>	// for memset() call
#include "uart.h"	// this functions needs exact this UART-lib, because this one writes the inputdata direct to gCommand
#include "timer.h"
#include "util.h"

/************* Configuration **************/
//#define BIDIRECTIONAL	// When this is defined, the uC will answer, too.

#define UART_BAUD_RATE      57600UL //Definition als unsigned long, sonst gibt es Fehler in der Berechnung

#define DELAY 90000


/************* Logic **************/

#define COMMAND_LENGTH 10

#ifdef BIDIRECTIONAL
	#define MSG(a) uart_send_s(a);
#else
	#define MSG(a)	;
#endif

static char gCommand[COMMAND_LENGTH];
static uint8_t gProgrammode=0;

static uint8_t gId = 0;

static void fillActualCommand(void);

int main(void)
{
// Pindefenition	
	cli();
	DDRD = (1<<PD6)|(1<<PD5);
	PORTD = 0x00;
	PIND = 0x00;
	PORTD |= (1 << PD6);
	uart_init_own( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );

// Interrupt für INT1 und INT0 intialisieren
	MCUCR |= 0x0F;
	
	sei();
	
	
	/********* PWD *********/
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
	
	int red, green, blue, id;
	int i;
	
	PORTD |= (1 << PD5);
	uart_send_s("Hallo Welt\r\n");

	while(1) {
		// Check if programm-button was pressed. (see: gProgrammode)
				
		
		fillActualCommand();
		
		switch (gCommand[1]) {
			case 'w': // check for an "w" as write
				id = decodeHex(gCommand[8],gCommand[9]);
				if (gId != id)
					continue;
				red = decodeHex(gCommand[2],gCommand[3]);
				green = decodeHex(gCommand[4],gCommand[5]);
				blue = decodeHex(gCommand[6],gCommand[7]);
				MSG("new colorvalues are set.\r\n");
				MSG(gCommand);
				MSG("\r\n");
				break;
			case 'i': // check for an "i" as initialize
				//TODO logic!
				if (gProgrammode == 0)
				{
					MSG("Initialize mode is not set!\r\n");
					continue;
				}
				break;
			default:
				MSG("Unknown Parameter\r\n");
				break;
		}
		
		// do the pwm
		OCR1AL = 0xFF - red;
		OCR1BL = 0xFF - green;
		OCR2   = 0xFF - blue;		
		//delayMicros(DELAY); // Sleep one Second
	};
}

/*
 Command:
 pwRRGGBBid							e.g. pw00FF0001
 pi0000NNid	<- NN = new device id
 Fill the serial input buffer into the gCommand value
 */
static void fillActualCommand(void)
{
	int i;
	unsigned int c;
	
	// Clear the buffer
	memset(gCommand, 0, COMMAND_LENGTH);
	
	for(i = 0; i < COMMAND_LENGTH; i++) {
		c = uart_getc();
		if ( c & UART_NO_DATA ) {
			// decrement, so the actual position could be filled at the next time.
			i--;
		} else {
			gCommand[i] = c;
			
			// check if command begins with a "p"
			if (gCommand[0] != 'p')
				return;
		}
	}
}
