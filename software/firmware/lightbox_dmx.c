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

#if PWM_LOG_16

int16_t pwmtable[256] PROGMEM = {0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
                                 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
                                 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6,
                                 6, 7, 7, 7, 8, 8, 8, 9, 9, 10, 10, 10, 11,
                                11, 12, 12, 13, 13, 14, 15, 15, 16, 17, 17,
                                18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
                                29, 31, 32, 33, 35, 36, 38, 40, 41, 43, 45,
                                47, 49, 52, 54, 56, 59, 61, 64, 67, 70, 73,
                                76, 79, 83, 87, 91, 95, 99, 103, 108, 112,
                               117, 123, 128, 134, 140, 146, 152, 159, 166,
                               173, 181, 189, 197, 206, 215, 225, 235, 245,
                               256, 267, 279, 292, 304, 318, 332, 347, 362,
                               378, 395, 412, 431, 450, 470, 490, 512, 535,
                               558, 583, 609, 636, 664, 693, 724, 756, 790,
                               825, 861, 899, 939, 981, 1024, 1069, 1117,
                              1166, 1218, 1272, 1328, 1387, 1448, 1512,
                              1579, 1649, 1722, 1798, 1878, 1961, 2048,
                              2139, 2233, 2332, 2435, 2543, 2656, 2773,
                              2896, 3025, 3158, 3298, 3444, 3597, 3756,
                              3922, 4096, 4277, 4467, 4664, 4871, 5087,
                              5312, 5547, 5793, 6049, 6317, 6596, 6889,
                              7194, 7512, 7845, 8192, 8555, 8933, 9329,
                              9742, 10173, 10624, 11094, 11585, 12098,
                             12634, 13193, 13777, 14387, 15024, 15689,
                             16384, 17109, 17867, 18658, 19484, 20346,
                             21247, 22188, 23170, 24196, 25267, 26386,
                             27554, 28774, 30048, 31378, 32768, 34218,
                             35733, 37315, 38967, 40693, 42494, 44376,
                             46340, 48392, 50534, 52772, 55108, 57548,
                             60096, 62757, 65535};
#endif

void initIO(void) {
    // Direction-Register für Port B setzten
    DDRB  |= (1 << PB4) | (1 << PB3) | (1 << PB2); 

    // Ausgabepins oder PullUp-Widerstände setzten
    PORTB |= (1 << PB7) | (1 << PB6 ) | (1 << PB5) | (1 << PB1) | (1<< PB0);

    // Direction-Register für Port D setzten
    DDRD |= (1 << PD5);

    // Ausgabepins oder PullUp-Widerstände setzten
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

void setRGB(uint8_t red, uint8_t green, uint8_t blue) {
    //static uint8_t red_s = 0;
    //static uint8_t green_s = 0;
    //static uint8_t blue_s = 0;	

    // pgm_read_word(pwmtable + red);

    PWM_REG_RED = red;
    PWM_REG_GREEN = green;
    PWM_REG_BLUE = blue;
}

void initPWM(void) {
    PWM_REG_RED = 0;
    PWM_REG_GREEN = 0;
    PWM_REG_BLUE = 0;

    //Fast-PWM-Mode mit SET bei 255 und CLEAR bei compare
    //OC0A 
    TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
    //OC1A & OC1B
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10) | (1 << WGM12);

    //prescaler 8
    TCCR0B = (1 << CS01);
    TCCR1B = (1 << CS11);
}

int main(void)
{
    cli();
    initIO();

    DmxAddress = readDMXAddress();

    initPWM();
    
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

