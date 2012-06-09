/**** A P P L I C A T I O N   N O T E   ************************************
*
* Title			: DMX512 reception library
* Version		: v1.3
* Last updated	: 08.06.12
* Target		: Lightbox-smd [attiny231]
* Clock			: 8MHz
*
* based on hendrik hoelscher, www.hoelscher-hi.de
***************************************************************************
 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License 
 as published by the Free Software Foundation; either version2 of 
 the License, or (at your option) any later version. 

 This program is distributed in the hope that it will be useful, 
 but WITHOUT ANY WARRANTY; without even the implied warranty of 
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 General Public License for more details. 

 If you have no copy of the GNU General Public License, write to the 
 Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 

 For other license models, please contact the author.

;***************************************************************************/

#include "lib_dmx_in.h"
#ifndef F_CPU
#warning "F_CPU war noch nicht definiert, wird nun nachgeholt mit 8000000"
#define F_CPU 8000000UL  // Systemtakt in Hz - Definition als unsigned long beachten 
                         // Ohne ergeben sich unten Fehler in der Berechnung
#endif
 
#define BAUD 250000UL      // Baudrate
 
// Berechnungen
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.
 
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch! 
#endif 

// ********************* local definitions *********************

enum {IDLE, BREAK, STARTB, STARTADR};			//DMX states

volatile uint8_t 	 gDmxState;

// *************** DMX Reception Initialisation ****************
void init_DMX_RX(void)
{
							
UBRRH = UBRR_VAL >> 8;
UBRRL = UBRR_VAL & 0xFF;		//250kbaud, 8N2
UCSRC  = (1<<UCSZ1)|(1<<UCSZ0)|(1<<USBS);
UCSRB  = (1<<RXEN)|(1<<RXCIE);

gDmxState= IDLE;
}



// *************** DMX Reception ISR ****************
ISR (USART_RXC_vect)
{
static  uint16_t DmxCount;
		uint8_t  USARTstate= UCSRA;				//get state before data!
		uint8_t  DmxByte   = UDR;				//get data
		uint8_t  DmxState  = gDmxState;			//just load once from SRAM to increase speed
 
if (USARTstate &(1<<FE))						//check for break
	{
	UCSRA &= ~(1<<FE);							//reset flag (necessary for simulation in AVR Studio)
	DmxCount =  DmxAddress;						//reset channel counter (count channels before start address)
	gDmxState= BREAK;
	}

else if (DmxState == BREAK)
	{
		/*TODO: Programmiermodus */
	if (DmxByte == 0) gDmxState= STARTB;		//normal start code detected
	else			  gDmxState= IDLE;
	}
	
else if (DmxState == STARTB)
	{
	if (--DmxCount == 0)						//start address reached?
		{
		DmxCount= 1;							//set up counter for required channels
		DmxRxField[0]= DmxByte;					//get 1st DMX channel of device
		gDmxState= STARTADR;
		}
	}

else if (DmxState == STARTADR)
	{
	DmxRxField[DmxCount++]= DmxByte;			//get channel
	if (DmxCount >= sizeof(DmxRxField)) 		//all ch received?
		{
		gDmxState= IDLE;						//wait for next break
		}
	}							
}

