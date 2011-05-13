/*
 * taster.h
 *
 * Created: 04.05.2011 16:52:13
 *  Author: tobias
 */ 
#include <avr/io.h>

#ifndef TASTER_H_
#define TASTER_H_

enum taster_states {TASTER_PRESSED, TASTER_REALEASED, TASTER_P_1, TASTER_R_1};

struct taster_struct {
	uint8_t state;
	volatile uint8_t *port;
	uint8_t pin;
	void (*pressed) ();
	void (*released) ();
};

typedef struct taster_struct taster_t;

void taster_init(taster_t *taster, volatile uint8_t *port, uint8_t pin, void (*pressed) (), void (*released) ());

void taster_refresh(taster_t *taster);


#endif /* TASTER_H_ */