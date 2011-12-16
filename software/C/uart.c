/*
 *  uart.c
 *  ChrisProj
 *
 *  Created by ollo on 24.03.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "uart.h"


/* size of RX/TX buffers */
#define UART_RX_BUFFER_MASK ( UART_RX_BUFFER_SIZE - 1)
#define UART_TX_BUFFER_MASK ( UART_TX_BUFFER_SIZE - 1)

/*
 *  module global variables
 */
static volatile unsigned char UART_TxBuf[UART_TX_BUFFER_SIZE];
static volatile unsigned char UART_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART_TxHead;
static volatile unsigned char UART_TxTail;
static volatile unsigned char UART_RxHead;
static volatile unsigned char UART_RxTail;
static volatile unsigned char UART_LastRxError;


void uart_send_c(unsigned char c)
{
	UDR = c;
}

ISR(USART_RXC_vect)
{	
	unsigned char tmphead;
	unsigned char data;
	unsigned char usr;
	unsigned char lastRxError;
    // This registers are used on the atmega 8
	usr  = UCSRA;
	
	data = UDR;
	   
	lastRxError = (usr & (_BV(FE)|_BV(DOR)) ); // This error stuff should be reactivated
	
    /* calculate buffer index */ 
    tmphead = ( UART_RxHead + 1) & UART_RX_BUFFER_MASK;
    
    if ( tmphead == UART_RxTail ) {
        /* error: receive buffer overflow */
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
				
    }else{
        /* store new index */
        UART_RxHead = tmphead;
        /* store received data in buffer */
        UART_RxBuf[tmphead] = data;
    }
	UART_LastRxError = lastRxError; 
	
}

ISR(SIG_UART_DATA)
{
	unsigned char tmptail;
	
    
    if ( UART_TxHead != UART_TxTail) {
        /* calculate and store new buffer index */
        tmptail = (UART_TxTail + 1) & UART_TX_BUFFER_MASK;
        UART_TxTail = tmptail;
        /* get one byte from buffer and write it to UART */
        UDR = UART_TxBuf[tmptail];  /* start transmission */
    }else{
        /* tx buffer empty, disable UDRE interrupt */
        UCSRB &= ~_BV(UDRIE);
    }
}

/*************************************************************************
 Function: uart_putc()
 Purpose:  write byte to ringbuffer for transmitting via UART
 Input:    byte to be transmitted
 Returns:  none          
 **************************************************************************/
extern void uart_putc(unsigned char data)
{
    unsigned char tmphead;
	
    
    tmphead  = (UART_TxHead + 1) & UART_TX_BUFFER_MASK;
    
    while ( tmphead == UART_TxTail ){
        ;/* wait for free space in buffer */
    }
    
    UART_TxBuf[tmphead] = data;
    UART_TxHead = tmphead;
	
    /* enable UDRE interrupt */
    UCSRB    |= _BV(UDRIE);
	
}/* uart_putc */

extern void uart_send_s(const char *s)
{
	while (*s > 0)
		uart_putc(*s++);
}

extern int uart_init_own(const int baudrate)
{
	UCSRB |= (1 << RXCIE) | (1 << RXEN) | (1 << TXEN);
	UBRRL = baudrate;
	return 0;
}

/*************************************************************************
 Function: uart_getc()
 Purpose:  return byte from ringbuffer  
 Returns:  lower byte:  received byte from ringbuffer
 higher byte: last receive error
 **************************************************************************/
extern unsigned int uart_getc(void)
{    
    unsigned char tmptail;
    unsigned char data;
	
	
    if ( UART_RxHead == UART_RxTail ) {
        return UART_NO_DATA;   /* no data available */
    }
    
    /* calculate /store buffer index */
    tmptail = (UART_RxTail + 1) & UART_RX_BUFFER_MASK;
    UART_RxTail = tmptail; 
    
    /* get data from receive buffer */
    data = UART_RxBuf[tmptail];
    
    return (UART_LastRxError << 8) + data;
	
}/* uart_getc */