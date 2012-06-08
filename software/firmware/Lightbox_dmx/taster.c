/*
 * taster->c
 *
 * Created: 04->05->2011 16:51:42
 *  Author: tobias
 */ 

#include "taster.h"
#define NULL (void *) 0

void taster_init(taster_t *taster, 
    volatile uint8_t *port, 
    uint8_t pin, 
    void (*pressed) (void), 
    void (*released) (void)) 
{
	taster->state = TASTER_REALEASED;
	taster->port = port;
	taster->pin = pin;
	taster->pressed = pressed;
	taster->released = released;
}

#define TASTER (!(*(taster->port) & (1 << taster->pin)))

void taster_refresh(taster_t *taster) {
	switch(taster->state) {
		case TASTER_PRESSED:
						if(!TASTER)
							taster->state = TASTER_R_1;
						break;
		case TASTER_REALEASED:
						if(TASTER)
							taster->state = TASTER_P_1;
						break;
		case TASTER_P_1:
						if(TASTER) {														
							taster->state = TASTER_PRESSED;
							if (taster->pressed != NULL)  taster->pressed();	
						} else taster->state = TASTER_REALEASED;							
						break;
		case TASTER_R_1:		
						if(TASTER) {
							taster->state = TASTER_PRESSED;
						} else {
							taster->state = TASTER_REALEASED;
							if (taster->released != NULL) taster->released();
						}														
						break;
	}
}
