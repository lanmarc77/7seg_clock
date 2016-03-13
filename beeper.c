/*
Copyright (C) 2015  Marcel Langner (langner.marcel@myiq.de)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BEEPER_C
#define BEEPER_C
#include <avr/io.h>
#include <avr/interrupt.h>
#include "beeper.h"

volatile unsigned char BEEPER_MODE=BEEPER_OFF;
#define BEEPER_PORT_ON PORTC|=0x04
#define BEEPER_PORT_OFF PORTC&=~0x04
unsigned int beeper_counter=0;
unsigned char I_BEEPER_MODE=BEEPER_OFF;

extern void beeper_set_mode(unsigned char mode){
	cli();
	I_BEEPER_MODE=mode;
	sei();
}

void beeper_ISR(void){
	if(BEEPER_MODE != I_BEEPER_MODE){
		BEEPER_MODE=I_BEEPER_MODE;
	}
	switch(BEEPER_MODE){
			case BEEPER_ON:		beeper_counter++;
								switch(beeper_counter){
									case 1: BEEPER_PORT_ON;break;
									case 350:	BEEPER_PORT_OFF;break;
									
									case 1501: BEEPER_PORT_ON;break;
									case 1850:	BEEPER_PORT_OFF;break;
									
									case 3001: BEEPER_PORT_ON;break;									
									case 3350:	BEEPER_PORT_OFF;break;
									
									case 4501: BEEPER_PORT_ON;break;
									case 4850:	BEEPER_PORT_OFF;break;
									
									case 10000: beeper_counter=0;break;
								}
								break;
			case BEEPER_OFF:	BEEPER_PORT_OFF;beeper_counter=0;
								break;
		
			case BEEPER_ON_FULL: BEEPER_PORT_ON;beeper_counter=0;
								break;
	}

}

void beeper_init(void){		
	//beeper port off by default
	PORTC&=~0x04;
	DDRC|=0x04;
}	

#endif