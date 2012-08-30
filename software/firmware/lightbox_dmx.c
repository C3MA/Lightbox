/*
 * lightbox.c
 *
 * Created: 08.06.2012
 *  Author: Florian Zahn (original-File created by tobias)
 *  Firmware version 2.1
 */ 
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "lib_dmx_in.h"

#define DMX_ADDRESS_PORT_0 PINB
#define DMX_ADDRESS_PIN_0  7
#define DMX_ADDRESS_PORT_1 PINB
#define DMX_ADDRESS_PIN_1  6
#define DMX_ADDRESS_PORT_2 PINB
#define DMX_ADDRESS_PIN_2  5
#define DMX_ADDRESS_PORT_3 PINB
#define DMX_ADDRESS_PIN_3  1
#define DMX_ADDRESS_PORT_4 PIND
#define DMX_ADDRESS_PIN_4  4
#define DMX_ADDRESS_PORT_5 PIND
#define DMX_ADDRESS_PIN_5  3
#define DMX_ADDRESS_PORT_6 PIND
#define DMX_ADDRESS_PIN_6  2
#define DMX_ADDRESS_PORT_7 PINB
#define DMX_ADDRESS_PIN_7  0
#define DMX_ADDRESS_PORT_8 PIND
#define DMX_ADDRESS_PIN_8  6

#define PWM_REG_RED   OCR1B
#define PWM_REG_GREEN OCR1A
#define PWM_REG_BLUE  OCR0A

#define PWM_LOG_8 1
#define USE_WDT 1

#if PWM_LOG_8

const uint8_t pwmtable[32] PROGMEM =
{
    0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23,
    27, 32, 38, 45, 54, 64, 76, 91, 108, 128, 152, 181, 215, 255
};

#endif

void initIO(void) {
    // Direction-Register f�r Port B setzten
    DDRB  |= (1 << PB4) | (1 << PB3) | (1 << PB2); 

    // Ausgabepins oder PullUp-Widerst�nde setzten
    PORTB |= (1 << PB7) | (1 << PB6 ) | (1 << PB5) | (1 << PB1) | (1<< PB0);

    // Direction-Register f�r Port D setzten
    DDRD |= (1 << PD5);

    // Ausgabepins oder PullUp-Widerst�nde setzten
    PORTD |= (1 << PD6) | (1 << PD4) | (1 << PD3) | (1 << PD2);
}

uint16_t readDMXAddress(void) {
    uint16_t address = 0;
    address |= (uint16_t) ((DMX_ADDRESS_PORT_0 & (1 << DMX_ADDRESS_PIN_0)) >> DMX_ADDRESS_PIN_0) |
        ((uint16_t) ((DMX_ADDRESS_PORT_1 & (1 << DMX_ADDRESS_PIN_1)) >> DMX_ADDRESS_PIN_1) << 1) |
        ((uint16_t) ((DMX_ADDRESS_PORT_2 & (1 << DMX_ADDRESS_PIN_2)) >> DMX_ADDRESS_PIN_2) << 2) |
        ((uint16_t) ((DMX_ADDRESS_PORT_3 & (1 << DMX_ADDRESS_PIN_3)) >> DMX_ADDRESS_PIN_3) << 3) |
        ((uint16_t) ((DMX_ADDRESS_PORT_4 & (1 << DMX_ADDRESS_PIN_4)) >> DMX_ADDRESS_PIN_4) << 4) |
        ((uint16_t) ((DMX_ADDRESS_PORT_5 & (1 << DMX_ADDRESS_PIN_5)) >> DMX_ADDRESS_PIN_5) << 5) |
        ((uint16_t) ((DMX_ADDRESS_PORT_6 & (1 << DMX_ADDRESS_PIN_6)) >> DMX_ADDRESS_PIN_6) << 6) |
        ((uint16_t) ((DMX_ADDRESS_PORT_7 & (1 << DMX_ADDRESS_PIN_7)) >> DMX_ADDRESS_PIN_7) << 7) |
        ((uint16_t) ((DMX_ADDRESS_PORT_8 & (1 << DMX_ADDRESS_PIN_8)) >> DMX_ADDRESS_PIN_8) << 8);

    return (~address) & 0x01FF;
}

#if PWM_LOG_8
uint8_t getLogValue(uint8_t value) {
    return pgm_read_word(pwmtable + value / 8);
}
#endif

void setRGB(uint8_t red, uint8_t green, uint8_t blue) {
    #if PWM_LOG_8
    red = getLogValue(red);
    green = getLogValue(green);
    blue = getLogValue(blue);
    #endif
    
    if (PWM_REG_RED == 0x00) {
	if (red > 0) {
		TCCR1A |= (1 << COM1B1);
	}
    }
    else {
        if (red == 0) {
		TCCR1A &= ~(1 << COM1B1);
		PORTB &= ~(1 << PB4);
        }
    }
    PWM_REG_RED = red;

    if (PWM_REG_GREEN == 0x00) {
	if (green > 0) {
		TCCR1A |= (1 << COM1A1);
	}
    }
    else {
        if (green == 0) {
		TCCR1A &= ~(1 << COM1A1);
		PORTB &= ~(1 << PB3);
        }
    }
    PWM_REG_GREEN = green;
    
    if (PWM_REG_BLUE == 0x00) {
	if (blue > 0) {
		TCCR0A |= (1 << COM0A1);
	}
    }
    else {
        if (blue == 0) {
		TCCR0A &= ~(1 << COM0A1);
		PORTB &= ~(1 << PB2);
        }
    }
    PWM_REG_BLUE = blue;

#if USE_WDT
    wdt_reset();
#endif
}

void initPWM(void) {
    PWM_REG_RED = 0;
    PWM_REG_GREEN = 0;
    PWM_REG_BLUE = 0;

    //Fast-PWM-Mode mit SET bei 255; aber nicht mit den Ausgangspin verbunden; Da standartm��ig 0 und das glimmen unerw�nscht ist. 
    // CLEAR bei compare muss gesetzt werden um die PWMs mit den Ausgangpins zu verbinden.
    //OC0A 
    TCCR0A = (1 << WGM01) | (1 << WGM00);
    //OC1A & OC1B
    TCCR1A = (1 << WGM10) | (1 << WGM12);

    //prescaler 64
    TCCR0B = (1 << CS01) | (1 << CS00); 
    TCCR1B = (1 << CS11) | (1 << CS10);
}

int main(void)
{
    cli();
    initIO();

    DmxAddress = readDMXAddress();

    initPWM();

#if USE_WDT
    // Init WDT 60 milisecends
    wdt_enable(WDTO_60MS);
#endif

    if(DmxAddress == 0) 
    {
        setRGB(255,255,255);
    }
    else
    {
        init_DMX_RX();
        setRGB(0,0,0);
    }
    sei();

    set_sleep_mode(SLEEP_MODE_IDLE);
	
    // Status-LED an
    // PORTD |= (1 << PD5);

    while(1)
    {
        sleep_mode();
        setRGB(DmxRxField[0], DmxRxField[1], DmxRxField[2]);
    }
}

