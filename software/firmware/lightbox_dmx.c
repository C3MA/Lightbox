/*
 * lightbox.c
 *
 * Created: 08.06.2012
 *  Author: tobias
 */ 
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include "lib_dmx_in.h"


void setRGB(int red, int green, int blue) {	

}

void initPWM(void) {
}

int main(void)
{
	DmxAddress = 23;
	
	cli();
	initPWM();
	init_DMX_RX();
	sei();
	
	set_sleep_mode(SLEEP_MODE_IDLE);
	
    while(1)
    {
		sleep_mode();
		setRGB(DmxRxField[0], DmxRxField[1], DmxRxField[2]);
    }
}

