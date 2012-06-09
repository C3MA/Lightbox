#include <avr/io.h>
#include <inttypes.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define  USE_DIP
#define  F_OSC			(8000)		  		//oscillator freq. in kHz (typical 8MHz or 16MHz)


volatile uint8_t	 DmxRxField[4]; 		//array of DMX vals (raw)
volatile uint16_t	 DmxAddress;			//start address

extern void    init_DMX_RX(void);
