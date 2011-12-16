/*
 *  util.c
 *  ChrisProj
 *
 *  Created by ollo on 25.03.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "util.h"
#include <string.h>
#include <ctype.h>

static int decodeSingleHex(char number) {
	if (number >= '0' && number <= '9') {
		return number - '0';
	} else if (number >= 'A' && number <= 'F') {
		return number - 'A' + 10;
	} else {
		return 0;	// default zero is returned, so the addition won't create an error
	}
}

extern int decodeHex(char number1, char number2) {
	int value = 0;
	number1 = toupper(number1);
	number2 = toupper(number2);
	
	value = decodeSingleHex(number1);
	//Serial.print("Number1: ");
	//Serial.println(value);
	value = value << 4;
	//Serial.print("Number1: ");
	//Serial.println(value);
	value += decodeSingleHex(number2);
	//Serial.println("-------------");
	return value; 
}
