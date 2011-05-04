/*
 * Hello.c
 *
 * Created: 30.04.2011 22:46:38
 *  Author: tobias
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

/* define CPU frequency in Mhz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 7372800UL
#endif

/* 57600 baud */
#define UART_BAUD_RATE 57600UL

#define UBRR_VAL ((F_CPU+UART_BAUD_RATE*8)/(UART_BAUD_RATE*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/UART_BAUD_RATE) // Fehler in Promille, 1000 = kein Fehler.
 
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error Systematischer Fehler der Baudrate groesser 1% und damit zu hoch! 
#endif 

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

volatile uint8_t programing_mode = 0;

void init_Int0() {
	DDRD &= ~(1<<DDD2);
	PORTD |= (1<<PD2);
}

void start_Timer0() {
	TCNT0 = 0x00;
	TCCR0 = (1<<CS02); //Prescaler 256	
	TIMSK |= (1<<TOIE0);
}


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
				setRGB(0,0,0);
				programing_mode = 0;
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

void init_uart(void)
{
	UCSRB = (1<<RXEN)|(1<<RXCIE);  // UART RX und RX Interrupt einschalten
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);  // Asynchron 8N1 
 
	UBRRH = UBRR_VAL >> 8;
	UBRRL = UBRR_VAL & 0xFF;
	
	
	#ifdef _DEBUG
	UCSRB |= (1<<TXEN); //Tx
	#endif // _DEBUG
}

#ifdef _DEBUG
int uart_putc(unsigned char c)
{
    while (!(UCSRA & (1<<UDRE)))  /* warten bis Senden moeglich */
    {
    }                             
 
    UDR = c;                      /* sende Zeichen */
    return 0;
}
 
 
/* puts ist unabhaengig vom Controllertyp */
void uart_puts (char *s)
{
    while (*s)
    {   /* so lange *s != '\0' also ungleich dem "String-Endezeichen" */
        uart_putc(*s);
        s++;
    }
}
#endif // _DEBUG

int main(void)
{
	cli();
	
	init_uart();
	
    /*
     * now enable interrupt, since UART library is interrupt controlled
     */
    sei();
	address =  eeprom_read_byte(&address_persist);
	
	initPWM();
	init_Int0();
	start_Timer0();
	
	setRGB(128,50,100);
	
	#ifdef _DEBUG
	uart_puts("booted!!!\n");
	#endif // _DEBUG
    
	set_sleep_mode(SLEEP_MODE_IDLE);
	
    while(1)
    {
		/* TODO: Taster auswerten (Interupt gesteuert)*/
		sleep_mode();
	}		
}

ISR(USART_RXC_vect) {
	unsigned char nextChar;
 
	// Daten aus dem Puffer lesen
	nextChar = UDR;
	
	#ifdef _DEBUG
	uart_putc(nextChar);
	#endif // _DEBUG
	
	refreshStateMachine(nextChar);
}

enum taster {PRESSED, REALEASED, P_1, R_1};
uint8_t taster_state = REALEASED;

#define TASTER (!(PIND & (1<<PIND2)))

ISR(TIMER0_OVF_vect) {
	switch(taster_state) {
		case PRESSED:
						if(!TASTER)
							taster_state = R_1;
						break;
		case REALEASED:
						if(TASTER)
							taster_state = P_1;
						break;
		case P_1:		
						if(TASTER) {
							if (programing_mode) {
								programing_mode = 0;
								setRGB(0x00,0x00,0x00);
							} else {
								programing_mode = 1;
								setRGB(0xff,0xff,0xff);
							}								
							taster_state = PRESSED;
						} else taster_state = REALEASED;							
						break;
		case R_1:		
						if(TASTER) {
							taster_state = PRESSED;
						} else {
							taster_state = REALEASED;
						}														
						break;
	}
}